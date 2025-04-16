%{
// scanner.l file made by Ian Kersz - 2025/1

#include "scanner.hpp"
#include "ast.hpp"

#include "parser.tab.hpp"

typedef yy::parser::token_type TokenType;

#define YY_DECL yy::parser::symbol_type yylex()

%}

/* Activates Automatic Line Counting */
%option yylineno

/* Removes the unused functions, reducing warnings */
%option noinput nounput

%x COMMENT

digit [0-9]
alpha [A-Za-z_]
white_space [ \t\n\r]
special_chars [-,;:\(\)\{\}\[\]+*\/%\<\>&\|~=]
single_line_comment \/\/[^\n\r]*
identif {alpha}({alpha}|{digit})*
real {digit}+\/{digit}+
integer {digit}+
char '.?'
string \"([^\n\r"]|(\\\"))*\"

%%

{white_space}           { }

"byte"                  { return yy::parser::make_KW_BYTE(); }
"int"                   { return yy::parser::make_KW_INT(); }
"real"                  { return yy::parser::make_KW_REAL(); }
"if"                    { return yy::parser::make_KW_IF(); }
"else"                  { return yy::parser::make_KW_ELSE(); }
"do"                    { return yy::parser::make_KW_DO(); }
"while"                 { return yy::parser::make_KW_WHILE(); }
"read"                  { return yy::parser::make_KW_READ(); }
"print"                 { return yy::parser::make_KW_PRINT(); }
"return"                { return yy::parser::make_KW_RETURN(); }
"<="                    { return yy::parser::make_OPERATOR_LE(); }
">="                    { return yy::parser::make_OPERATOR_GE(); }
"=="                    { return yy::parser::make_OPERATOR_EQ(); }
"!="                    { return yy::parser::make_OPERATOR_DIF(); }
{special_chars}         { return yy::parser::symbol_type(yytext[0]); }

{identif}               { register_symbol(SymbolType::SYMBOL_IDENTIFIER, yytext, getLineNumber()); return yy::parser::make_TK_IDENTIFIER(); }

{real}                  { register_symbol(SymbolType::SYMBOL_REAL, yytext, getLineNumber()); return yy::parser::make_LIT_REAL();  }
{integer}               { register_symbol(SymbolType::SYMBOL_INT, yytext, getLineNumber()); return yy::parser::make_LIT_INT(); }
{char}                  { register_symbol(SymbolType::SYMBOL_CHAR, yytext, getLineNumber()); return yy::parser::make_LIT_CHAR();}
{string}                { register_symbol(SymbolType::SYMBOL_STRING, yytext, getLineNumber()); return yy::parser::make_LIT_STRING(); }

"\/--"                  { BEGIN(COMMENT); }
{single_line_comment}   { }

.                       { setError(); return yy::parser::make_TOKEN_ERROR(); }

<COMMENT>"--/"          { BEGIN(INITIAL); }
<COMMENT><<EOF>>        { BEGIN(INITIAL); return yy::parser::make_TOKEN_ERROR();};
<COMMENT>.|\n              { }


%%

LineNumber getLineNumber(void)
{
    return static_cast<LineNumber>(yylineno);
}
