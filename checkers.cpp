#include "checkers.hpp"

#include <sstream>
#include <iostream>
#include <optional>
#include <limits>

typedef std::function<SemanticAnalyzer(NodePtr)> Checker;

std::pair<size_t, std::string> run_semantic_analysis(NodePtr node)
{
    const std::vector<Checker> checkers{
        check_declarations,
        // check_uses,
        // check_types,
        // check_arguments,
        // check_return,
    };

    std::vector<SemanticAnalyzer> analyzers;

    for (const auto &checker : checkers)
    {
        analyzers.push_back(checker(node));
    }

    std::stringstream ss;
    size_t number_of_errors = 0;
    for (const auto &analyzer : analyzers)
    {
        ss << analyzer.generate_error_messages();
        number_of_errors += analyzer.error_count();
    }

    return {number_of_errors, ss.str()};
}

constexpr ptrdiff_t SKIP_NONE = 0;
constexpr ptrdiff_t SKIP_ALL = std::numeric_limits<ptrdiff_t>::max();

// Helper: sets the optional only if it's not already set
template<typename T, typename U>
void set_if_unset(std::optional<T>& opt, U&& value) {
    if (!opt) {
        opt = T(std::forward<U>(value));
    }
}

ptrdiff_t declaration_checker(SemanticAnalyzer& analyzer, const NodeType node_type, const NodeList& children)
{
    std::optional<std::string> redec_of;
    std::optional<IdentType> ident_type;
    switch (node_type)
    {
    // Mark as declared and check redeclaration
    case NODE_VAR_DECL: // type: 0, symbol: 1, init_val: 2
        set_if_unset(redec_of, "Variable");
        set_if_unset(ident_type, IDENT_VAR);
    case NODE_VEC_DEF: // type: 0, symbol: 1, size: 2
        set_if_unset(redec_of, "Vector");
        set_if_unset(ident_type, IDENT_VECTOR);
    case NODE_FUN_DECL: // ret_type: 0, symbol: 1
        set_if_unset(redec_of, "Function");
        set_if_unset(ident_type, IDENT_FUNC);
    case NODE_PARAM_DECL: // type: 0, symbol: 1
        set_if_unset(redec_of, "Parameter");
        set_if_unset(ident_type, IDENT_VAR);
        {
            const auto type = to_ast_node(children[0]);
            const auto data_type = type->kw_type();
            const auto symbol = to_symbol_node(children[1]);
            const auto could_set = symbol->set_types(data_type, ident_type.value());
            if (!could_set)
            {
                const auto line_number = type->get_line_number();
                std::stringstream ss;
                ss << "Redeclaration of ";
                ss << redec_of.value();
                ss << " ";
                ss << symbol->get_text();
                ss << ". Originally declared at ";
                ss << std::to_string(symbol->get_original_line_number());
                analyzer.add_error(line_number, ss.str());
            }
            return node_type != NODE_FUN_DECL ? SKIP_ALL : SKIP_NONE;
        }

    // Check if undeclare:
    case NODE_SYMBOL: // Itself: 0
        {
            /*
                * NODE_ATRIB, NODE_FUN_CALL, NODE_ARG_LIST, NODE_IF, NODE_WHILE, NODE_DO_WHILE, NODE_READ, NODE_PRINT, NODE_RETURN
                ? Handles all the above cases, as at this point everything should be defined
            */
            const auto symbol = to_symbol_node(children[0]);
            if (!symbol->is_valid())
            {
                analyzer.add_error(symbol->get_line_number(), "Undeclared Variable " + symbol->get_text());
            }
            return SKIP_ALL;
        }

    default:
        throw std::runtime_error("Unhandled case in declaration checker. " + NodeTypeString(node_type));
        break;
    }

    return SKIP_NONE;
}

