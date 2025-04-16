#pragma once
// scanner.hpp file made by Ian Kersz Amaral - 2025/1

#include <cstdbool>
#include <cstdint>
#include <string>
#include <tuple>
#include <map>
#include <memory>

#define REMOVE_LEADING_ZEROS

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

typedef unsigned int LineNumber;
typedef std::string Lexeme;

typedef struct Symbol
{
    SymbolType type;
    Lexeme lexeme;
    LineNumber line_number;

    std::string to_string() const;
} Symbol;

typedef std::shared_ptr<Symbol> SymbolTableEntry;
typedef std::map<Lexeme, SymbolTableEntry> SymbolTable;

void initMe(void);

void stopRunning(void);

void setError(void);

LineNumber getLineNumber(void);

std::string symbolName(SymbolType symbol);

SymbolTableEntry register_symbol(const SymbolType symbol_type, Lexeme lexeme, LineNumber line_number);

void printSymbolTable(void);
