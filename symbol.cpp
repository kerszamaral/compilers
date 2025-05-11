#include "symbol.hpp"

#include <algorithm>
#include <vector>
#include <iostream>
#include <sstream>

// symbol.cpp file made by Ian Kersz Amaral - 2025/1


bool running = true;

std::vector<LineNumber> encounteredError;

SymbolTable symbolTable;

void stopRunning(void)
{
    running = false;
}

int isRunning(void)
{
    return running;
}

void setError(void)
{
    encounteredError.push_back(getLineNumber());
}

extern "C" int yywrap(void)
{
    stopRunning();
    if (encounteredError.size() > 0)
    {
        std::cerr << "\n!!Errors in the input file found in lines: ";
        for (const auto &line : encounteredError)
        {
            std::cerr << line;
            if (line != encounteredError.back())
            {
                std::cerr << ", ";
            }
        }
        std::cerr << "!!\n";
    }

    return 1;
}

void initMe(void)
{
    symbolTable = SymbolTable();
    encounteredError.clear();
}

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::pair<DataType, IdentType> symbol_to_data_type(const SymbolType symbol_type) {
    switch (symbol_type)
    {
    case SYMBOL_INVALID:
        return {TYPE_INVALID, IDENT_INVALID};
    case SYMBOL_IDENTIFIER:
        return {TYPE_UNINITIALIZED, IDENT_UNINITIALIZED};
    case SYMBOL_REAL:
        return {TYPE_REAL, IDENT_LIT};
    case SYMBOL_INT:
        return {TYPE_INT, IDENT_LIT};
    case SYMBOL_CHAR:
        return {TYPE_CHAR, IDENT_LIT};
    case SYMBOL_STRING:
        return {TYPE_STRING, IDENT_LIT};
    case SYMBOL_OTHER:
        return {TYPE_OTHER, IDENT_LIT};
    }
}
#pragma clang diagnostic pop

SymbolTableEntry register_symbol(const SymbolType symbol_type, Lexeme lexeme, LineNumber line_number)
{   
    // If we encounter numbers, we need to reverse them and remove the leading zeros
    if (symbol_type == SymbolType::SYMBOL_INT) {
        std::reverse(lexeme.begin(), lexeme.end());
#ifdef REMOVE_LEADING_ZEROS
        // Need to be careful with the case where the number is 0, as we would erase the whole string
        const auto firstNonZero = std::min(lexeme.find_first_not_of('0'), lexeme.size()-1);
        lexeme.erase(0, firstNonZero);
#endif
    } else if (symbol_type == SymbolType::SYMBOL_REAL) {
        // Same as above, but we need to treat the part before and after the slash separately
        const auto slashPos = lexeme.find('/');
        // We first reverse the number after the slash, and remove the leading zeros, so our slash position is correct after the reverse
        std::reverse(lexeme.begin() + static_cast<long>(slashPos) + 1, lexeme.end());
#ifdef REMOVE_LEADING_ZEROS
        const auto firstNonZero_after_slash = std::min(lexeme.find_first_not_of('0', slashPos + 1), lexeme.size()-1);
        lexeme.erase(slashPos + 1, firstNonZero_after_slash - slashPos - 1);
#endif

        std::reverse(lexeme.begin(), lexeme.begin() + static_cast<long>(slashPos));
#ifdef REMOVE_LEADING_ZEROS
        const auto firstNonZero_before_slash = std::min(lexeme.find_first_not_of('0'), static_cast<size_t>(slashPos)-1);
        lexeme.erase(0, firstNonZero_before_slash);
#endif
    }
    // If the key already exists, emplace does nothing, and returns an iterator to the existing element

    const auto [data_type, ident_type] = symbol_to_data_type(symbol_type);

    return symbolTable.emplace(lexeme, new Symbol{symbol_type, lexeme, line_number, data_type, ident_type, std::nullopt}).first->second; // We dereference the iterator to get the value as a reference
}

