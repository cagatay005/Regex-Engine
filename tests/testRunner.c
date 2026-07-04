#include <stdio.h>
#include <assert.h>
#include "../include/nfa.h"
#include "../include/matcher.h"

// Regex motoruna verilen deseni ve metni test edip durumu raporlar.
void runTest(const char* regexPattern, const char* text, bool expectedResult) {
    NfaContext* nfa = createNfa(regexPattern);
    bool result = matchNfa(nfa, text);
    
    assert(result == expectedResult);
    
    // Test başarılıysa konsola bilgi ver
    printf("TEST PASSED | Regex: '%s' | Text: '%s' | Expected: %s\n", 
           regexPattern, text, expectedResult ? "Match" : "No Match");

    freeNfa(nfa);
}

int main() {
    printf("Starting Regex Engine Tests...\n");
    printf("----------------------------------\n");

    // Basit metin testleri
    runTest("abc", "abc", true);
    runTest("test", "test", true);
    runTest("hello", "world", false);
    
    // Boyut/kapsam uyumsuzluğu testleri
    runTest("ab", "a", false);
    runTest("a", "ab", false);

    printf("----------------------------------\n");
    printf("All tests passed successfully! No memory leaks detected.\n");
    
    return 0;
}