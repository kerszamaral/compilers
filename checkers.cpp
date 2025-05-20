#include "checkers.hpp"

// checkers.cpp file made by Ian Kersz Amaral - 2025/1

#include <sstream>
#include <iostream>
#include <optional>
#include <limits>

typedef std::function<SemanticAnalyzer(NodePtr)> Checker;

std::pair<size_t, std::string> run_semantic_analysis(NodePtr node)
{
    const std::vector<Checker> checkers{
        check_declarations,
        check_uses,
        check_types,
        check_arguments,
        check_return,
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
    if (!opt.has_value()) {
        opt = T(std::forward<U>(value));
    }
}
// Helper: sets the optional only if it's not already set, also prevents the index from being evaluated.
template<typename T, typename U>
void set_if_unset(std::optional<T>& opt, std::vector<U> value, size_t&& index) {
    if (!opt.has_value()) {
        opt = T(std::forward<U>(value[index]));
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
    case NODE_FUN_DECL: // ret_type: 0, symbol: 1, param_list: 2, body: 3
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
            if (node_type == NODE_FUN_DECL)
            {
                // param list is the third child
                symbol->set_node(children[2]);
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
            // check if vec is a vector
            if (vec->get_ident_type() != IDENT_VECTOR)
            {
                analyzer.add_error(vec->get_line_number(), "Variable " + vec->get_text() + " is not a vector");
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
            /*
            AST NodeType: NODE_ATRIB
                AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, a, 3, TYPE_CHAR, IDENT_VAR]
                AST NodeType: NODE_ADD
                    AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, a, 3, TYPE_CHAR, IDENT_VAR]
                    AST SymbolTableEntry: Symbol[SYMBOL_INT, 1, 7, TYPE_INT, IDENT_LIT]
            */
            // check if var is a variable, not a literal or a function
            // A crash can happen because the assigned can be either a vector or a variable.
            const auto assignee = children[0];
            if (assignee->get_node_type() != NODE_SYMBOL)
            {
                return SKIP_NONE; // Check both sides (vector and expression).
            }
            else
            {
                // check if var is a variable
                const auto var = to_symbol_node(children[0]);
                const auto type = var->get_ident_type();
                if (type == IDENT_LIT || type == IDENT_FUNC || type == IDENT_VECTOR)
                {
                    const auto type_str = ident_type_to_str(type, true);
                    analyzer.add_error(var->get_line_number(), type_str + var->get_text() + " cannot be assigned to.");
                }
                return 1; // Check expression type, skipping the variable name itself
            }
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
    case NODE_FUN_DECL:
        return 2; // Skip the function return type and the function name, but check the arguments and the body
    case NODE_VEC_DEF:
        return SKIP_ALL; // Declarations need to be skipped.
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

        // Declarations need to be skipped.
        // NODE_VAR_DECL,
        NODE_VEC_DEF,
        NODE_FUN_DECL,
        // NODE_PARAM_DECL,
    };

    WalkFunc func = uses_checker;

    node->walk_tree(analyzer, active_nodes, func, true);

    return analyzer;
}

ptrdiff_t types_checker(SemanticAnalyzer& analyzer, const NodeType node_type, const NodeList& children)
{
    std::optional<NodePtr> assignee_opt;
    std::optional<NodePtr> expr_opt;
    std::optional<std::string> type_name;
    switch (node_type)
    {
    case NODE_VEC: // vec: 0, index: 1
        {
            /*
            AST NodeType: NODE_VEC
                AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, v, 12, TYPE_INT, IDENT_VECTOR]
                AST SymbolTableEntry: Symbol[SYMBOL_INT, 7, 35, TYPE_INT, IDENT_LIT]
            */
            const auto index = children[1];
            // check if index is an int or byte variable, or a int or byte literal
            const auto index_type = index->check_expr_type();
            if (index_type != TYPE_INT && index_type != TYPE_CHAR)
            {
                analyzer.add_error(index->get_line_number(), "Vector index is not an int or byte");
            }
            return SKIP_ALL;
        }
    case NODE_VEC_DECL:
        /*
        AST NodeType: NODE_VEC_DECL 12
            AST NodeType: NODE_VEC_DEF 12
                AST NodeType: NODE_KW_INT 12
                AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, v, 12, TYPE_INT, IDENT_VECTOR]
                AST SymbolTableEntry: Symbol[SYMBOL_INT, 10, 12, TYPE_INT, IDENT_LIT]
            AST NodeType: NODE_VEC_INIT 12
                AST SymbolTableEntry: Symbol[SYMBOL_CHAR, 'a', 12, TYPE_CHAR, IDENT_LIT]
                AST SymbolTableEntry: Symbol[SYMBOL_INT, 0, 12, TYPE_INT, IDENT_LIT]
        */
        if (children.size() < 2)
        {
            return SKIP_ALL;
        }
        set_if_unset(assignee_opt, children, 0);
        set_if_unset(expr_opt, children, 1);
        set_if_unset(type_name, "Vector declaration");
    case NODE_ATRIB: // var: 0, expr: 1
        set_if_unset(assignee_opt, children, 0);
        set_if_unset(expr_opt, children, 1);
        set_if_unset(type_name, "Assignment");
    case NODE_VAR_DECL: // type: 0, symbol: 1, init_val: 2
        set_if_unset(assignee_opt, children, 1);
        set_if_unset(expr_opt, children, 2);
        set_if_unset(type_name, "Variable declaration");
        {
            /*
            AST NodeType: NODE_ATRIB
                AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, a, 3, TYPE_CHAR, IDENT_VAR]
                AST NodeType: NODE_ADD
                    AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, a, 3, TYPE_CHAR, IDENT_VAR]
                    AST SymbolTableEntry: Symbol[SYMBOL_INT, 1, 7, TYPE_INT, IDENT_LIT]
            */
            // check if var is a variable, not a literal or a function
            const auto assignee_type = assignee_opt.value()->check_expr_type();
            const auto expr_type = expr_opt.value()->check_expr_type();
            const auto line_number = assignee_opt.value()->get_line_number();
            if (assignee_type == TYPE_INVALID || expr_type == TYPE_INVALID)
            {
                analyzer.add_error(line_number, "Invalid " + data_type_to_str(assignee_type, true) + " to " + data_type_to_str(expr_type, true));   
                return SKIP_NONE;
            }
            if ( (expr_type == TYPE_REAL && assignee_type != TYPE_REAL) 
              || ((expr_type == TYPE_INT || expr_type == TYPE_CHAR) && assignee_type == TYPE_REAL))
            {
                analyzer.add_error(line_number, type_name.value() + ": Cannot convert " + data_type_to_str(expr_type, true) + " to " + data_type_to_str(assignee_type, true));
            }
            if (expr_type == TYPE_BOOL)
            {
                analyzer.add_error(line_number, type_name.value() + ": Cannot assign boolean to a variable");
            }
            // Need to check if the vector initialization lenghth is the same as the vector size
            if (node_type == NODE_VEC_DECL)
            {
                const auto vec_decl = to_ast_node(assignee_opt.value());
                const auto vec_decl_index = to_symbol_node(vec_decl->get_children()[2]);
                if (vec_decl_index->check_expr_type() != TYPE_INT && vec_decl_index->check_expr_type() != TYPE_CHAR)
                {
                    analyzer.add_error(vec_decl_index->get_line_number(), "Vector size " + vec_decl_index->get_text() + " is not an int or byte");
                    return SKIP_ALL;
                }
                const auto vec_decl_size = static_cast<size_t>(std::stoi(vec_decl_index->get_text()));
                const auto vec_init_size = expr_opt.value()->get_children().size();
                if (vec_decl_size != vec_init_size)
                {
                    analyzer.add_error(line_number, "Vector size " + std::to_string(vec_decl_size) + " does not match the initialization size " + std::to_string(vec_init_size));
                }
            }
            if (assignee_opt.value()->get_node_type() == NODE_VEC)
            {
                types_checker(analyzer, NODE_VEC, assignee_opt.value()->get_children());
            }
            return SKIP_ALL;
        }
    case NODE_IF:
        /*
        AST NodeType: NODE_IF 70
          AST NodeType: NODE_EQ 65
            AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, x, 7, TYPE_INT, IDENT_VAR]
            AST SymbolTableEntry: Symbol[SYMBOL_INT, 1, 7, TYPE_INT, IDENT_LIT]
          AST NodeType: NODE_CMD_BLOCK 68
            ...
        */
        set_if_unset(expr_opt, children, 0);
        set_if_unset(type_name, "If");
    case NODE_WHILE:
        /*
        AST NodeType: NODE_WHILE 48
          AST NodeType: NODE_LT 44
            AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, x, 7, TYPE_INT, IDENT_VAR]
            AST SymbolTableEntry: Symbol[SYMBOL_INT, 20, 44, TYPE_INT, IDENT_LIT]
          AST NodeType: NODE_CMD_BLOCK 48
            ...
        */
        set_if_unset(expr_opt, children, 0);
        set_if_unset(type_name, "While");
    case NODE_DO_WHILE:
        /*
        AST NodeType: NODE_DO_WHILE 55
          AST NodeType: NODE_CMD_BLOCK 55
            ...
          AST NodeType: NODE_PARENTHESIS 55
            AST NodeType: NODE_LT 55
              AST SymbolTableEntry: Symbol[SYMBOL_IDENTIFIER, x, 7, TYPE_INT, IDENT_VAR]
              AST SymbolTableEntry: Symbol[SYMBOL_INT, 10, 12, TYPE_INT, IDENT_LIT]
        */
        set_if_unset(expr_opt, children, 1);
        set_if_unset(type_name, "Do-While");
        {
            const auto expr_type = expr_opt.value()->check_expr_type();
            const auto line_number = expr_opt.value()->get_line_number();
            if (expr_type == TYPE_INVALID)
            {
                analyzer.add_error(line_number, "Invalid" + type_name.value() + "condition");
                return SKIP_NONE;
            }
            if (expr_type != TYPE_BOOL)
            {
                analyzer.add_error(line_number, type_name.value() + " condition is not a boolean");
            }
            return SKIP_NONE;
        }
    default:
        throw std::runtime_error("Unhandled case in types checker. " + NodeTypeString(node_type));
        break;
    }

    return SKIP_NONE;
}

SemanticAnalyzer check_types(NodePtr node)
{
    if (node == nullptr)
    {
        return SemanticAnalyzer();
    }

    auto analyzer = SemanticAnalyzer();
    // Need to check the following:
    const ActiveNodes active_nodes{
        NODE_VAR_DECL, // Declarations are initialized with the correct type (int, real, byte)
        NODE_VEC_DECL,  // Be careful as they can be not initialized.
        NODE_ATRIB, // Assignments are used with the correct type (int and bytes can mix, but real cannot, booleans can never be assigned)
        NODE_VEC, // Vec indexes are used with the correct type (int or byte)
        NODE_IF, // If conditions are used with the correct type (boolean)
        NODE_WHILE, // While conditions are used with the correct type (boolean)
        NODE_DO_WHILE, // Do-while conditions are used with the correct type (boolean)
    };

    WalkFunc func = types_checker;

    node->walk_tree(analyzer, active_nodes, func, true);

    return analyzer;
}

// Again... as we are using C++17, we dont have std::zip from the STL ranges...
// Implemented a simple version of it, but it is not as efficient as the STL one. From:
// https://stackoverflow.com/questions/12552277/whats-the-best-way-to-iterate-over-two-or-more-containers-simultaneously
template<typename... Container>
auto zip(Container&... containers) noexcept {
    using tuple_type = std::tuple<std::decay_t<decltype(*std::begin(containers))>...>;
    std::size_t container_size = std::min({ std::size(containers)... });

    std::vector<tuple_type> result;
    result.reserve(container_size);

    auto iterators = std::make_tuple(std::begin(containers)...);
    for (std::size_t i = 0; i < container_size; ++i) {
        std::apply([&result](auto&... it) { 
            result.emplace_back(*it++...);
        }, iterators);
    }

    return result;
}

ptrdiff_t arguments_checker(SemanticAnalyzer& analyzer, const NodeType node_type, const NodeList& children)
{
    switch (node_type)
    {
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
            const auto fun_decl = fun->get_node();
            if (!fun_decl.has_value() || fun_decl == nullptr)
            {
                // Should never happen, but just in case
                analyzer.add_error(fun->get_line_number(), "Function " + fun->get_text() + " is not declared");
                return SKIP_ALL;
            }
            const auto declared_args = fun_decl.value()->get_children();
            const auto args_list = args->get_children();
            if (declared_args.size() != args_list.size())
            {
                analyzer.add_error(fun->get_line_number(), "Function " + fun->get_text() + " expects " + std::to_string(declared_args.size()) + " arguments, but got " + std::to_string(args_list.size()));
                return SKIP_ALL;
            }
            // Check if the types match
            for (const auto& [declared_arg, arg] : zip(declared_args, args_list))
            {
                const auto declared_arg_type = declared_arg->check_expr_type();
                const auto arg_type = arg->check_expr_type();
                const auto int_conversion = (declared_arg_type == TYPE_INT && arg_type == TYPE_CHAR) || (declared_arg_type == TYPE_CHAR && arg_type == TYPE_INT);
                if (declared_arg_type != arg_type && !int_conversion)
                {
                    analyzer.add_error(arg->get_line_number(), "Function " + fun->get_text() + " expects argument of type " + data_type_to_str(declared_arg_type, true) + ", but got " + data_type_to_str(arg_type, true));
                }
            }
            return SKIP_ALL;
        }
    default:
        throw std::runtime_error("Unhandled case in arguments checker. " + NodeTypeString(node_type));
        break;
    }

    return SKIP_NONE;
}

SemanticAnalyzer check_arguments(NodePtr node)
{
    if (node == nullptr)
    {
        return SemanticAnalyzer();
    }

    auto analyzer = SemanticAnalyzer();
    const ActiveNodes active_nodes{
        NODE_FUN_CALL,
    };

    WalkFunc func = arguments_checker;

    node->walk_tree(analyzer, active_nodes, func, true);

    return analyzer;
}

ptrdiff_t return_checker(SemanticAnalyzer& analyzer, const NodeType node_type, const NodeList& children)
{
    switch (node_type)
    {
    case NODE_FUN_DECL: // ret_type: 0, symbol: 1, param_list: 2, body: 3
        {
            const auto fun_decl = to_symbol_node(children[1]);
            const auto body = children[3];
            const auto rets = body->find_all(NODE_RETURN);
            if (rets.size() > 0)
            {
                for (const auto& ret : rets)
                {
                    const auto ret_type = ret->check_expr_type();
                    if (ret_type != TYPE_INVALID && ret_type != fun_decl->get_data_type())
                    {
                        analyzer.add_error(ret->get_line_number(), "Function " + fun_decl->get_text() + " must return " + data_type_to_str(fun_decl->get_data_type(), true) + ", but got " + data_type_to_str(ret_type, true));
                    }
                }
            }
            return SKIP_ALL;
        }
    default:
        throw std::runtime_error("Unhandled case in return checker. " + NodeTypeString(node_type));
        break;
    }

    return SKIP_NONE;
}

SemanticAnalyzer check_return(NodePtr node)
{
    if (node == nullptr)
    {
        return SemanticAnalyzer();
    }

    auto analyzer = SemanticAnalyzer();
    const ActiveNodes active_nodes{
        NODE_FUN_DECL,
    };

    WalkFunc func = return_checker;

    node->walk_tree(analyzer, active_nodes, func, true);

    return analyzer;
}