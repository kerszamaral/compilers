#include "checkers.hpp"

#include <sstream>
#include <iostream>

typedef std::function<SemanticAnalyzer(NodePtr)> Checker;

std::pair<size_t, std::string> run_semantic_analysis(NodePtr node)
{
    const std::vector<Checker> checkers{
        check_declarations,
        // check_types,
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

constexpr auto SHOULD_CONTINUE = true;

std::string redec_of(const NodeType node_type)
{
    std::stringstream ss;
    ss << "Redeclaration of ";
    switch (node_type)
    {
        case NODE_VAR_DECL:
            ss << "Variable";
            break;
        case NODE_VEC_DEF:
            ss << "Vector";
            break;
        case NODE_PARAM_DECL:
            ss << "Parameter";
            break;
        case NODE_FUN_DECL:
            ss << "Function";
            break;
        default:
            ss << "UNKNOWN";
            break;
    }
    ss << " ";
    return ss.str();
}

bool declaration_checker(SemanticAnalyzer& analyzer, const NodeType node_type, const NodeList& children)
{
    switch (node_type)
    {
    // Mark as declared and check redeclaration
    case NODE_VAR_DECL: // type: 0, symbol: 1, init_val: 2
    case NODE_VEC_DEF: // type: 0, symbol: 1, size: 2
    case NODE_PARAM_DECL: // type: 0, symbol: 1
    case NODE_FUN_DECL: // ret_type: 0, symbol: 1
        {
            const auto type = to_ast_node(children[0]);
            const auto data_type = type->kw_type();
            const auto symbol = to_symbol_node(children[1]);
            const auto could_set = symbol->set_data_type(data_type);
            if (!could_set)
            {
                const auto line_number = type->get_line_number();
                analyzer.add_error(line_number, redec_of(node_type) + symbol->get_text() + ". Originally declared at " + std::to_string(symbol->get_line_number()));
            }
            return node_type != NODE_FUN_DECL ? !SHOULD_CONTINUE : SHOULD_CONTINUE;
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
            return !SHOULD_CONTINUE;
        }

    default:
        throw std::runtime_error("Unhandled case in declaration checker. " + NodeTypeString(node_type));
        break;
    }

    return SHOULD_CONTINUE;
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
