#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Lexer'ın okuyabileceği sembol tiplerini belirler
typedef enum {
    tokenChar,
    tokenStar,
    tokenPlus,
    tokenQuestion,
    tokenPipe,
    tokenLparen,
    tokenRparen,
    tokenLbrace,
    tokenRbrace,
    tokenClass,
    tokenEof
} TokenType;

// Tek bir sembolü ve gerekirse harf değerini tutar
typedef struct {
    TokenType type;
    char value;
    bool classMask[256];
    bool isNegativeClass;
} Token;

// Lexer'ın metin üzerindeki anlık durumunu tutar
typedef struct {
    const char* input;
    size_t position;
    size_t length;
} LexerContext;

// Lexer yapısının başlangıç değerlerini atar
void initLexer(LexerContext* lexer, const char* input);

// Metndeki sıradaki karakteri okuyarak ilgili token'ı oluşturur ve döndürür
Token getNextToken(LexerContext* lexer);

#endif