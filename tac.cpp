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
        case TAC_ATRIB: return "ATRIB";
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

    std::vector<TACptr> child_tacs;
    for (auto &child : node->get_children())
    {
        auto tac = generate_code(child);
        if (tac)
        {
            child_tacs.push_back(tac);
        }
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
            TACptr new_tac = std::make_shared<TAC>(
                tac_type.value(),
                nullptr,
                child_tacs[0],
                child_tacs[1]
            );
            return TAC::join(child_tacs[0], child_tacs[1], new_tac);
        }
    case NodeType::NODE_NOT:
        {
            TACptr new_tac = std::make_shared<TAC>(
                TAC_NOT,
                nullptr,
                child_tacs[0],
                nullptr
            );
            return TAC::join(child_tacs[0], new_tac);
        }
    case NodeType::NODE_ATRIB:
        {
            TACptr new_tac = std::make_shared<TAC>(
                TAC_ATRIB,
                child_tacs[0],
                child_tacs[1],
                nullptr
            );

            return TAC::join(child_tacs[0], child_tacs[1], new_tac);
        }
    // case NodeType::NODE_PARENTHESIS: // Default
    default:
        return TAC::join(child_tacs);
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
