%{
// scanner.l file made by Ian Kersz - 2025/1

#include "symbol.hpp"
#include "ast.hpp"

#include "parser.tab.hpp"

typedef yy::parser::token_type TokenType;

#define YY_DECL yy::parser::symbol_type yylex(yy::parser::location_type& yylloc)

#undef yyterminate
#define yyterminate() return yy::parser::make_YYEOF(yylloc)

#define YY_USER_ACTION \
    yylloc.step(); \
    yylloc.columns(yyleng);

extern SymbolTable g_symbolTable;

%}

/* Activates Automatic Line Counting */
%option yylineno

/* Removes the unused functions, reducing warnings */
%option noinput nounput

%x COMMENT

digit [0-9]
alpha [A-Za-z_]
white_space [ \t\r]
special_chars [-,;:\(\)\{\}\[\]+*\/%\<\>&\|~=]
single_line_comment \/\/[^\n\r]*
identif {alpha}({alpha}|{digit})*
real {digit}+\/{digit}+
integer {digit}+
char '.?'
string \"([^\n\r"]|(\\\"))*\"

%%

{white_space}           { }
"\n"                    { yylloc.lines(1); }

"byte"                  { return yy::parser::make_KW_BYTE(yylloc); }
"int"                   { return yy::parser::make_KW_INT(yylloc); }
"real"                  { return yy::parser::make_KW_REAL(yylloc); }
"if"                    { return yy::parser::make_KW_IF(yylloc); }
"else"                  { return yy::parser::make_KW_ELSE(yylloc); }
"do"                    { return yy::parser::make_KW_DO(yylloc); }
"while"                 { return yy::parser::make_KW_WHILE(yylloc); }
"read"                  { return yy::parser::make_KW_READ(yylloc); }
"print"                 { return yy::parser::make_KW_PRINT(yylloc); }
"return"                { return yy::parser::make_KW_RETURN(yylloc); }
"<="                    { return yy::parser::make_OPERATOR_LE(yylloc); }
">="                    { return yy::parser::make_OPERATOR_GE(yylloc); }
"=="                    { return yy::parser::make_OPERATOR_EQ(yylloc); }
"!="                    { return yy::parser::make_OPERATOR_DIF(yylloc); }
{special_chars}         { return yy::parser::symbol_type(yytext[0], yylloc); }

{identif}               { return yy::parser::make_TK_IDENTIFIER(register_symbol(g_symbolTable, SymbolType::SYMBOL_IDENTIFIER, yytext, getLineNumber()), yylloc); }

{real}                  { return yy::parser::make_LIT_REAL(register_symbol(g_symbolTable, SymbolType::SYMBOL_REAL, yytext, getLineNumber()), yylloc); }
{integer}               { return yy::parser::make_LIT_INT(register_symbol(g_symbolTable, SymbolType::SYMBOL_INT, yytext, getLineNumber()), yylloc); }
{char}                  { return yy::parser::make_LIT_CHAR(register_symbol(g_symbolTable, SymbolType::SYMBOL_CHAR, yytext, getLineNumber()), yylloc); }
{string}                { return yy::parser::make_LIT_STRING(register_symbol(g_symbolTable, SymbolType::SYMBOL_STRING, yytext, getLineNumber()), yylloc); }

"\/--"                  { BEGIN(COMMENT); }
{single_line_comment}   { }

.                       { setError(); return yy::parser::make_TOKEN_ERROR(yylloc); }

<COMMENT>"--/"          { BEGIN(INITIAL); }
<COMMENT><<EOF>>        { BEGIN(INITIAL); return yy::parser::make_TOKEN_ERROR(yylloc);};
<COMMENT>.|\n           { }


%%

LineNumber getLineNumber(void)
{
    return static_cast<LineNumber>(yylineno);
}
