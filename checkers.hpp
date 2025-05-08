#pragma once

#include "semantic.hpp"
#include "ast.hpp"

SemanticAnalyzer check_declarations(NodePtr node);

std::pair<size_t, std::string> run_semantic_analysis(NodePtr node);
