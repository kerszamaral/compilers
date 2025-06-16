#pragma once

// optimizer.hpp file made by Ian Kersz Amaral - 2025/1

#include "tac.hpp"
#include "symbol.hpp"
#include <tuple>

std::tuple<TACList, SymbolTable, std::string> optimize(TACList tac_list, const SymbolTable &original_symbol_table);