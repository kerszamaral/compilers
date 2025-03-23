#pragma once
// scanner.h file made by Ian Kersz Amaral

#include <stdbool.h>
#include <string>
#include <map>


#include "tokens.h"

#define printf(...) fprintf(stderr, __VA_ARGS__)

bool running = true;

int getLineNumber(void);

typedef int TokenType;
typedef unsigned int LineNumber;
typedef std::tuple<TokenType, LineNumber> SymbolTableEntry;
typedef std::string Lexeme;
typedef std::map<Lexeme, SymbolTableEntry> SymbolTable;

SymbolTable symbolTable;

std::string tokenName(TokenType token);

void add_token(int token, char *lexeme, int line);
