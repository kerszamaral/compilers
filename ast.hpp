#pragma once
// ast.hpp file made by Ian Kersz Amaral - 2025/1

#include <cstdint>
#include <memory>
#include <vector>
#include <variant>

#include "scanner.hpp"

enum NodeType
{
    NODE_UNKNOWN
};

std::string NodeTypeString(const NodeType type);

typedef struct Node
{
private:
    std::variant<NodeType, SymbolTableEntry> value;
    std::vector<std::shared_ptr<Node>> children;

public:
    Node(SymbolTableEntry symbol, std::vector<std::shared_ptr<Node>> children = {})
        : value(symbol), children(children) {}

    Node(NodeType type, std::vector<std::shared_ptr<Node>> children = {})
        : value(type), children(children) {}

    void add_child(std::shared_ptr<Node> child);

    std::string to_string() const;
    std::string tree_string(size_t level = 0);
} Node;

typedef std::shared_ptr<Node> NodePtr;

NodePtr make_node(NodeType type, std::vector<NodePtr> children = {});

NodePtr make_node(SymbolTableEntry symbol, std::vector<NodePtr> children = {});
