#include "nfa.h"
#include <stdbool.h>

// Eşleştirme algoritması
// textStart: Çapalar (^) için metnin orijinal başlangıç adresini tutar
static bool checkState(State* state, const char* text, const char* textStart) {
    if (state == NULL) return false;

    // Başarılı Eşleşme
    if (state->type == stateMatch) {
        return true; 
    }

    // ^ Çapası: Sıfır genişlikli kontrol
    if (state->type == stateAnchorStart) {
        if (text == textStart) {
            // Metin ilerletilmeden sonraki duruma geçilir
            return checkState(state->out, text, textStart); 
        }
        return false;
    }

    // $ Çapası: Sıfır genişlikli kontrol
    if (state->type == stateAnchorEnd) {
        if (*text == '\0' || *text == '\n') {
            return checkState(state->out, text, textStart); 
        }
        return false;
    }

    if (state->type == stateChar) {
        if (*text != '\0' && *text == state->value) {
            return checkState(state->out, text + 1, textStart);
        }
        return false;
    }

    if (state->type == stateAny) {
        if (*text != '\0' && *text != '\n') {
            return checkState(state->out, text + 1, textStart);
        }
        return false;
    }

    if (state->type == stateClass) {
        if (*text != '\0') {
            unsigned char c = (unsigned char)(*text);
            bool isMatch = state->classMask[c];
            if (state->isNegativeClass) isMatch = !isMatch;
            
            if (isMatch) {
                return checkState(state->out, text + 1, textStart);
            }
        }
        return false;
    }

    if (state->type == stateSplit) {
        return checkState(state->out, text, textStart) || 
               checkState(state->out1, text, textStart);
    }

    return false;
}

// Dışarıya açılan ana Regex çalıştırma fonksiyonu
bool matchNfa(NfaContext* ctx, const char* text) {
    const char* current = text;
    
    // Metnin üzerinde kayarak Substring Search yapar
    do {
        if (checkState(ctx->startState, current, text)) {
            return true; // Herhangi bir yerde eşleşme yakalandı
        }
    } while (*current++ != '\0');
    
    return false;
}