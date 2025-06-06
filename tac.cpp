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

TACptr TAC::join(const std::vector<TACptr> &tac_list)
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
        return std::make_shared<TAC>(TacType::TAC_SYMBOL, to_symbol_node(node)->get_symbol());
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
            TACptr math_tac = std::make_shared<TAC>(
                tac_type.value(),
                nullptr,
                first_op,
                second_op
            );
            return TAC::join(first_op, second_op, math_tac);
        }
    case NodeType::NODE_NOT:
        {
            const auto first_op = generate_code(node->get_children()[0]);
            TACptr not_tac = std::make_shared<TAC>(
                TAC_NOT,
                nullptr,
                first_op,
                nullptr
            );
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

            TACptr move_tac = std::make_shared<TAC>(
                tac_should_be,
                move_to,
                moved_from,
                offset
            );

            return TAC::join(move_to, moved_from, offset, move_tac);
        }
    case NodeType::NODE_FUN_DECL:
        {
            const auto func_name = to_symbol_node(node->get_children()[1])->get_symbol();
            
            const auto begin_func_tac = std::make_shared<TAC>(
                TAC_BEGINFUN,
                func_name
            );

            const auto func_body = generate_code(node->get_children()[3]);

            const auto end_func_tac = std::make_shared<TAC>(
                TAC_ENDFUN,
                func_name
            );
            
            return TAC::join(
                begin_func_tac,
                func_body,
                end_func_tac
            );
        }
    case NodeType::NODE_FUN_CALL:
        {
            std::vector<TACptr> call_seq_tacs;
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

                const auto tac_arg = std::make_shared<TAC>(
                    TAC_ARG,
                    param_tac,
                    arg_tac,
                    nullptr
                );
                call_seq_tacs.push_back(tac_arg);
            }

            const auto call_tac = std::make_shared<TAC>(
                TAC_CALL,
                nullptr,
                func_name_tac,
                nullptr
            );
            call_seq_tacs.push_back(call_tac);

            return TAC::join(call_seq_tacs);
        }
    case NodeType::NODE_IF:
        {
            auto cond_expr_code = generate_code(node->get_children()[0]); // Code that evaluates condition
            SymbolTableEntry cond_var_symbol = cond_expr_code->get_result(); // Symbol holding condition's boolean result

            auto if_block_code = generate_code(node->get_children()[1]); // Code for the if block
            TACptr else_block_code = (node->get_children().size() > 2) ? generate_code(node->get_children()[2]) : nullptr;

            SymbolTableEntry else_label_sym = register_label();
            SymbolTableEntry endif_label_sym = (else_block_code) ? register_label() : else_label_sym; // If no else, IFZ jumps to endif

            // TAC_IFZ Lbl, Var -> type=TAC_IFZ, result_tac (for Lbl), first_op_tac (for Var)
            auto else_label_tac_node = std::make_shared<TAC>(TacType::TAC_SYMBOL, else_label_sym); // Wrapper for label symbol
            auto cond_var_tac_node = std::make_shared<TAC>(TacType::TAC_SYMBOL, cond_var_symbol); // Wrapper for cond var symbol

            auto tac_ifz = std::make_shared<TAC>(TacType::TAC_IFZ, else_label_tac_node, cond_var_tac_node, nullptr);

            auto tac_else_label_instr = std::make_shared<TAC>(TacType::TAC_LABEL, else_label_sym);
            auto tac_endif_label_instr = std::make_shared<TAC>(TacType::TAC_LABEL, endif_label_sym);
            
            std::vector<TACptr> if_sequence = {cond_expr_code, tac_ifz, if_block_code};

            if (else_block_code) {
                auto tac_jump_to_endif = std::make_shared<TAC>(TacType::TAC_JUMP, endif_label_sym);
                if_sequence.push_back(tac_jump_to_endif);
                if_sequence.push_back(tac_else_label_instr);
                if_sequence.push_back(else_block_code);
            }
            if_sequence.push_back(tac_endif_label_instr); // endif_label is distinct if there was an else block

            return TAC::join(if_sequence);
        }
    case NodeType::NODE_WHILE:
        {
            SymbolTableEntry cond_label_sym = register_label();
            SymbolTableEntry end_while_label_sym = register_label();

            auto tac_cond_label_instr = std::make_shared<TAC>(TacType::TAC_LABEL, cond_label_sym);
            
            auto cond_expr_code = generate_code(node->get_children()[0]); // Code that evaluates condition
            SymbolTableEntry cond_var_symbol = cond_expr_code->get_result();
            auto cond_var_tac_node = std::make_shared<TAC>(TacType::TAC_SYMBOL, cond_var_symbol);


            auto end_while_label_tac_node = std::make_shared<TAC>(TacType::TAC_SYMBOL, end_while_label_sym);
            auto tac_ifz = std::make_shared<TAC>(TacType::TAC_IFZ, end_while_label_tac_node, cond_var_tac_node, nullptr);
            
            auto while_block_code = generate_code(node->get_children()[1]); // Code for the while block
            auto tac_jump_to_cond = std::make_shared<TAC>(TacType::TAC_JUMP, cond_label_sym);
            auto tac_end_while_label_instr = std::make_shared<TAC>(TacType::TAC_LABEL, end_while_label_sym);

            return TAC::join(tac_cond_label_instr, cond_expr_code, tac_ifz, while_block_code, tac_jump_to_cond, tac_end_while_label_instr);
        }
    case NodeType::NODE_DO_WHILE:
        {
            SymbolTableEntry loop_start_label_sym = register_label();
            SymbolTableEntry after_loop_label_sym = register_label();

            auto tac_loop_start_label_instr = std::make_shared<TAC>(TacType::TAC_LABEL, loop_start_label_sym);
            auto do_block_code = generate_code(node->get_children()[0]); // Code for the do block
            auto cond_expr_code = generate_code(node->get_children()[1]); // Code that evaluates condition
            SymbolTableEntry cond_var_symbol = cond_expr_code->get_result();
            auto cond_var_tac_node = std::make_shared<TAC>(TacType::TAC_SYMBOL, cond_var_symbol);

            // IFZ cond_var, L_AFTER_LOOP  (if condition is false, jump out)
            // JUMP L_START
            // L_AFTER_LOOP:
            auto after_loop_label_tac_node = std::make_shared<TAC>(TacType::TAC_SYMBOL, after_loop_label_sym);
            auto tac_ifz_cond = std::make_shared<TAC>(TacType::TAC_IFZ, after_loop_label_tac_node, cond_var_tac_node, nullptr);
            auto tac_jump_to_start = std::make_shared<TAC>(TacType::TAC_JUMP, loop_start_label_sym);
            auto tac_after_loop_label_instr = std::make_shared<TAC>(TacType::TAC_LABEL, after_loop_label_sym);
        
            return TAC::join({tac_loop_start_label_instr, do_block_code, cond_expr_code, tac_ifz_cond, tac_jump_to_start, tac_after_loop_label_instr});
        }
    case NodeType::NODE_PRINT:
        {
            std::vector<TACptr> print_tacs;
            for (const auto &child : node->get_children())
            {
                const auto child_tac = generate_code(child);
                print_tacs.push_back(child_tac);
                const auto print_tac = std::make_shared<TAC>(
                    TAC_PRINT,
                    child_tac->get_result()
                );
                print_tacs.push_back(print_tac);
            }

            // Join all print TACs into a single TAC
            return TAC::join(print_tacs);
        }
    case NodeType::NODE_READ:
        {
            const auto dest_var_symbol = generate_code(node->get_children()[0]);
            const auto read_tac = std::make_shared<TAC>(
                TAC_READ,
                dest_var_symbol->get_result()
            );

            return TAC::join(dest_var_symbol, read_tac);
        }
    case NodeType::NODE_RETURN:
        {
            const auto return_value = generate_code(node->get_children()[0]);
            TACptr return_tac = std::make_shared<TAC>(
                TAC_RET,
                return_value->get_result()
            );
            return TAC::join(return_value, return_tac);
        }
    case NodeType::NODE_VEC:
        {
            const auto symbol_tac = generate_code(node->get_children()[0]);
            const auto offset_tac = generate_code(node->get_children()[1]);

            TACptr vec_tac = std::make_shared<TAC>(
                TAC_VECLOAD,
                nullptr,
                symbol_tac,
                offset_tac
            );

            return TAC::join(symbol_tac, offset_tac, vec_tac);
        }

    // case NodeType::NODE_PARENTHESIS: // Default
    default:
        {
            // Generate code for all children
            std::vector<TACptr> child_tacs;
            for (const auto &child : node->get_children())
            {
                TACptr child_tac = generate_code(child);
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

TACptr TAC::build_forward_links(TACptr tac) {
    if (!tac) {
        return nullptr;
    }

    std::vector<TACptr> tacs_in_execution_order;
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
    return tacs_in_execution_order.front(); // Return the head of the list
}
