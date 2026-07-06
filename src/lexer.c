#include "lexer.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// {n,m} sözdizimini (a{2,4} -> aa(a|)(a|) gibi) NFA'nın anlayacağı primitiflere çevirir
static void expandQuantifier(LexerContext* lexer) {
    size_t i = lexer->position;
    int n = 0, m = -1;
    bool hasComma = false;

    // Hedeflenen önceki karakteri/grubu bul
    size_t targetStart = i - 1;
    if (lexer->input[targetStart] == ')') {
        int depth = 1;
        targetStart--;
        while (targetStart > 0 && depth > 0) {
            if (lexer->input[targetStart] == ')') depth++;
            else if (lexer->input[targetStart] == '(') depth--;
            if (depth > 0) targetStart--;
        }
    } else {
        // Eğer hedef karakter bir kaçış dizisiyse (örneğin \d, \w, \s)
        if (targetStart > 0 && lexer->input[targetStart - 1] == '\\') {
            targetStart--; // Başlangıcı '\' işaretini de kapsayacak şekilde 1 birim geri çek
        }
    }
    size_t targetLen = i - targetStart;

    // Sayıları Parse et
    while (i < lexer->length && lexer->input[i] != '}') {
        if (lexer->input[i] == ',') {
            hasComma = true;
        } else if (lexer->input[i] >= '0' && lexer->input[i] <= '9') {
            if (hasComma) {
                if (m == -1) m = 0;
                m = m * 10 + (lexer->input[i] - '0');
            } else {
                n = n * 10 + (lexer->input[i] - '0');
            }
        }
        i++;
    }
    
    // Geçersiz format, işlemi iptal et
    if (i >= lexer->length || lexer->input[i] != '}') return;
    
    // Eğer m yazılmadıysa (örn {3})
    if (!hasComma) m = n;

    // Yeni regex için bellek ayır
    size_t maxNewLen = lexer->length + (m == -1 ? n : m) * (targetLen + 5) + 10;
    char* newRegex = (char*)malloc(maxNewLen);
    
    // Şablonun sol tarafını kopyala (hedef kısım hariç)
    strncpy(newRegex, lexer->input, targetStart);
    size_t destIdx = targetStart;
    
    // N (Minimum) kere zorunlu kopyala
    for (int k = 0; k < n; k++) {
        strncpy(newRegex + destIdx, lexer->input + targetStart, targetLen);
        destIdx += targetLen;
    }
    
    // ,m yoksa sonsuz tekrar (Kleene Star)
    if (hasComma && m == -1) {
        strncpy(newRegex + destIdx, lexer->input + targetStart, targetLen);
        destIdx += targetLen;
        newRegex[destIdx++] = '*';
    } 
    // M kere opsiyonel kopyala
    else if (m > n) {
        for (int k = n; k < m; k++) {
            newRegex[destIdx++] = '(';
            strncpy(newRegex + destIdx, lexer->input + targetStart, targetLen);
            destIdx += targetLen;
            newRegex[destIdx++] = '|';
            newRegex[destIdx++] = ')';
        }
    }

    // Şablonun sağ tarafını (süslü parantezden sonrasını) kopyala
    strcpy(newRegex + destIdx, lexer->input + i + 1);

    // Orijinal lexer dizgisini değiştir ve pozisyonu hedef başlangıcına çek
    lexer->input = strdup(newRegex);
    lexer->length = strlen(newRegex);
    lexer->position = targetStart;
    
    free(newRegex);
}

// Lexer yapısının başlangıç değerlerini atar.
void initLexer(LexerContext* lexer, const char* input) {
    lexer->input = input;
    lexer->position = 0;
    lexer->length = strlen(input);
}

