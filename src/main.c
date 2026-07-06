#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "nfa.h"

// İşletim sistemine (Windows veya Linux/Mac) göre bekleme ve temizleme komutlarını ayarla
#ifdef _WIN32
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
    #define clear_screen() system("cls")
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
    #define clear_screen() system("clear")
#endif

// Dil seçeneğini tutan global değişken (Varsayılan: Türkçe)
bool isEnglish = false;

// Uygulama başlığını ekrana basan yardımcı fonksiyon
void printHeader() {
    clear_screen();
    printf("==========================================\n");
    printf("            REGEX ENGINE\n");
    printf("==========================================\n\n");
}

int main() {
    char regexPattern[256];
    char text[1024];

    printHeader();

    while (1) {
        // Dile göre input mesajı
        if (isEnglish) {
            printf("\nEnter Regex ('q': quit, 'clear': clean, 'tr': Turkish): ");
        } else {
            printf("\nRegex Deseni Girin ('q': cikis, 'clear': temizle, 'en': Ingilizce): ");
        }
        
        if (fgets(regexPattern, sizeof(regexPattern), stdin) == NULL) break;
        regexPattern[strcspn(regexPattern, "\n")] = 0; // Yeni satır karakterini temizle

        // Çıkış Komutu
        if (strcmp(regexPattern, "q") == 0) {
            if (isEnglish) {
                printf("\nExiting system...\n");
            } else {
                printf("\nSistemden cikiliyor...\n");
            }
            sleep_ms(1500); // 1.5 saniye (1500 milisaniye) bekle
            break;          // Döngüyü kır ve programı kapat
        }

        // Ekranı Temizle Komutu
        if (strcmp(regexPattern, "clear") == 0) {
            printHeader();
            continue; // Döngünün başına dön
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

        // Dile göre metin girişi mesajı
        if (isEnglish) {
            printf("Enter Text to Match: ");
        } else {
            printf("Eslesecek Metni Girin: ");
        }
        
        if (fgets(text, sizeof(text), stdin) == NULL) break;
        text[strcspn(text, "\n")] = 0;

        // --- REGEX MOTORUNU ÇALIŞTIRIR ---
        NfaContext* ctx = createNfa(regexPattern);
        bool result = matchNfa(ctx, text);

        // Dile göre sonuç çıktısı
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

    return 0;
}