#include "scanner.hpp"

#include <algorithm>
#include <vector>

// scanner.cpp file made by Ian Kersz Amaral - 2025/1


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
        fprintf(stderr, "\n!!Errors in the input file found in lines: ");
        for (const auto &line : encounteredError)
        {
            fprintf(stderr, "%d", line);
            if (line != encounteredError.back())
            {
                fprintf(stderr, ", ");
            }
        }
        fprintf(stderr, "!!\n");
    }

    return 1;
}

void initMe(void)
{
    symbolTable = SymbolTable();
    encounteredError.clear();
}

SymbolTableEntry &register_symbol(const SymbolType symbol_type, Lexeme lexeme)
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
    return symbolTable.emplace(lexeme, SymbolTableEntry(symbol_type, lexeme)).first->second; // We dereference the iterator to get the value as a reference
}

void printSymbolTable(void)
{
    for (auto &entry : symbolTable)
    {
        const auto symbol = std::get<0>(entry.second);
        const auto lexeme = std::get<1>(entry.second);
        fprintf(stderr, "Symbol[%s, %s]\n", symbolName(symbol).c_str(), lexeme.c_str());
    }
}

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
        default:
            return "UNKNOWN";
    }
}

std::string tokenName(TokenType token)
{
    switch (token)
    {
    case KW_BYTE:
        return "KW_BYTE";
    case KW_INT:
        return "KW_INT";
    case KW_REAL:
        return "KW_REAL";
    case KW_IF:
        return "KW_IF";
    case KW_ELSE:
        return "KW_ELSE";
    case KW_DO:
        return "KW_DO";
    case KW_WHILE:
        return "KW_WHILE";
    case KW_READ:
        return "KW_READ";
    case KW_PRINT:
        return "KW_PRINT";
    case KW_RETURN:
        return "KW_RETURN";
    case OPERATOR_LE:
        return "OPERATOR_LE";
    case OPERATOR_GE:
        return "OPERATOR_GE";
    case OPERATOR_EQ:
        return "OPERATOR_EQ";
    case OPERATOR_DIF:
        return "OPERATOR_DIF";
    case TK_IDENTIFIER:
        return "TK_IDENTIFIER";
    case LIT_INT:
        return "LIT_INT";
    case LIT_CHAR:
        return "LIT_CHAR";
    case LIT_REAL:
        return "LIT_REAL";
    case LIT_STRING:
        return "LIT_STRING";
    case TOKEN_ERROR:
        return "TOKEN_ERROR";
    default:
        return "UNKNOWN";
    }
}
