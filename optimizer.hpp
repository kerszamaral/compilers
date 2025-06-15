#pragma once

#include "tac.hpp"
#include "symbol.hpp"
#include <tuple>

std::tuple<TACList, SymbolTable, std::string> optimize(TACList tac_list, const SymbolTable &original_symbol_table);