SemanticAnalyzer check_declarations(NodePtr node)
{
    if (node == nullptr)
    {
        return SemanticAnalyzer();
    }

    auto analyzer = SemanticAnalyzer();
    const ActiveNodes active_nodes{
        // Mark as declared and check redeclaration
        NODE_VAR_DECL,
        NODE_VEC_DEF,
        NODE_FUN_DECL,
        NODE_PARAM_DECL,
        // Check if undeclared
        NODE_SYMBOL, //? It will be skipped over when unneeded by should_continue.
    };

    WalkFunc func = declaration_checker;

    node->walk_tree(analyzer, active_nodes, func, true);

    return analyzer;
}


ptrdiff_t uses_checker(SemanticAnalyzer& analyzer, const NodeType node_type, const NodeList& children)
{
    switch (node_type)
    {
    case NODE_VEC: // vec: 0, index: 1
        {
            /*
            AST NodeType: NODE_VEC
                AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, v, 12, TYPE_INT, IDENT_VECTOR]
                AST SymbolTableEntry: Symbol[SYMBOL_INT, 7, 35, TYPE_INT, IDENT_LIT]
            */
            const auto vec = to_symbol_node(children[0]);
            const auto index = to_symbol_node(children[1]);
            // check if vec is a vector
            if (vec->get_ident_type() != IDENT_VECTOR)
            {
                analyzer.add_error(vec->get_line_number(), "Variable " + vec->get_text() + " is not a vector");
            }
            // check if index is an int or byte variable, or a int or byte literal
            const auto index_type = index->check_expr_type();
            if (index_type != TYPE_INT && index_type != TYPE_CHAR)
            {
                analyzer.add_error(index->get_line_number(), "Index " + index->get_text() + " is not an int or byte");
            }
            return SKIP_ALL;
        }
    case NODE_FUN_CALL: // fun: 0, args: 1
        {
            /*
            AST NodeType: NODE_FUN_CALL
                AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, incn, 27, TYPE_INT, IDENT_FUNC]
                AST NodeType: NODE_ARG_LIST
                    AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, x, 7, TYPE_INT, IDENT_VAR]
                    AST SymbolTableEntry: Symbol[SYMBOL_INT, 1, 7, TYPE_INT, IDENT_LIT]
            */
            const auto fun = to_symbol_node(children[0]);
            const auto args = to_ast_node(children[1]);
            // check if fun is a function
            if (fun->get_ident_type() != IDENT_FUNC)
            {
                analyzer.add_error(fun->get_line_number(), "Variable " + fun->get_text() + " is not a function");
            }
            return 1; // Check arguments in the function, skipping the function name itself
        }
    case NODE_ATRIB: // var: 0, expr: 1
        {
            // check if var is a variable, not a literal or a function
                return 1; // Check expression type, skipping the variable name itself
        }
    case NODE_SYMBOL:
        // need to check if its not a literal, then it is being used as a variable
        // Functions and vectors cannot be used as variables. Literals can be ignored.
        {
            const auto symbol = to_symbol_node(children[0]);
            if (symbol->get_ident_type() == IDENT_LIT)
            {
                return SKIP_ALL;
            }
            else if (symbol->get_ident_type() == IDENT_FUNC)
            {
                analyzer.add_error(symbol->get_line_number(), "Function " + symbol->get_text() + " cannot be used as a variable");
            }
            else if (symbol->get_ident_type() == IDENT_VECTOR)
            {
                analyzer.add_error(symbol->get_line_number(), "Vector " + symbol->get_text() + " cannot be used as a variable");
            }
            return SKIP_NONE;
        }
    default:
        throw std::runtime_error("Unhandled case in uses checker. " + NodeTypeString(node_type));
        break;
    }

    return SKIP_NONE;
}

SemanticAnalyzer check_uses(NodePtr node)
{
    if (node == nullptr)
    {
        return SemanticAnalyzer();
    }

    auto analyzer = SemanticAnalyzer();
    const ActiveNodes active_nodes{
        NODE_ATRIB,
        NODE_FUN_CALL,
        NODE_VEC,
        NODE_SYMBOL,
    };

    WalkFunc func = uses_checker;

    node->walk_tree(analyzer, active_nodes, func, true);

    return analyzer;
}

