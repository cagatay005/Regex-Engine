#include "nfa.h"
#include <stdlib.h>
#include <string.h>

// Bir NFA parçasının başlangıç ve bitiş düğümlerini tutar
typedef struct {
    State* start;
    State* end;
} Fragment;

// İleri seviye fonksiyon prototipleri
static Fragment parseExpression(NfaContext* ctx, LexerContext* lexer);
static Fragment parseConcat(NfaContext* ctx, LexerContext* lexer);
static Fragment parseRepetition(NfaContext* ctx, LexerContext* lexer);
static Fragment parseAtom(NfaContext* ctx, LexerContext* lexer);

// Lexer'ın konumunu bozmadan sıradaki token'a bakar
static Token peekToken(LexerContext* lexer) {
    size_t oldPos = lexer->position;
    Token t = getNextToken(lexer);
    lexer->position = oldPos;
    return t;
}

// Yeni bir durum yaratır ve bellek yöneticisine ekler
static State* createState(NfaContext* ctx, StateType type, char value, State* out, State* out1) {
    State* state = (State*)malloc(sizeof(State));
    state->type = type;
    state->value = value;
    state->out = out;
    state->out1 = out1;
    state->lastListId = 0;

    if (ctx->stateCount >= ctx->capacity) {
        ctx->capacity = (ctx->capacity == 0) ? 16 : ctx->capacity * 2;
        ctx->allStates = (State**)realloc(ctx->allStates, ctx->capacity * sizeof(State*));
    }
    
    ctx->allStates[ctx->stateCount++] = state;
    return state;
}

// Sadece 'out' bağlantısı olan boş (epsilon) bir geçiş düğümü oluşturur
static State* createEpsilon(NfaContext* ctx) {
    return createState(ctx, stateSplit, '\0', NULL, NULL);
}

// Yakalama grupları için kayıt (Save) düğümü
static State* createSaveState(NfaContext* ctx, int saveId, State* out) {
    State* state = createState(ctx, stateSave, '\0', out, NULL);
    state->saveId = saveId;
    return state;
}

// Küme düğümü oluşturur
static State* createClassState(NfaContext* ctx, bool* classMask, bool isNegativeClass, State* out) {
    State* state = createState(ctx, stateClass, '\0', out, NULL);
    memcpy(state->classMask, classMask, 256);
    state->isNegativeClass = isNegativeClass;
    return state;
}

