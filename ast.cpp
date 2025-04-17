#include "ast.hpp"

// ast.hpp file made by Ian Kersz Amaral - 2025/1

#include <iostream>
#include <sstream>

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string NodeTypeString(const NodeType type)
{
    switch (type)
    {
        case NODE_UNKNOWN:
            return "NODE_UNKNOWN";
        case NODE_PROGRAM:
            return "NODE_PROGRAM";
        case NODE_VAR_DECL:
            return "NODE_VAR_DECL";
        case NODE_VEC_DECL:
            return "NODE_VEC_DECL";
        case NODE_VEC_DEF:
            return "NODE_VEC_DEF";
        case NODE_VEC_INIT:
            return "NODE_VEC_INIT";
        case NODE_FUN_DECL:
            return "NODE_FUN_DECL";
        case NODE_PARAM_LIST:
            return "NODE_PARAM_LIST";
        case NODE_PARAM_DECL:
            return "NODE_PARAM_DECL";
        case NODE_CMD_BLOCK:
            return "NODE_CMD_BLOCK";
        case NODE_CMD_LIST:
            return "NODE_CMD_LIST";
        case NODE_ATRIB:
            return "NODE_ATRIB";
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
        case NODE_FUN_CALL:
            return "NODE_FUN_CALL";
        case NODE_ARG_LIST:
            return "NODE_ARG_LIST";
        case NODE_IF:
            return "NODE_IF";
        case NODE_WHILE:
            return "NODE_WHILE";
        case NODE_DO_WHILE:
            return "NODE_DO_WHILE";
        case NODE_READ:
            return "NODE_READ";
        case NODE_PRINT:
            return "NODE_PRINT";
        case NODE_RETURN:
            return "NODE_RETURN";
        case NODE_VEC:
            return "NODE_VEC";
        case NODE_KW_INT:
            return "NODE_KW_INT";
        case NODE_KW_REAL:
            return "NODE_KW_REAL";
        case NODE_KW_BYTE:
            return "NODE_KW_BYTE";
    }
}
#pragma clang diagnostic pop

void Node::add_child(NodePtr child)
{
    if (child == nullptr)
    {
        return;
    }
    // children.push_back(child);
    // Because the parser is bottom-up, we need to insert the child at the beginning of the list
    children.insert(children.begin(), child);
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

std::string Node::tree_string(size_t level) const
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

NodePtr make_node()
{
    return std::make_shared<Node>(NODE_UNKNOWN);
}

std::string print_tree(NodePtr node)
{
    if (node == nullptr)
    {
        return "Node is null\n";
    }

    return node->tree_string() + "\n";
}
