#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/nfa.h"
#include "../include/matcher.h"

// Terminal için ANSI Renk Kodları
#define COLOR_MATRIX_GREEN "\033[1;32m"
#define COLOR_RESET "\033[0m"

// fgets ile alınan metnin sonundaki alt satır (\n) karakterini temizler
void stripNewline(char* str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int main() {
    char regexPattern[256];
    char text[256];

    // Terminal boyama ve baslik
    printf("%s", COLOR_MATRIX_GREEN);
    printf("==========================================\n");
    printf("               REGEX ENGINE\n");
    printf("==========================================\n\n");

    while (1) {
        printf("Regex Deseni Girin (Cikmak icin 'q'): ");
        if (fgets(regexPattern, sizeof(regexPattern), stdin) == NULL) break;
        stripNewline(regexPattern);

        if (strcmp(regexPattern, "q") == 0) {
            break;
        }

        printf("Eslesecek Metni Girin: ");
        if (fgets(text, sizeof(text), stdin) == NULL) break;
        stripNewline(text);

        NfaContext* nfa = createNfa(regexPattern);
        bool result = matchNfa(nfa, text);
        
        if (result) {
            printf("SONUC: ESLESTI\n\n");
        } else {
            printf("SONUC: ESLESMEDI\n\n");
        }

        freeNfa(nfa);
    }

    printf("Sistemden cikiliyor...\n");
    
    // Uygulama kapanırken terminal rengini varsayılana döndürür
    printf("%s", COLOR_RESET);
    
    return 0;
}