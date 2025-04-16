#include "ast.hpp"

// ast.hpp file made by Ian Kersz Amaral - 2025/1

#include <sstream>

std::string NodeTypeString(const NodeType type)
{
    switch (type)
    {
        default:
            return "UNKNOWN_NODE_TYPE";
    }
}

void Node::add_child(std::shared_ptr<Node> child)
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
            return "NodeType: " + NodeTypeString(value);
        }
        else if constexpr (std::is_same_v<T, SymbolTableEntry>)
        {
            return "SymbolTableEntry: " + SymbolEntryString(value);
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

NodePtr make_node(NodeType type, std::vector<NodePtr> children)
{
    return std::make_shared<Node>(type, children);
}

NodePtr make_node(SymbolTableEntry symbol, std::vector<NodePtr> children)
{
    return std::make_shared<Node>(symbol, children);
}
