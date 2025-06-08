#pragma once

#include "symbol.hpp"
#include "tac.hpp"

std::string generate_asm(const TACList tac_list, const SymbolTable &symbol_table);
