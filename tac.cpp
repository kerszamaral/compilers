#include "tac.hpp"

#include "set_once.hpp"

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
            const auto move_to = generate_code(node->get_children()[0]);
            const auto moved_from = generate_code(node->get_children()[1]);
            TACptr move_tac = std::make_shared<TAC>(
                TAC_MOVE,
                move_to,
                moved_from,
                nullptr
            );

            return TAC::join(move_to, moved_from, move_tac);
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
