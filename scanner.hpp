#pragma once
// scanner.hpp file made by Ian Kersz Amaral - 2025/1

#include <cstdbool>
#include <cstdint>
#include <string>
#include <tuple>
#include <map>
#include <memory>

#include "parser.tab.hpp"

#define REMOVE_LEADING_ZEROS

#define printf(...) fprintf(stderr, __VA_ARGS__)

enum SymbolType : uint8_t
{
    SYMBOL_INVALID,
    SYMBOL_IDENTIFIER,
    SYMBOL_REAL,
    SYMBOL_INT,
    SYMBOL_CHAR,
    SYMBOL_STRING,
    SYMBOL_OTHER
};

typedef yytoken_kind_t TokenType;
typedef unsigned int LineNumber;
typedef std::string Lexeme;
typedef std::tuple<SymbolType, Lexeme, LineNumber> Symbol;
typedef std::shared_ptr<Symbol> SymbolTableEntry;
typedef std::map<Lexeme, SymbolTableEntry> SymbolTable;

void initMe(void);

void stopRunning(void);

void setError(void);

LineNumber getLineNumber(void);

std::string symbolName(SymbolType symbol);

std::string tokenName(TokenType token);

SymbolTableEntry register_symbol(const SymbolType symbol_type, Lexeme lexeme, LineNumber line_number);

std::string SymbolEntryString(const SymbolTableEntry &entry);

void printSymbolTable(void);
