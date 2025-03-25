#pragma once
// scanner.h file made by Ian Kersz Amaral - 2025/1

#include <cstdbool>
#include <string>
#include <tuple>
#include <map>

#include "tokens.h"

#define printf(...) fprintf(stderr, __VA_ARGS__)

void initMe(void);

void stopRunning(void);

void setInComment(bool value);

int getLineNumber(void);

typedef int TokenType;
typedef unsigned int LineNumber;
typedef std::string Lexeme;
typedef std::tuple<TokenType, Lexeme> SymbolTableEntry;
typedef std::map<Lexeme, SymbolTableEntry> SymbolTable;

std::string tokenName(TokenType token);

SymbolTableEntry &add_token(const TokenType token_type, const Lexeme &lexeme);

void printSymbolTable(void);
