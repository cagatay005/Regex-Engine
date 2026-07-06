#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "nfa.h"

// İşletim sistemine (Windows veya Linux/Mac) göre bekleme ve temizleme komutları
#ifdef _WIN32
    void __stdcall Sleep(unsigned long); 
    #define sleep_ms(ms) Sleep(ms)
    #define clear_screen() system("cls")
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
    #define clear_screen() system("clear")
#endif

#define COLOR_GREEN "\033[1;32m"
#define COLOR_RESET "\033[0m"

bool isEnglish = false;

// Uygulama başlığını ekrana basan fonksiyon
void printHeader() {
    clear_screen();
    printf("%s", COLOR_GREEN); // Yeşili Aktif Et!
    printf("==========================================\n");
    printf("            REGEX ENGINE\n");
    printf("==========================================\n\n");
}

// 'code' komutuyla çağrılan yardım menüsü
void printCommands() {
    printf("\n--- %s ---\n", isEnglish ? "COMMANDS" : "KOMUTLAR");
    if (isEnglish) {
        printf("  'code'  : Show this list\n");
        printf("  'clear' : Clean the terminal\n");
        printf("  'tr'    : Switch to Turkish\n");
        printf("  'q'     : Quit system\n");
    } else {
        printf("  'code'  : Bu listeyi gosterir\n");
        printf("  'clear' : Terminali temizler\n");
        printf("  'en'    : Ingilizce diline gecer\n");
        printf("  'q'     : Sistemden cikar\n");
    }
    printf("--------------------------\n");
}

int main() {
    char regexPattern[256];
    char text[1024];

    printHeader();

    while (1) {
        if (isEnglish) {
            printf("\nEnter Regex ('code' for cmds): ");
        } else {
            printf("\nRegex Deseni Girin ('code' ile komutlar): ");
        }
        
        if (fgets(regexPattern, sizeof(regexPattern), stdin) == NULL) break;
        regexPattern[strcspn(regexPattern, "\n")] = 0; 

        // Çıkış Komutu
        if (strcmp(regexPattern, "q") == 0) {
            if (isEnglish) {
                printf("\nExiting system...\n");
            } else {
                printf("\nSistemden cikiliyor...\n");
            }
            sleep_ms(1500); 
            break;          
        }

        // Ekranı Temizle Komutu
        if (strcmp(regexPattern, "clear") == 0) {
            printHeader();
            continue; 
        }

        // Ingilizceye Geç Komutu
        if (strcmp(regexPattern, "en") == 0) {
            isEnglish = true;
            printf(">>> Language changed to English.\n");
            continue;
        }

        // Türkçeye Geç Komutu
        if (strcmp(regexPattern, "tr") == 0) {
            isEnglish = false;
            printf(">>> Dil Turkce olarak degistirildi.\n");
            continue;
        }

        // Komutları Listele Komutu
        if (strcmp(regexPattern, "code") == 0) {
            printCommands();
            continue;
        }

        if (isEnglish) {
            printf("Enter Text to Match: ");
        } else {
            printf("Eslesecek Metni Girin: ");
        }
        
        if (fgets(text, sizeof(text), stdin) == NULL) break;
        text[strcspn(text, "\n")] = 0;

        NfaContext* ctx = createNfa(regexPattern);
        bool result = matchNfa(ctx, text);

        if (result) {
            if (isEnglish) {
                printf("RESULT: MATCHED\n");
            } else {
                printf("SONUC: ESLESTI\n");
            }
        } else {
            if (isEnglish) {
                printf("RESULT: NO MATCH\n");
            } else {
                printf("SONUC: ESLESMEDI\n");
            }
        }
    }

    // Uygulama kapanırken terminalin rengini varsayılana döndür
    printf("%s", COLOR_RESET);
    return 0;
}