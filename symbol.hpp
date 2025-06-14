#pragma once
// symbol.hpp file made by Ian Kersz Amaral - 2025/1

#include <cstdbool>
#include <cstdint>
#include <string>
#include <tuple>
#include <map>
#include <memory>
#include <optional>
#include <vector>
#include <functional>

#define REMOVE_LEADING_ZEROS

struct Node;

enum SymbolType : uint8_t
{
    SYMBOL_INVALID,
    SYMBOL_IDENTIFIER,
    SYMBOL_REAL,
    SYMBOL_INT,
    SYMBOL_CHAR,
    SYMBOL_STRING,
    SYMBOL_OTHER,
    SYMBOL_TEMP,
    SYMBOL_LABEL
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
    IDENT_PARAM,
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
    std::optional<std::shared_ptr<Node>> node;

    std::string to_string() const;
    std::string get_original_text() const;
    std::string get_text() const;
    std::string get_type() const;
    DataType get_data_type() const;

    bool set_types(DataType type, IdentType ident_type);

    bool is_valid() const;

    bool set_node(std::shared_ptr<Node> node)
    {
        this->node = node;
        return true;
    }

    std::optional<std::shared_ptr<Node>> get_node() const
    {
        return this->node;
    }
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
SymbolTableEntry register_temp(DataType data_type = TYPE_OTHER);
SymbolTableEntry register_label();

std::string generateSymbolTable(void);

const SymbolTable &get_symbol_table(void);

const std::vector<SymbolTableEntry> filtered_table_entries(const SymbolTable &symbol_table, const std::function<bool(const SymbolTableEntry &)> &filter);

void erase_symbol(const Lexeme &symbol);

void erase_symbol(const SymbolTableEntry &symbol);