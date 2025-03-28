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

enum SymbolType : uint8_t
{
    SYMBOL_INVALID,
    SYMBOL_IDENTIFIER,
    SYMBOL_REAL,
    SYMBOL_INT,
    SYMBOL_CHAR,
    SYMBOL_STRING
};

typedef int TokenType;
typedef unsigned int LineNumber;
typedef std::string Lexeme;
typedef std::tuple<SymbolType, Lexeme> SymbolTableEntry;
typedef std::map<Lexeme, SymbolTableEntry> SymbolTable;

std::string symbolName(SymbolType symbol);

std::string tokenName(TokenType token);

SymbolTableEntry &register_symbol(const SymbolType symbol_type, Lexeme lexeme);

void printSymbolTable(void);
