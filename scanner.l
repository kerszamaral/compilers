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

"byte"                  { return KW_BYTE; }
"int"                   { return KW_INT; }
"real"                  { return KW_REAL; }
"if"                    { return KW_IF; }
"else"                  { return KW_ELSE; }
"do"                    { return KW_DO; }
"while"                 { return KW_WHILE; }
"read"                  { return KW_READ; }
"print"                 { return KW_PRINT; }
"return"                { return KW_RETURN; }
"<="                    { return OPERATOR_LE; }
">="                    { return OPERATOR_GE; }
"=="                    { return OPERATOR_EQ; }
"!="                    { return OPERATOR_DIF; }
{special_chars}         { return yytext[0]; }

{identif}               { register_symbol(SymbolType::SYMBOL_IDENTIFIER, yytext); return TK_IDENTIFIER; }

{real}                  { register_symbol(SymbolType::SYMBOL_REAL, yytext); return LIT_REAL; }
{integer}               { register_symbol(SymbolType::SYMBOL_INT, yytext); return LIT_INT; }
{char}                  { register_symbol(SymbolType::SYMBOL_CHAR, yytext); return LIT_CHAR; }
{string}                { register_symbol(SymbolType::SYMBOL_STRING, yytext); return LIT_STRING; }

"\/--"                  { setInComment(true); BEGIN(COMMENT); }
{single_line_comment}   { }

.                       { setError(); return TOKEN_ERROR; }

<COMMENT>"--/"          { setInComment(false); BEGIN(INITIAL); }
<COMMENT>.|\n           { }


%%

int getLineNumber(void)
{
    return yylineno;
}