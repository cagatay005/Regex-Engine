#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    tokenChar,
    tokenStar,
    tokenPipe,
    tokenLparen,
    tokenRparen,
    tokenEof
} TokenType;

typedef struct {
    TokenType type;
    char value; // Sadece tokenChar için anlamlıdır
} Token;

typedef struct {
    const char* input;
    size_t position;
    size_t length;
} LexerContext;

// Lexer durumunu varsayılan değerlerle başlatır
void initLexer(LexerContext* lexer, const char* input);

// Metindeki sıradaki karakteri ayrıştırıp ilgili token yapısını döndürür
Token getNextToken(LexerContext* lexer);

#endif