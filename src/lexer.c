#include "lexer.h"
#include <string.h>

// Lexer yapısının başlangıç değerlerini atar.
void initLexer(LexerContext* lexer, const char* input) {
    lexer->input = input;
    lexer->position = 0;
    lexer->length = strlen(input);
}

// Metindeki sıradaki karakteri okuyarak ilgili token'ı oluşturur ve döndürür.
Token getNextToken(LexerContext* lexer) {
    Token token;
    token.value = '\0';

    if (lexer->position >= lexer->length) {
        token.type = tokenEof;
        return token;
    }

    char currentChar = lexer->input[lexer->position];
    lexer->position++;

    switch (currentChar) {
        case '*':
            token.type = tokenStar;
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
        default:
            token.type = tokenChar;
            token.value = currentChar;
            break;
    }

    return token;
}