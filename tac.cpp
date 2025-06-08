#include "tac.hpp"

#include "set_once.hpp"
#include "symbol.hpp"
#include <memory>

// tac.cpp file made by Ian Kersz Amaral - 2025/1


std::string tac_type_to_string(TacType type)
{
    switch (type)
    {
        case TAC_INVALID: return "TAC_INVALID";
        case TAC_SYMBOL: return "TAC_SYMBOL";
        case TAC_ADD: return "ADD";
        case TAC_SUB: return "SUB";
        case TAC_MUL: return "MUL";
        case TAC_DIV: return "DIV";
        case TAC_MOD: return "MOD";
        case TAC_LT: return "LT";
        case TAC_GT: return "GT";
        case TAC_LE: return "LE";
        case TAC_GE: return "GE";
        case TAC_EQ: return "EQ";
        case TAC_DIF: return "DIF";
        case TAC_AND: return "AND";
        case TAC_OR: return "OR";
        case TAC_NOT: return "NOT";
        case TAC_MOVE: return "MOVE";
        case TAC_LABEL: return "LABEL";
        case TAC_BEGINFUN: return "BEGINFUN";
        case TAC_ENDFUN: return "ENDFUN";
        case TAC_IFZ: return "IFZ";
        case TAC_JUMP: return "JUMP";
        case TAC_CALL: return "CALL";
        case TAC_ARG: return "ARG";
        case TAC_RET: return "RET";
        case TAC_PRINT: return "PRINT";
        case TAC_READ: return "READ";
        case TAC_VECLOAD: return "VECLOAD"; 
        case TAC_VECSTORE: return "VECSTORE";
        case TAC_BEGINVARS: return "BEGINVARS";
        case TAC_BEGINCODE: return "BEGINCODE";
        case TAC_VARBEGIN: return "VARBEGIN";
        case TAC_VARINIT: return "VARINIT";
        case TAC_VAREND: return "VAREND";
        case TAC_VECBEGIN: return "VECBEGIN";
        case TAC_VECINIT: return "VECINIT";
        case TAC_VECZEROS: return "VECZEROS";
        case TAC_VECEND: return "VECEND";
        default: return "UNKNOWN_TAC_TYPE";
    }
}

TACptr TAC::join(const TACptr &first, const TACptr &second)
{
    if (!first)
    {
        return second;
    }
    if (!second)
    {
        return first;
    }

    TACptr current = second;
    while (current->prev)
    {
        current = current->prev; // Traverse to the first TAC in the second list
    }
    
    current->prev = first; // Link the first TAC to the second

    return second;
}