// Metndeki sıradaki karakteri okuyarak ilgili token'ı oluşturur ve döndürür.
Token getNextToken(LexerContext* lexer) {
    Token token;
    token.value = '\0';

    if (lexer->position >= lexer->length) {
        token.type = tokenEof;
        return token;
    }

    char currentChar = lexer->input[lexer->position];
    lexer->position++;

    // Eğer süslü parantez açıldıysa makroyu tetikle
    if (currentChar == '{') {
        lexer->position--; // { işaretine geri dön
        expandQuantifier(lexer); // Yeni şablonu çöz
        return getNextToken(lexer); // Yeni şablon üzerinden baştan oku
    }

    switch (currentChar) {
        case '*':
            token.type = tokenStar;
            break;
        case '+':
            token.type = tokenPlus;
            break;
        case '?':
            token.type = tokenQuestion;
            break;
        case '{':
            token.type = tokenLbrace;
            break;
        case '}':
            token.type = tokenRbrace;
            break;
        case '|':
            token.type = tokenPipe;
            break;
        case '(':
            token.type = tokenLparen;
            break;
        case ')':
            token.type = tokenRparen;
            break;
        case '[':
            token.type = tokenClass;
            memset(token.classMask, 0, 256); // Haritayı sıfırla
            token.isNegativeClass = false;

            // Negatif küme kontrolü [^...]
            if (lexer->position < lexer->length && lexer->input[lexer->position] == '^') {
                token.isNegativeClass = true;
                lexer->position++;
            }

            // ']' görene kadar oku
            while (lexer->position < lexer->length && lexer->input[lexer->position] != ']') {
                char c = lexer->input[lexer->position++];
                
                // Aralık kontrolü (Örn: a-z veya 0-9)
                if (lexer->position < lexer->length && lexer->input[lexer->position] == '-' &&
                    lexer->position + 1 < lexer->length && lexer->input[lexer->position + 1] != ']') {
                    lexer->position++; // '-' işaretini atla
                    char endC = lexer->input[lexer->position++];
                    for (int i = c; i <= endC; i++) {
                        token.classMask[i] = true; // Aralıktaki tüm harfleri haritada işaretle
                    }
                } else {
                    token.classMask[(unsigned char)c] = true; // Tekil harfi işaretle
                }
            }
            // ']' işaretini tüket
            if (lexer->position < lexer->length && lexer->input[lexer->position] == ']') {
                lexer->position++;
            }
            break;
        case '.':
            token.type = tokenDot;
            break;
        case '\\': // Kaçış karakteri yakalandı
            if (lexer->position < lexer->length) {
                char nextChar = lexer->input[lexer->position++];
                
                // Özel kaçış dizileri (\d, \w, \s) -> Doğrudan Kümeye çevirir
                if (nextChar == 'd' || nextChar == 'w' || nextChar == 's') {
                    token.type = tokenClass;
                    memset(token.classMask, 0, 256);
                    token.isNegativeClass = false;
                    
                    if (nextChar == 'd') { // Rakamlar
                        for (int i = '0'; i <= '9'; i++) token.classMask[i] = true;
                    } else if (nextChar == 'w') { // Kelime karakterleri (Harf, rakam ve altçizgi)
                        for (int i = 'a'; i <= 'z'; i++) token.classMask[i] = true;
                        for (int i = 'A'; i <= 'Z'; i++) token.classMask[i] = true;
                        for (int i = '0'; i <= '9'; i++) token.classMask[i] = true;
                        token.classMask['_'] = true;
                    } else if (nextChar == 's') { // Boşluk karakterleri
                        token.classMask[' '] = true;
                        token.classMask['\t'] = true;
                        token.classMask['\n'] = true;
                        token.classMask['\r'] = true;
                        token.classMask['\v'] = true;
                        token.classMask['\f'] = true;
                    }
                } else {
                    // Gerçek operatör karakterini arama (\*, \|, \. vb.)
                    // Slash'i yut, sadece sağındaki karakteri normal harfmiş gibi döndür
                    token.type = tokenChar;
                    token.value = nextChar;
                }
            } else {
                // Metnin en sonunda tek başına '\' kaldıysa normal karakter say
                token.type = tokenChar;
                token.value = '\\';
            }
            break;
        case '^':
            token.type = tokenCaret;
            break;
        case '$':
            token.type = tokenDollar;
            break;
        default:
            token.type = tokenChar;
            token.value = currentChar;
            break;
    }

    return token;
}