#pragma once

#include "semantic.hpp"
#include "ast.hpp"

SemanticAnalyzer check_declarations(NodePtr node);

SemanticAnalyzer check_uses(NodePtr node);

SemanticAnalyzer check_types(NodePtr node);

SemanticAnalyzer check_arguments(NodePtr node);

SemanticAnalyzer check_return(NodePtr node);

std::pair<size_t, std::string> run_semantic_analysis(NodePtr node);