// Parantez içi ifadeleri veya tek bir karakteri ayrıştırır
static Fragment parseAtom(NfaContext* ctx, LexerContext* lexer) {
    Token peek = peekToken(lexer);
    Fragment f = {NULL, NULL};
    
    if (peek.type == tokenChar) {
        Token token = getNextToken(lexer); // Karakteri güvenle tüket
        if (token.isError) ctx->hasError = true; // örn: sonda kalan tek başına '\'
        State* s = createState(ctx, stateChar, token.value, NULL, NULL);
        f.start = s;
        f.end = s;
    } else if (peek.type == tokenLparen) {
        getNextToken(lexer); // '(' sembolünü tüket
        
        ctx->groupCount++; // Yeni bir grup açıldığını bildir
        int groupId = ctx->groupCount; 
        
        f = parseExpression(ctx, lexer);
        Token closeParen = getNextToken(lexer); // ')' sembolünü tüket
        if (closeParen.type != tokenRparen) {
            ctx->hasError = true; // Kapanmamış parantez, örn: "(abc"
        }

        // Başlangıç (2*id) ve Bitiş (2*id+1) için gizli kayıt düğümleri oluştur
        State* startSave = createSaveState(ctx, groupId * 2, f.start);
        State* endSave = createSaveState(ctx, groupId * 2 + 1, NULL);
        
        if (f.end != NULL) f.end->out = endSave;
        else startSave->out = endSave;
        
        f.start = startSave;
        f.end = endSave;
    } else if (peek.type == tokenNCLparen) {
        getNextToken(lexer); // '(?:' sembollerini tüket

        // Yakalamayan grup: save-state ekleme, groupCount'u artırma.
        // {n,m} genişletmesinin ürettiği iç içe geçmiş "(?:x|)" sarmalayıcıları
        // bu sayede capture slotlarını tüketmez.
        f = parseExpression(ctx, lexer);
        Token closeParen = getNextToken(lexer); // ')' sembolünü tüket
        if (closeParen.type != tokenRparen) {
            ctx->hasError = true; // Kapanmamış parantez, örn: "(?:abc"
        }
    } else if (peek.type == tokenClass) {
        Token token = getNextToken(lexer); // Küme token'ını tüket
        if (token.isError) ctx->hasError = true; // örn: kapanmamış küme "[abc"
        State* s = createClassState(ctx, token.classMask, token.isNegativeClass, NULL);
        f.start = s;
        f.end = s;
    } else if (peek.type == tokenDot) {
        getNextToken(lexer); // Nokta token'ını tüket
        State* s = createState(ctx, stateAny, '\0', NULL, NULL); // Yeni düğümü yarat
        f.start = s;
        f.end = s;
    } else if (peek.type == tokenCaret) {
        getNextToken(lexer); // ^ sembolünü tüket
        State* s = createState(ctx, stateAnchorStart, '\0', NULL, NULL);
        f.start = s;
        f.end = s;
    } else if (peek.type == tokenDollar) {
        getNextToken(lexer); // $ sembolünü tüket
        State* s = createState(ctx, stateAnchorEnd, '\0', NULL, NULL);
        f.start = s;
        f.end = s;
    } else if (peek.type == tokenEof || peek.type == tokenRparen || peek.type == tokenPipe) {
        // Boş alt-ifade: "()", "a|" veya "|a" gibi durumlarda burada tüketilecek
        // bir şey yok; boş fragment'ı olduğu gibi yukarı döndürür.
    } else {
        // '*', '+', '?', '{', '}' gibi önünde atomu olmayan veya tanınmayan bir sembol.
        // İlerlemeyi garanti altına almak için tüket ve sözdizimi hatası olarak işaretle.
        getNextToken(lexer);
        ctx->hasError = true;
    }

    return f;
}

// '*' ve '+' gibi tekrar operatörlerini işler
static Fragment parseRepetition(NfaContext* ctx, LexerContext* lexer) {
    Fragment f = parseAtom(ctx, lexer);

    // Atom hiç üretilemediyse (boş alt-ifade ya da sözdizimi hatası), üzerine
    // tekrar operatörü uygulamaya kalkışma - "nothing to repeat" durumu.
    if (f.start == NULL) return f;

    Token peek = peekToken(lexer);
    if (peek.type == tokenStar) {
        getNextToken(lexer); // '*' sembolünü tüket
        
        // Döngü ve atlama yolları için baştan Split düğümü
        State* split = createState(ctx, stateSplit, '\0', f.start, NULL);
        f.end->out = split; 
        
        State* funnel = createEpsilon(ctx);
        split->out1 = funnel; 
        
        f.start = split;
        f.end = funnel;
    } 
    else if (peek.type == tokenPlus) {
        getNextToken(lexer); // '+' sembolünü tüket
        
        // En az 1 kez çalışması için dallanmayı sona koyar
        State* split = createState(ctx, stateSplit, '\0', f.start, NULL);
        f.end->out = split; 
        
        State* funnel = createEpsilon(ctx);
        split->out1 = funnel;
        
        // Başlangıç aynı kalır, bitiş olur
        f.end = funnel;
    }

    else if (peek.type == tokenQuestion) {
        getNextToken(lexer); // '?' sembolünü tüket
        
        // Parçaya girme veya tamamen atlama kararı
        State* split = createState(ctx, stateSplit, '\0', f.start, NULL);
        State* funnel = createEpsilon(ctx);
        
        f.end->out = funnel;   // Parça işlenirse funnel'a çıkar
        split->out1 = funnel;  // Parça atlanırsa doğrudan funnel'a çıkar
        
        f.start = split;
        f.end = funnel;
    }
    
    return f;
}

