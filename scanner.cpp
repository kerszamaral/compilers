#include "scanner.h"

// scanner.cpp file made by Ian Kersz Amaral - 2025/1

bool inComment = false;

bool running = true;

SymbolTable symbolTable;

void stopRunning(void)
{
    running = false;
}

int isRunning(void)
{
    return running;
}

void setInComment(bool value)
{
    inComment = value;
}

extern "C" int yywrap(void)
{
    stopRunning();
    if (inComment)
    {
        fprintf(stderr, "\n!!Unclosed comment!!\n");
    }
    return 1;
}

void initMe(void)
{
    symbolTable = SymbolTable();
}

SymbolTableEntry &add_token(const TokenType token_type, const Lexeme &lexeme)
{
    // If the key already exists, emplace does nothing, and returns an iterator to the existing element
    return symbolTable.emplace(lexeme, SymbolTableEntry(token_type, lexeme)).first->second; // We dereference the iterator to get the value as a reference
}

void printSymbolTable(void)
{
    for (auto &entry : symbolTable)
    {
        fprintf(stderr, "Lexeme: %s\n", entry.first.c_str());
        fprintf(stderr, "Token: %s\n", tokenName(std::get<0>(entry.second)).c_str());
        fprintf(stderr, "Lexeme: %s\n", std::get<1>(entry.second).c_str());
        fprintf(stderr, "\n");
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
    case '-':
    case ',':
    case ';':
    case ':':
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case '+':
    case '*':
    case '/':
    case '%':
    case '<':
    case '>':
    case '&':
    case '|':
    case '~':
    case '=':
    case '!':
        return std::string(1, static_cast<char>(token));
    default:
        return "UNKNOWN";
    }
}
