%{
// scanner.l file made by Ian Kersz - 2025/1

#include "scanner.hpp"

%}

/* Activates Automatic Line Counting */
%option yylineno

/* Removes the unused functions, reducing warnings */
%option noinput nounput

%x COMMENT

digit [0-9]
alpha [A-Za-z_]
white_space [ \t\n\r]
special_chars [-,;:\(\)\{\}\[\]+*\/%\<\>&\|~!=]
single_line_comment \/\/[^\n\r]+
identif {alpha}({alpha}|{digit})*
real {digit}+\/{digit}+
integer {digit}+
char '.?'
string \"([^\n\r"]|(\\\"))*\"

%%

{white_space}           { }

"byte"                  { return TokenType::KW_BYTE; }
"int"                   { return TokenType::KW_INT; }
"real"                  { return TokenType::KW_REAL; }
"if"                    { return TokenType::KW_IF; }
"else"                  { return TokenType::KW_ELSE; }
"do"                    { return TokenType::KW_DO; }
"while"                 { return TokenType::KW_WHILE; }
"read"                  { return TokenType::KW_READ; }
"print"                 { return TokenType::KW_PRINT; }
"return"                { return TokenType::KW_RETURN; }
"<="                    { return TokenType::OPERATOR_LE; }
">="                    { return TokenType::OPERATOR_GE; }
"=="                    { return TokenType::OPERATOR_EQ; }
"!="                    { return TokenType::OPERATOR_DIF; }
{special_chars}         { return yytext[0]; }

{identif}               { register_symbol(SymbolType::SYMBOL_IDENTIFIER, yytext); return TokenType::TK_IDENTIFIER; }

{real}                  { register_symbol(SymbolType::SYMBOL_REAL, yytext); return TokenType::LIT_REAL; }
{integer}               { register_symbol(SymbolType::SYMBOL_INT, yytext); return TokenType::LIT_INT; }
{char}                  { register_symbol(SymbolType::SYMBOL_CHAR, yytext); return TokenType::LIT_CHAR; }
{string}                { register_symbol(SymbolType::SYMBOL_STRING, yytext); return TokenType::LIT_STRING; }

"\/--"                  { setInComment(true); BEGIN(COMMENT); }
{single_line_comment}   { }

.                       { setError(); return TokenType::TOKEN_ERROR; }

<COMMENT>"--/"          { setInComment(false); BEGIN(INITIAL); }
<COMMENT>.|\n           { }


%%

int getLineNumber(void)
{
    return yylineno;
}