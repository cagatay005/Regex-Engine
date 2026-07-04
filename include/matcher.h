#ifndef MATCHER_H
#define MATCHER_H

#include "nfa.h"
#include <stdbool.h>

// Verilen NFA yapısının hedef metinle eşleşip eşleşmediğini kontrol eder.
bool matchNfa(NfaContext* ctx, const char* text);

#endif