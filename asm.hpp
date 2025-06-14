#pragma once
// asm.hpp file made by Ian Kersz Amaral - 2025/1

#include "symbol.hpp"
#include "tac.hpp"

std::string generate_asm(const TACList tac_list, const SymbolTable &symbol_table);
