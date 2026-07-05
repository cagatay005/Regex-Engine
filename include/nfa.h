#ifndef NFA_H
#define NFA_H

#include <stddef.h>
#include "lexer.h"

// NFA dugumlerinin gorev tiplerini belirtir
typedef enum {
    stateChar,
    stateSplit,
    stateMatch,
    stateClass,
    stateAny
} StateType;

// NFA grafi uzerindeki tek bir baglanti noktasini temsil eder
typedef struct State {
    StateType type;
    char value;          // type stateChar ise aranacak karakteri tutar
    bool classMask[256]; // kümeye dahil olan karakterler
    bool isNegativeClass; // negatif küme bayrağı
    struct State* out;   // Birinci cikis durumunu gosterir
    struct State* out1;  // Ikinci cikis durumunu gosterir
    int lastListId;      // NFA gezinmesinde ayni dugume tekrar girilmesini onler
} State;

// NFA sisteminin baslangic durumunu ve tahsis edilen bellekleri tutar
typedef struct {
    State* startState;   // NFA kontrolunun baslayacagi ilk dugumu gosterir
    State** allStates;   // Bellekten temizlemek icin tum dugumlerin listesini tutar
    size_t stateCount;   // Olusturulan toplam dugum sayisini tutar
    size_t capacity;     // allStates dizisinin mevcut kapasitesini tutar
} NfaContext;

// Verilen regex metninden NFA makinesini insa eder
NfaContext* createNfa(const char* regexPattern);

// NFA yapisinin kullandigi tum bellegi temizler
void freeNfa(NfaContext* ctx);

#endif