TACptr TAC::join(const TACList &tac_list)
{
    if (tac_list.empty())
    {
        return nullptr;
    }

    TACptr result = tac_list[0];
    for (size_t i = 1; i < tac_list.size(); ++i)
    {
        result = join(result, tac_list[i]);
    }
    return result;
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

TACptr TAC::generate_code(NodePtr node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    SetOnce<TacType> tac_type;

    switch (node->get_node_type())
    {
    case NodeType::NODE_SYMBOL:
        return make_tac_symbol(to_symbol_node(node)->get_symbol());
    case NodeType::NODE_ADD:
        tac_type = TacType::TAC_ADD;
    case NodeType::NODE_SUB:
        tac_type = TacType::TAC_SUB;
    case NodeType::NODE_MUL:
        tac_type = TacType::TAC_MUL;
    case NodeType::NODE_DIV:
        tac_type = TacType::TAC_DIV;
    case NodeType::NODE_MOD:
        tac_type = TacType::TAC_MOD;
    case NodeType::NODE_LT:
        tac_type = TacType::TAC_LT;
    case NodeType::NODE_GT:
        tac_type = TacType::TAC_GT;
    case NodeType::NODE_LE:
        tac_type = TacType::TAC_LE;
    case NodeType::NODE_GE:
        tac_type = TacType::TAC_GE;
    case NodeType::NODE_EQ:
        tac_type = TacType::TAC_EQ;
    case NodeType::NODE_DIF:
        tac_type = TacType::TAC_DIF;
    case NodeType::NODE_AND:
        tac_type = TacType::TAC_AND;
    case NodeType::NODE_OR:
        tac_type = TacType::TAC_OR;
        {
            const auto first_op = generate_code(node->get_children()[0]);
            const auto second_op = generate_code(node->get_children()[1]);
            const auto result_data_type = node->check_expr_type();
            const auto math_tac = make_tac_temp(tac_type.value(), result_data_type, first_op, second_op);
            return TAC::join(first_op, second_op, math_tac);
        }
    case NodeType::NODE_NOT:
        {
            const auto first_op = generate_code(node->get_children()[0]);
            const auto result_data_type = node->check_expr_type();
            const auto not_tac = make_tac_temp(TacType::TAC_NOT, result_data_type, first_op);
            return TAC::join(first_op, not_tac);
        }
    case NodeType::NODE_ATRIB:
        {
            const auto maybe_vec = node->get_children()[0];
            const auto is_vec = node->get_children()[0]->get_node_type() == NODE_VEC;
            const auto &move = is_vec ? maybe_vec : node;
            const auto tac_should_be = is_vec ? TAC_VECSTORE : TAC_MOVE;

            const auto move_to = generate_code(move->get_children()[0]);
            const auto offset = is_vec ? generate_code(move->get_children()[1]) : nullptr;

            const auto moved_from = generate_code(node->get_children()[1]);
            const auto move_tac = make_tac(tac_should_be, move_to, moved_from, offset);

            return TAC::join(move_to, moved_from, offset, move_tac);
        }
    case NodeType::NODE_FUN_DECL:
        {
            const auto func_name = to_symbol_node(node->get_children()[1])->get_symbol();
            
            const auto begin_func_tac = make_tac(TAC_BEGINFUN, func_name);

            const auto func_body = generate_code(node->get_children()[3]);

            const auto end_func_tac = make_tac(TAC_ENDFUN, func_name);

            return TAC::join(begin_func_tac, func_body, end_func_tac);
        }
    case NodeType::NODE_FUN_CALL:
        {
            TACList call_seq_tacs;
            const auto func_args = node->get_children()[1]->get_children();
            const auto func_name = to_symbol_node(node->get_children()[0]);
            const auto func_params = func_name->get_symbol()->get_node().value()->get_children();

            const auto func_name_tac = generate_code(func_name);

            // call_seq_tacs.push_back(func_name_tac);

            for (const auto &[param, arg] : zip(func_params, func_args))
            {
                const auto param_tac = generate_code(param);
                call_seq_tacs.push_back(param_tac);

                const auto arg_tac = generate_code(arg);
                call_seq_tacs.push_back(arg_tac);

                const auto tac_arg = make_tac(TAC_ARG, param_tac, arg_tac);
                call_seq_tacs.push_back(tac_arg);
            }

            const auto func_return_type = node->check_expr_type();
            const auto call_tac = make_tac_temp(TAC_CALL, func_return_type, func_name_tac);

            call_seq_tacs.push_back(call_tac);

            return TAC::join(call_seq_tacs);
        }
    case NodeType::NODE_IF:
        {
            const auto condition = generate_code(node->get_children()[0]);

            const auto if_block = generate_code(node->get_children()[1]);
            const auto else_block = node->get_children().size() > 2 ? generate_code(node->get_children()[2]) : nullptr;

            const auto else_label = make_tac_label();
            const auto endif_label = else_block ? make_tac_label() : else_label; // If no else, IFZ jumps to endif

            const auto tac_ifz = make_tac(TAC_IFZ, else_label, condition);

            TACList if_sequence = {condition, tac_ifz, if_block};

            if (else_block) {
                const auto endif_jump = make_tac(TAC_JUMP, endif_label->get_result());
                if_sequence.push_back(endif_jump);
                if_sequence.push_back(else_label);
                if_sequence.push_back(else_block);
            }
            if_sequence.push_back(endif_label);

            return TAC::join(if_sequence);
        }
    case NodeType::NODE_WHILE:
        {
            const auto end_while_label = make_tac_label();

            const auto condition_label = make_tac_label();
            const auto jump_to_condition = make_tac(TAC_JUMP, condition_label->get_result());

            const auto condition = generate_code(node->get_children()[0]);

            const auto tac_ifz = make_tac(TAC_IFZ, end_while_label->get_result(), condition);

            const auto while_block = generate_code(node->get_children()[1]);

            return TAC::join(condition_label, condition, tac_ifz, while_block, jump_to_condition, end_while_label);
        }
    case NodeType::NODE_DO_WHILE:
        {
            const auto loop_start_label = make_tac_label();
            const auto after_loop_label = make_tac_label();

            const auto do_block = generate_code(node->get_children()[0]);
            const auto condition = generate_code(node->get_children()[1]);

            const auto ifz = make_tac(TAC_IFZ, after_loop_label->get_result(), condition);
            
            const auto jump_to_start = make_tac(TAC_JUMP, loop_start_label->get_result());

            return TAC::join(loop_start_label, do_block, condition, ifz, jump_to_start, after_loop_label);
        }
    case NodeType::NODE_PRINT:
        {
            TACList print_tacs;
            for (const auto &child : node->get_children())
            {
                const auto child_tac = generate_code(child);
                print_tacs.push_back(child_tac);
                const auto print_tac = make_tac(TAC_PRINT, child_tac->get_result());
                print_tacs.push_back(print_tac);
            }

            return TAC::join(print_tacs);
        }
    case NodeType::NODE_READ:
        {
            const auto dest_var_symbol = generate_code(node->get_children()[0]);
            const auto read_tac = make_tac(TAC_READ, dest_var_symbol->get_result());
            return TAC::join(dest_var_symbol, read_tac);
        }
    case NodeType::NODE_RETURN:
        {
            const auto return_value = generate_code(node->get_children()[0]);
            const auto return_tac = make_tac(TAC_RET, return_value->get_result());
            return TAC::join(return_value, return_tac);
        }
    case NodeType::NODE_VEC:
        {
            const auto symbol_tac = generate_code(node->get_children()[0]);
            const auto offset_tac = generate_code(node->get_children()[1]);
            const auto vec_data_type = node->check_expr_type();
            const auto vec_tac = make_tac_temp(TAC_VECLOAD, vec_data_type, symbol_tac, offset_tac);
            return TAC::join(symbol_tac, offset_tac, vec_tac);
        }

    // case NodeType::NODE_PARENTHESIS: // Default
    default:
        {
            // Generate code for all children
            TACList child_tacs;
            for (const auto &child : node->get_children())
            {
                const auto child_tac = generate_code(child);
                if (child_tac)
                {
                    child_tacs.push_back(child_tac);
                }
            }

            // If no children, return nullptr
            if (child_tacs.empty())
            {
                return nullptr;
            }

            // Join all child TACs into a single TAC
            return TAC::join(child_tacs);
        }
    }
}

TACptr TAC::generate_vars(NodePtr node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    switch (node->get_node_type())
    {
    case NODE_VAR_DECL:
        {
            const auto var = generate_code(node->get_children()[1]);
            const auto var_begin = make_tac(TAC_VARBEGIN, var->get_result());
            const auto init = generate_code(node->get_children()[2]);
            const auto init_tac = make_tac(TAC_VARINIT, init->get_result());
            const auto var_end = make_tac(TAC_VAREND, var->get_result());
            return TAC::join(var_begin, init_tac, var_end);
        }
    case NODE_VEC_DECL:
        {
            TACList vec_decl;
            const auto vec_def = node->get_children()[0];
            const auto vec_symb = generate_code(vec_def->get_children()[1]);
            const auto vec_size = generate_code(vec_def->get_children()[2]);

            const auto vec_begin = make_tac(TAC_VECBEGIN, vec_symb->get_result());

            vec_decl.push_back(vec_begin);
            
            const auto vec_end = make_tac(TAC_VECEND, vec_symb->get_result(), vec_size);

            if (node->get_children().size() > 1)
            {
                const auto &inits = node->get_children()[1];
                for (const auto &init_val : inits->get_children())
                {
                        const auto init_val_tac = generate_code(init_val);
                        const auto init_tac = make_tac(TAC_VECINIT, init_val_tac->get_result());
                        vec_decl.push_back(init_tac);
                }
            }
            else
            {
                const auto zeros = make_tac(TAC_VECZEROS, vec_size->get_result());
                vec_decl.push_back(zeros);
            }

            vec_decl.push_back(vec_end);
            return TAC::join(vec_decl);
        }
#ifndef PARAM_ISNT_VAR_DECL
    case NODE_PARAM_DECL:
        {
            const auto var = generate_code(node->get_children()[1]);
            const auto var_begin = make_tac(TAC_VARBEGIN, var->get_result());
            const auto var_end = make_tac(TAC_VAREND, var->get_result());
            return TAC::join(var_begin, var_end);
        }
#endif
    default:
        {
            TACList child_tacs;
            for (const auto &child : node->get_children())
            {
                TACptr child_tac = generate_vars(child);
                if (child_tac)
                {
                    child_tacs.push_back(child_tac);
                }
            }

            return TAC::join(child_tacs);
        }

    }
}

TACptr TAC::generate_tacs(NodePtr node)
{
    if (node == nullptr)
    {
        return nullptr;
    }

    // Generate variable declarations first
    const auto begin_vars = std::make_shared<TAC>(
        TAC_BEGINVARS,
        register_label()
    );

    const auto vars = generate_vars(node);

    const auto begin_code = std::make_shared<TAC>(
        TAC_BEGINCODE,
        register_label()
    );
    const auto code = generate_code(node);

    return TAC::join(begin_vars, vars, begin_code, code);
}

std::string TAC::to_string() const
{
    if (type == TAC_SYMBOL)
    {
        return "TAC(" +
               tac_type_to_string(type) + ", " + (result ? result->get_text() : "null") + ")";
    }
    if (type == TAC_INVALID)
    {
        return "TAC(INVALID)";
    }
    return "TAC(" + tac_type_to_string(type) + ", " 
        + (result ? result->get_text() : "null") + ", "
        + (first_operator ? first_operator->get_text() : "null") + ", "
        + (second_operator ? second_operator->get_text() : "null") + ")";
}

std::string TAC::tac_string(TACptr tac)
{
    std::stringstream ss;
    while (tac && tac->next)
    {
#ifndef SHOW_TAC_SYMBOL
        if (tac->get_type() == TAC_SYMBOL)
        {
            tac = tac->next; // Skip symbol TACs
            continue;
        }
#endif
        ss << tac->to_string() << "\n";
        tac = tac->next; // Traverse to the first TAC in the list
    }

    if (tac)
    {
        ss << tac->to_string(); // Add the last TAC
    }

    return ss.str();
}

std::string TAC::tac_string(TACList tac_list)
{
    return tac_string(tac_list.empty() ? nullptr : tac_list[0]);
}

std::string TAC::tac_string_backwards(TACptr tac)
{
    std::stringstream ss;
    while (tac && tac->prev)
    {
#ifndef SHOW_TAC_SYMBOL
        if (tac->get_type() == TAC_SYMBOL)
        {
            tac = tac->prev; // Skip symbol TACs
            continue;
        }
#endif
        ss << tac->to_string() << "\n";
        tac = tac->prev; // Traverse to the first TAC in the list
    }

    return ss.str();
}

TACList TAC::build_forward_links(TACptr tac) {
    if (!tac) {
        return {};
    }

    TACList tacs_in_execution_order;
    TACptr current = tac;

    while (current) {
        tacs_in_execution_order.push_back(current);
        if (!current->prev) { // Found the head of the list
            break;
        }
        current = current->prev;
    }

    std::reverse(tacs_in_execution_order.begin(), tacs_in_execution_order.end());

    for (size_t i = 0; i < tacs_in_execution_order.size(); ++i) {
        if (i + 1 < tacs_in_execution_order.size()) {
            tacs_in_execution_order[i]->next = tacs_in_execution_order[i+1];
        } else {
            tacs_in_execution_order[i]->next = nullptr; // Last TACs next is nullptr
        }
    }
    return tacs_in_execution_order;
}


TACptr make_tac_symbol(const SymbolTableEntry result)
{    
    if (!result)
    {
        throw std::runtime_error("Cannot create TAC with null symbol");
    }
    return std::make_shared<TAC>(TAC_SYMBOL, result);
}

TACptr make_tac_temp(const TacType type, const DataType data_type, const TACptr first, const TACptr second)
{
    return std::make_shared<TAC>(type, nullptr, first, second, data_type);
}

TACptr make_tac(const TacType type, const TACptr result, const TACptr first, const TACptr second)
{
    return std::make_shared<TAC>(type, result, first, second);
}

TACptr make_tac(const TacType type, const SymbolTableEntry result, const TACptr first, const TACptr second)
{
    const auto result_tac = make_tac_symbol(result);
    const auto ret_tac = std::make_shared<TAC>(type, result_tac, first, second);
    return TAC::join(result_tac, ret_tac);
}

TACptr make_tac(const TacType type, const SymbolTableEntry symbol)
{
    return std::make_shared<TAC>(type, symbol);
}

TACptr make_tac_label()
{
    return std::make_shared<TAC>(TAC_LABEL, register_label());
}
