#include "nfa.h"
#include <stdlib.h>

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

// Parantez içi ifadeleri veya tek bir karakteri ayrıştırır
static Fragment parseAtom(NfaContext* ctx, LexerContext* lexer) {
    Token token = getNextToken(lexer);
    Fragment f = {NULL, NULL};
    
    if (token.type == tokenChar) {
        State* s = createState(ctx, stateChar, token.value, NULL, NULL);
        f.start = s;
        f.end = s;
    } else if (token.type == tokenLparen) {
        f = parseExpression(ctx, lexer);
        getNextToken(lexer); // ')' sembolünü tüket
    }
    return f;
}

// '*' ve '+' gibi tekrar operatörlerini işler
static Fragment parseRepetition(NfaContext* ctx, LexerContext* lexer) {
    Fragment f = parseAtom(ctx, lexer);
    
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
        f.end->out = next.start;
        f.end = next.end;
    }
    return f;
}

// VEYA (|) operatörlerini işler ve dallanmaları yönetir
static Fragment parseExpression(NfaContext* ctx, LexerContext* lexer) {
    Fragment f = parseConcat(ctx, lexer);
    
    while (1) {
        Token peek = peekToken(lexer);
        if (peek.type == tokenPipe) {
            getNextToken(lexer); // '|' sembolünü tüket
            
            Fragment right = parseConcat(ctx, lexer);
            
            State* split = createState(ctx, stateSplit, '\0', f.start, right.start);
            State* funnel = createEpsilon(ctx);
            
            f.end->out = funnel;
            right.end->out = funnel;
            
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

    LexerContext lexer;
    initLexer(&lexer, regexPattern);

    Fragment f = parseExpression(ctx, &lexer);

    State* matchState = createState(ctx, stateMatch, '\0', NULL, NULL);
    if (f.end != NULL) {
        f.end->out = matchState;
        ctx->startState = f.start;
    } else {
        ctx->startState = matchState;
    }

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