// Yan yana yazılmış metinleri birbirine bağlar
static Fragment parseConcat(NfaContext* ctx, LexerContext* lexer) {
    Fragment f = parseRepetition(ctx, lexer);
    
    while (1) {
        Token peek = peekToken(lexer);
        if (peek.type == tokenEof || peek.type == tokenPipe || peek.type == tokenRparen) {
            break;
        }
        
        Fragment next = parseRepetition(ctx, lexer);

        if (f.start == NULL) {
            // Birikmiş parça boş/hatalıysa akışı kırmadan bir sonrakini benimse
            f = next;
        } else if (next.start != NULL) {
            f.end->out = next.start;
            f.end = next.end;
        }
        // next.start == NULL ise (hatalı/boş parça) yut ve devam et;
        // lexer zaten ilerledi, sonsuz döngü riski yok.
    }
    return f;
}

// VEYA (|) operatörlerini işler ve dallanmaları yönetir
static Fragment parseExpression(NfaContext* ctx, LexerContext* lexer) {
    Fragment f = parseConcat(ctx, lexer);
    
    while (1) {
        Token peek = peekToken(lexer);
        if (peek.type == tokenPipe) {
            getNextToken(lexer); 
            
            Fragment right = parseConcat(ctx, lexer);
            
            State* split = createState(ctx, stateSplit, '\0', f.start, right.start);
            State* funnel = createEpsilon(ctx);
            
            // BOŞ FRAGMENT (NULL) KONTROLLERİ
            if (f.end != NULL) f.end->out = funnel;
            else split->out = funnel;
            
            if (right.end != NULL) right.end->out = funnel;
            else split->out1 = funnel;
            
            f.start = split;
            f.end = funnel;
        } else {
            break;
        }
    }
    return f;
}

// Verilen regex deseni için NFA ağacını inşa eder
NfaContext* createNfa(const char* regexPattern) {
    NfaContext* ctx = (NfaContext*)malloc(sizeof(NfaContext));
    ctx->allStates = NULL;
    ctx->stateCount = 0;
    ctx->capacity = 0;
    ctx->groupCount = 0; // Grup sayacını sıfırla
    ctx->hasError = false;

    LexerContext lexer;
    initLexer(&lexer, regexPattern);

    Fragment f = parseExpression(ctx, &lexer);

    // Yakalama grubu sayısı MAX_CAPTURES sınırını aşıyorsa güvenle reddet.
    // {n,m} genişletmesi (özellikle "{n,m}" aralıklı olanlar) her kullanımda
    // gizli parantez/grup oluşturduğundan, kullanıcının yazdığından çok daha
    // fazla grup ortaya çıkabilir ve sabit boyutlu captures[] dizisini taşırabilir.
    if ((ctx->groupCount + 1) * 2 > MAX_CAPTURES) {
        ctx->hasError = true;
    }

    // Ayrıştırma bitmesine rağmen ortada tüketilmemiş token kaldıysa
    // (örn: "abc)" içindeki fazladan ')') bu da bir sözdizimi hatasıdır.
    if (peekToken(&lexer).type != tokenEof) {
        ctx->hasError = true;
    }

    // expandQuantifier tarafından strdup'lanmış olabilecek belleği temizle
    if (lexer.ownsInput) free((char*)lexer.input);

    // Ayrıştırıcı bilmediği bir sembol ya da sözdizimi hatası yüzünden
    // geçerli bir NFA kuramadıysa, o ana kadar oluşturulan tüm düğümleri
    // temizleyip çökmek yerine NULL döndür.
    if (f.start == NULL || f.end == NULL || ctx->hasError) {
        freeNfa(ctx);
        return NULL;
    }

    // Group 0 için ana sarmalayıcılar
    State* startSave = createSaveState(ctx, 0, f.start);
    State* endSave = createSaveState(ctx, 1, NULL);
    State* matchState = createState(ctx, stateMatch, '\0', NULL, NULL);

    if (f.end != NULL) f.end->out = endSave;
    else startSave->out = endSave;
    
    endSave->out = matchState;
    ctx->startState = startSave;

    return ctx;
}

// NFA için ayrılan tüm belleği serbest bırakır
void freeNfa(NfaContext* ctx) {
    if (ctx == NULL) return;
    for (size_t i = 0; i < ctx->stateCount; i++) {
        free(ctx->allStates[i]);
    }
    free(ctx->allStates);
    free(ctx);
}