#include "nfa.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Her bir paralel klonun taşıyacağı iş parçacığı yapısı
typedef struct {
    State* state;
    const char* captures[MAX_CAPTURES]; // Klonun sırtındaki hafıza çantası
} Thread;

typedef struct {
    Thread* threads;
    int count;
    int capacity;
} ThreadList;

static int listIdCounter = 0;

static ThreadList* createList(int capacity) {
    ThreadList* l = (ThreadList*)malloc(sizeof(ThreadList));
    l->threads = (Thread*)malloc(capacity * sizeof(Thread));
    l->count = 0;
    l->capacity = capacity;
    return l;
}

static void freeList(ThreadList* l) {
    free(l->threads);
    free(l);
}

// Yeni bir düğümü (ve klonun o anki çantasını) listeye ekler
static void addThread(ThreadList* l, State* s, const char** captures, int listId, const char* text, const char* textStart) {
    if (s == NULL || s->lastListId == listId) return;
    s->lastListId = listId;

    if (s->type == stateSplit) {
        addThread(l, s->out, captures, listId, text, textStart);
        addThread(l, s->out1, captures, listId, text, textStart);
        return;
    }

    // Klonun çantasını kopyala, yeni adresi not al ve Epsilon gibi devam et
    if (s->type == stateSave) {
        const char* oldCaptures[MAX_CAPTURES];
        memcpy(oldCaptures, captures, sizeof(const char*) * MAX_CAPTURES);
        // Savunma amaçlı sınır kontrolü: createNfa grup sayısını zaten MAX_CAPTURES'a
        // göre denetliyor, ama burada bir daha kontrol etmek dizi taşmasını (ve
        // olası çökmeyi) hiçbir koşulda mümkün kılmıyor.
        if (s->saveId >= 0 && s->saveId < MAX_CAPTURES) {
            oldCaptures[s->saveId] = text;
        }
        addThread(l, s->out, oldCaptures, listId, text, textStart);
        return;
    }

    if (s->type == stateAnchorStart) {
        if (text == textStart) addThread(l, s->out, captures, listId, text, textStart);
        return;
    }
    if (s->type == stateAnchorEnd) {
        if (*text == '\0' || *text == '\n') addThread(l, s->out, captures, listId, text, textStart);
        return;
    }

    if (l->count >= l->capacity) {
        l->capacity *= 2;
        l->threads = (Thread*)realloc(l->threads, l->capacity * sizeof(Thread));
    }
    l->threads[l->count].state = s;
    // Çantayı klonla
    memcpy(l->threads[l->count].captures, captures, sizeof(const char*) * MAX_CAPTURES);
    l->count++;
}

static bool isMatch(State* state, const char* text) {
    if (state->type == stateChar) return (*text != '\0' && *text == state->value);
    if (state->type == stateAny) return (*text != '\0' && *text != '\n');
    if (state->type == stateClass) {
        if (*text != '\0') {
            unsigned char c = (unsigned char)(*text);
            bool match = state->classMask[c];
            return state->isNegativeClass ? !match : match;
        }
    }
    return false;
}

static bool checkStateSet(NfaContext* ctx, const char* text, const char* textStart) {
    ThreadList* clist = createList(ctx->stateCount > 0 ? ctx->stateCount : 16);
    ThreadList* nlist = createList(ctx->stateCount > 0 ? ctx->stateCount : 16);

    const char* current = text;
    listIdCounter++;
    
    // Tertemiz, boş bir hafıza çantasıyla ilk klonu başlatır
    const char* initialCaptures[MAX_CAPTURES] = {NULL};
    addThread(clist, ctx->startState, initialCaptures, listIdCounter, current, textStart);

    bool matched = false;
    Thread* winningThread = NULL;

    static Thread bestMatch; // Kazanan thread'in hafızası, döngü devam edince kaybolmasın

    while (clist->count > 0) {
        listIdCounter++;
        nlist->count = 0;
        bool matchedThisStep = false;

        for (int i = 0; i < clist->count; i++) {
            Thread* t = &clist->threads[i];

            // Eşleşme bulunsa dahi döngü kırma
            // O anki en uzun/en iyi eşleşmeyi winningThread'e kaydet,
            // ama diğer klonların daha uzun bir eşleşme bulma ihtimaline karşı çalışmaya devam et.
            if (t->state->type == stateMatch) {
                // Aynı adımda (aynı metin uzunluğunda) birden fazla thread eşleşirse,
                // öncelik sırası clist içindeki sırayla belirlenir (ilk/en yüksek öncelikli
                // thread kazanır). Bu adımda zaten bir eşleşme kaydedildiyse, daha düşük
                // öncelikli sonraki eşleşmelerin üzerine yazmasına izin verme.
                if (!matchedThisStep) {
                    matchedThisStep = true;
                    matched = true;
                    bestMatch.state = t->state;
                    memcpy(bestMatch.captures, t->captures, sizeof(const char*) * MAX_CAPTURES);
                    winningThread = &bestMatch;
                }
                continue; // Bu klon hedefe ulaştı, ama diğer klonlar ilerlemeye devam eder
            }

            if (isMatch(t->state, current)) {
                addThread(nlist, t->state->out, t->captures, listIdCounter, current + 1, textStart);
            }
        }

        if (nlist->count == 0) break;
        ThreadList* temp = clist; clist = nlist; nlist = temp;
        current++;
    }
    
    for (int i = 0; i < clist->count; i++) {
        if (clist->threads[i].state->type == stateMatch) {
            matched = true; winningThread = &clist->threads[i]; break;
        }
    }

    // Eşleşme sağlandıysa, o anki klonun hafıza çantasını ekrana bastırır
    if (matched && winningThread != NULL) {
        printf("\n--- YAKALANAN GRUPLAR ---\n");
        for (int g = 0; g <= ctx->groupCount; g++) {
            const char* start = winningThread->captures[g * 2];
            const char* end = winningThread->captures[g * 2 + 1];
            if (start != NULL && end != NULL) {
                int len = end - start;
                printf("Grup %d: %.*s\n", g, len, start);
            }
        }
        printf("-------------------------\n");
    }

    freeList(clist); freeList(nlist);
    return matched;
}

bool matchNfa(NfaContext* ctx, const char* text) {
    const char* current = text;
    do {
        if (checkStateSet(ctx, current, text)) return true;
    } while (*current++ != '\0');
    return false;
}