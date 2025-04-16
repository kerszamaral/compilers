#include "ast.hpp"

// ast.hpp file made by Ian Kersz Amaral - 2025/1

#include <iostream>
#include <sstream>

std::string NodeTypeString(const NodeType type)
{
    switch (type)
    {
        case NODE_UNKNOWN:
            return "NODE_UNKNOWN";
        case NODE_ADD:
            return "NODE_ADD";
        case NODE_SUB:
            return "NODE_SUB";
        case NODE_MUL:
            return "NODE_MUL";
        case NODE_DIV:
            return "NODE_DIV";
        case NODE_MOD:
            return "NODE_MOD";
        case NODE_LT:
            return "NODE_LT";
        case NODE_GT:
            return "NODE_GT";
        case NODE_LE:
            return "NODE_LE";
        case NODE_GE:
            return "NODE_GE";
        case NODE_EQ:
            return "NODE_EQ";
        case NODE_DIF:
            return "NODE_DIF";
        case NODE_AND:
            return "NODE_AND";
        case NODE_OR:
            return "NODE_OR";
        case NODE_NOT:
            return "NODE_NOT";
        case NODE_PARENTHESIS:
            return "NODE_PARENTHESIS";
        case NODE_IF:
            return "NODE_IF";
        case NODE_ARG_LIST:
            return "NODE_ARG_LIST";
        case NODE_FUNC_CALL:
            return "NODE_FUNC_CALL";
        default:
            return "NODE_UNKNOWN";
    }
}

void Node::add_child(NodePtr child)
{
    if (child == nullptr)
    {
        return;
    }
    children.push_back(child);
}

std::string Node::to_string() const
{
    return std::visit([](auto &&value)
    {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, NodeType>)
        {
            return "AST NodeType: " + NodeTypeString(value);
        }
        else if constexpr (std::is_same_v<T, SymbolTableEntry>)
        {
            return "AST SymbolTableEntry: " + value->to_string();
        }
    }, this->value);
}

std::string Node::tree_string(size_t level)
{
    std::stringstream ss;
    ss << std::string(level*2, ' ') << this->to_string() << "\n";
    
    for (const auto &child : children)
    {
        ss << child->tree_string(level + 1);
    }
    
    return ss.str();
}

NodeList remove_null_nodes(const NodeList &children)
{
    NodeList result;
    for (const auto &child : children)
    {
        if (child != nullptr)
        {
            result.push_back(child);
        }
    }
    return result;
}

NodePtr make_node(NodeType type, NodeList children)
{

    return std::make_shared<Node>(type, remove_null_nodes(children));
}

NodePtr make_node(SymbolTableEntry symbol, NodeList children)
{
    return std::make_shared<Node>(symbol, remove_null_nodes(children));
}

void print_tree(NodePtr node)
{
    if (node == nullptr)
    {
        std::cerr << "Node is null" << std::endl;
        return;
    }

    std::cerr << node->tree_string() << std::endl;
}