#include "matcher.h"
#include <string.h>

// NFA üzerinde o anki durumdan itibaren metnin eşleşip eşleşmediğini kontrol eder.
static bool checkState(State* state, const char* text) {
    if (state == NULL) {
        return false;
    }

    // Başarı durumuna ulaştıysa ve metin de tamamen bittiyse eşleşme sağlanmıştır
    if (state->type == stateMatch) {
        return *text == '\0'; // Sadece metnin sonuna gelindiyse true dön
    }

    // Karakter eşleşmesi gerektiren bir durumdaysa
    if (state->type == stateChar) {
        // Metnin sonuna gelmediyse ve karakterler uyuyorsa bir sonraki duruma geç
        if (*text != '\0' && *text == state->value) {
            return checkState(state->out, text + 1);
        }
        return false;
    }

    // Küme eşleşmesi gerektiren bir durumdaysa
    if (state->type == stateClass) {
        if (*text != '\0') {
            unsigned char c = (unsigned char)(*text);
            bool isMatch = state->classMask[c]; // Karakter haritada var mı
            
            // Eğer negatif kümeyse [^...] sonucu tersine çevir
            if (state->isNegativeClass) {
                isMatch = !isMatch;
            }
            
            if (isMatch) {
                return checkState(state->out, text + 1);
            }
        }
        return false;
    }

    // İleride eklenecek olan dallanma (|) durumu için iki yolu da dene
    if (state->type == stateSplit) {
        return checkState(state->out, text) || checkState(state->out1, text);
    }

    return false;
}

// Verilen NFA yapısının hedef metinle eşleşip eşleşmediğini kontrol eder.
bool matchNfa(NfaContext* ctx, const char* text) {
    if (ctx == NULL || ctx->startState == NULL) {
        return false;
    }
    
    // NFA'nın başlangıç durumundan metni kontrol etmeye başla
    return checkState(ctx->startState, text);
}