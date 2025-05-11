#pragma once
// symbol.hpp file made by Ian Kersz Amaral - 2025/1

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

enum DataType : uint8_t
{
    TYPE_INVALID,
    TYPE_UNINITIALIZED,
    TYPE_INT,
    TYPE_REAL,
    TYPE_CHAR,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_OTHER
};

enum IdentType : uint8_t
{
    IDENT_INVALID,
    IDENT_UNINITIALIZED,
    IDENT_FUNC,
    IDENT_VECTOR,
    IDENT_VAR,
    IDENT_LIT,
};

typedef unsigned int LineNumber;
typedef std::string Lexeme;

typedef struct Symbol
{
    SymbolType type;
    Lexeme lexeme;
    LineNumber line_number;
    DataType data_type;
    IdentType ident_type;

    std::string to_string() const;
    std::string get_text() const;
    std::string get_type() const;
    DataType get_data_type() const;

    bool set_types(DataType type, IdentType ident_type);

    bool is_valid() const;
} Symbol;

typedef std::shared_ptr<Symbol> SymbolTableEntry;
typedef std::map<Lexeme, SymbolTableEntry> SymbolTable;

void initMe(void);

void stopRunning(void);

void setError(void);

LineNumber getLineNumber(void);

std::string symbolName(SymbolType symbol);
std::string data_type_to_str(const DataType data_type, bool user_friendly = false);
std::string ident_type_to_str(const IdentType ident_type, bool user_friendly = false);

SymbolTableEntry register_symbol(const SymbolType symbol_type, Lexeme lexeme, LineNumber line_number);

std::string generateSymbolTable(void);