std::string Symbol::to_string() const
{
    std::stringstream ss;
    ss << "Symbol[";
    ss << symbolName(this->type);
    ss << ", ";
    ss << this->lexeme;
    ss << ", ";
    ss << std::to_string(this->line_number);
    ss << ", ";
    ss << data_type_to_str(this->data_type);
    ss << ", ";
    ss << ident_type_to_str(this->ident_type);
    ss << "]";

    return ss.str();
}

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string Symbol::get_text() const
{
    switch (this->type)
    {
    case SymbolType::SYMBOL_IDENTIFIER:
        return this->lexeme;
    case SymbolType::SYMBOL_REAL:
    {
        std::string result = this->lexeme;
        // Reverse the string to get the original number
        const auto slashPos = result.find('/');
        std::reverse(result.begin(), result.begin() + static_cast<long>(slashPos));
        std::reverse(result.begin() + static_cast<long>(slashPos) + 1, result.end());
        return result;
    }
    case SymbolType::SYMBOL_INT:
    {
        std::string result = this->lexeme;
        // Reverse the string to get the original number
        std::reverse(result.begin(), result.end());
        return result;
    }
    case SymbolType::SYMBOL_CHAR:
        return this->lexeme;
    case SymbolType::SYMBOL_STRING:
        return this->lexeme;
    case SymbolType::SYMBOL_OTHER:
        return this->lexeme;
    case SymbolType::SYMBOL_INVALID:
        return "SYMBOL_INVALID";
    }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string Symbol::get_type() const
{
    switch (this->type)
    {
    case SymbolType::SYMBOL_REAL:
        return "Real";
    case SymbolType::SYMBOL_INT:
        return "Integer";
    case SymbolType::SYMBOL_CHAR:
        return "Character";
    case SymbolType::SYMBOL_STRING:
        return "String";
    case SymbolType::SYMBOL_IDENTIFIER:
    case SymbolType::SYMBOL_OTHER:
    case SymbolType::SYMBOL_INVALID:
        return "";
    }
}
#pragma clang diagnostic pop

std::string generateSymbolTable(void)
{
    std::stringstream ss;
    for (auto &entry : symbolTable)
    {
        ss << entry.second->to_string() << std::endl;
    }
    return ss.str();
}

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string symbolName(SymbolType symbol) {
    switch (symbol) {
        case SymbolType::SYMBOL_INVALID:
            return "SYMBOL_INVALID";
        case SymbolType::SYMBOL_IDENTIFIER:
            return "SYMBOL_IDENTIFIER";
        case SymbolType::SYMBOL_REAL:
            return "SYMBOL_REAL";
        case SymbolType::SYMBOL_INT:
            return "SYMBOL_INT";
        case SymbolType::SYMBOL_CHAR:
            return "SYMBOL_CHAR";
        case SymbolType::SYMBOL_STRING:
            return "SYMBOL_STRING";
        case SymbolType::SYMBOL_OTHER:
            return "SYMBOL_OTHER";
    }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string data_type_to_str(const DataType data_type, bool user_friendly = false)
{
    if (user_friendly)
    {
        switch (data_type)
        {
        case TYPE_INVALID:
            return "Invalid";
        case TYPE_UNINITIALIZED:
            return "Uninitialized";
        case TYPE_INT:
            return "Integer";
        case TYPE_REAL:
            return "Real";
        case TYPE_CHAR:
            return "Character";
        case TYPE_STRING:
            return "String";
        case TYPE_BOOL:
            return "Boolean";
        case TYPE_OTHER:
            return "Other";
        }
    }
    else
    {
    switch (data_type)
    {
        case TYPE_INVALID:
            return "TYPE_INVALID";
        case TYPE_UNINITIALIZED:
            return "TYPE_UNINITIALIZED";
        case TYPE_INT:
            return "TYPE_INT";
        case TYPE_REAL:
            return "TYPE_REAL";
        case TYPE_CHAR:
            return "TYPE_CHAR";
        case TYPE_STRING:
            return "TYPE_STRING";
        case TYPE_BOOL:
            return "TYPE_BOOL";
        case TYPE_OTHER:
            return "TYPE_OTHER";
        }
    }
}

std::string ident_type_to_str(const IdentType ident_type, bool user_friendly = false)
{
    if (user_friendly)
    {
        switch (ident_type)
        {
        case IDENT_INVALID:
            return "Invalid";
        case IDENT_UNINITIALIZED:
            return "Uninitialized";
        case IDENT_FUNC:
            return "Function";
        case IDENT_VECTOR:
            return "Vector";
        case IDENT_VAR:
            return "Variable";
        case IDENT_LIT:
            return "Literal";
        }
    }
    else
    {
        switch (ident_type)
        {
        case IDENT_INVALID:
            return "IDENT_INVALID";
        case IDENT_UNINITIALIZED:
            return "IDENT_UNINITIALIZED";
        case IDENT_FUNC:
            return "IDENT_FUNC";
        case IDENT_VECTOR:
            return "IDENT_VECTOR";
        case IDENT_VAR:
            return "IDENT_VAR";
        case IDENT_LIT:
            return "IDENT_LIT";
        }
    }
}
#pragma clang diagnostic pop

DataType Symbol::get_data_type() const
{
    return this->data_type;
}


bool Symbol::set_types(DataType type, IdentType ident_type)
{
    if (this->data_type != TYPE_UNINITIALIZED && this->data_type != type)
    {
        return false;
    }
    this->ident_type = ident_type;
    this->data_type = type;
    return true;
}

bool Symbol::is_valid() const
{
    return this->type != SYMBOL_INVALID && this->data_type != TYPE_INVALID && this->data_type != TYPE_UNINITIALIZED;
}