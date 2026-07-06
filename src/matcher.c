#include "nfa.h"
#include <stdlib.h>
#include <stdbool.h>

// Aynı anda üzerinde bulunduğumuz düğümleri tutacak liste
typedef struct {
    State** states;
    int count;
    int capacity;
} StateList;

static int listIdCounter = 0; // Sonsuz döngü engelliyici ID

// Yeni liste oluşturur
static StateList* createList(int capacity) {
    StateList* l = (StateList*)malloc(sizeof(StateList));
    l->states = (State**)malloc(capacity * sizeof(State*));
    l->count = 0;
    l->capacity = capacity;
    return l;
}

// Listeyi bellekten siler
static void freeList(StateList* l) {
    free(l->states);
    free(l);
}

// Epsilon yolları tüketerek gidilebilecek tüm aktif düğümleri listeye ekler
static void addState(StateList* l, State* s, int listId, const char* text, const char* textStart) {
    // Düğüm boşsa veya bu adımda zaten listeye eklendiyse atla (sonsuz döngü engeli)
    if (s == NULL || s->lastListId == listId) return;
    s->lastListId = listId;

    if (s->type == stateSplit) {
        // İki yola birden aynı anda gir
        addState(l, s->out, listId, text, textStart);
        addState(l, s->out1, listId, text, textStart);
        return;
    }

    if (s->type == stateAnchorStart) {
        if (text == textStart) addState(l, s->out, listId, text, textStart);
        return;
    }

    if (s->type == stateAnchorEnd) {
        if (*text == '\0' || *text == '\n') addState(l, s->out, listId, text, textStart);
        return;
    }

    // Karakter tüketecek standart düğümü listeye kaydet
    if (l->count >= l->capacity) {
        l->capacity *= 2;
        l->states = (State**)realloc(l->states, l->capacity * sizeof(State*));
    }
    l->states[l->count++] = s;
}

// Karakter eşleşme kuralları
static bool isMatch(State* state, const char* text) {
    if (state->type == stateChar) {
        return (*text != '\0' && *text == state->value);
    }
    if (state->type == stateAny) {
        return (*text != '\0' && *text != '\n');
    }
    if (state->type == stateClass) {
        if (*text != '\0') {
            unsigned char c = (unsigned char)(*text);
            bool match = state->classMask[c];
            return state->isNegativeClass ? !match : match;
        }
    }
    return false;
}

// Belirli bir metin başlangıcı için State-Set Simülasyonunu çalıştırır
static bool checkStateSet(NfaContext* ctx, const char* text, const char* textStart) {
    // Geçerli adım ve bir sonraki adımın durum listeleri
    StateList* clist = createList(ctx->stateCount > 0 ? ctx->stateCount : 16);
    StateList* nlist = createList(ctx->stateCount > 0 ? ctx->stateCount : 16);

    const char* current = text;
    listIdCounter++;
    
    // Ağacın başından başla ve tüm paralel yolları listeye yükle
    addState(clist, ctx->startState, listIdCounter, current, textStart);

    bool matched = false;

    // Aktif bir yol olduğu sürece ilerle
    while (clist->count > 0) {
        listIdCounter++;
        nlist->count = 0;

        for (int i = 0; i < clist->count; i++) {
            State* s = clist->states[i];
            
            if (s->type == stateMatch) {
                matched = true;
                goto end; // Başarılı döngüyü kır
            }
            
            // Eğer karakter kurallara uyuyorsa, hedefteki yeni düğümleri sonraki listeye aktar
            if (isMatch(s, current)) {
                addState(nlist, s->out, listIdCounter, current + 1, textStart);
            }
        }

        if (nlist->count == 0) break; // İlerleyecek yol kalmadıysa dur

        // Listeleri takas et (Yeni liste artık aktif liste oldu)
        StateList* temp = clist;
        clist = nlist;
        nlist = temp;

        current++;
    }
    
    // Uç senaryo: Belki kelimenin tam bittiği yerde (EOF) Match düğümü kalmıştır
    for (int i = 0; i < clist->count; i++) {
        if (clist->states[i]->type == stateMatch) {
            matched = true;
            break;
        }
    }

end:
    freeList(clist);
    freeList(nlist);
    return matched;
}

// Dışarıya açılan ana Regex çalıştırma fonksiyonu
bool matchNfa(NfaContext* ctx, const char* text) {
    const char* current = text;
    
    // Metnin üzerinde kayarak Substring Search yapar
    do {
        if (checkStateSet(ctx, current, text)) {
            return true;
        }
    } while (*current++ != '\0');
    
    return false;
}