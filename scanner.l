%{
// scanner.l file made by Ian Kersz Amaral - 00338368

#include "tokens.h"

extern bool inComment;

extern void add_token(int token, char *lexeme, int line);

%}

/* Activates Automatic Line Counting */
%option yylineno

%x COMMENT

digit [0-9]
alpha [A-z_]
white_space [ \t\n\r]
special_chars [-,;:\(\)\{\}\[\]+*\/%\<\>&\|~!=]
single_line_comment \/\/[^\n\r]+
identif {alpha}({alpha}|{digit})*
real {digit}+\/{digit}+
integer {digit}+
char '.?'
string \"([^\n\r"])*\"

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

{identif}               { return TK_IDENTIFIER; }

{real}                  { return LIT_REAL; }
{integer}               { return LIT_INT; }
{char}                  { return LIT_CHAR; }
{string}                { return LIT_STRING; }

"\/--"                  { inComment = true; BEGIN(COMMENT); }
{single_line_comment}   { }

.                       { return TOKEN_ERROR; }

<COMMENT>"--/"          { inComment = false; BEGIN(INITIAL); }
<COMMENT>.|\n           { }


%%

int getLineNumber(void)
{
    return yylineno;
}