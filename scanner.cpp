#include "scanner.h"

// scanner.cpp file made by Ian Kersz Amaral

bool inComment = false;

extern int getLineNumber(void);

extern "C" int yywrap(void)
{
    running = false;
    if (inComment)
    {
        fprintf(stderr, "\n!!Unclosed comment!!\n");
        return 1;
    }
    return 1;
}

void initMe(void)
{
    symbolTable = SymbolTable();
}

void add_token(int token, char *lexeme, int line)
{
    Lexeme lex = Lexeme(lexeme);
    SymbolTableEntry entry = SymbolTableEntry(token, line);
    symbolTable.emplace(lex, entry); // If the key already exists, emplace does nothing
}

int isRunning(void)
{
    return running;
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
