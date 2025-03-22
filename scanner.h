#pragma once
// scanner.h file made by Ian Kersz Amaral

#include <stdbool.h>
#include <string>
#include <map>


#include "tokens.h"

#define printf(...) fprintf(stderr, __VA_ARGS__)

bool running = true;

int getLineNumber(void);

typedef int TokenType;

std::map<std::string, TokenType> symbolTable;

std::string tokenName(TokenType token);
