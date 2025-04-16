#pragma once
// ast.hpp file made by Ian Kersz Amaral - 2025/1

#include <cstdint>
#include <memory>
#include <vector>
#include <variant>

#include "scanner.hpp"

enum NodeType
{
    NODE_UNKNOWN,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,
    NODE_LT,
    NODE_GT,
    NODE_LE,
    NODE_GE,
    NODE_EQ,
    NODE_DIF,
    NODE_AND,
    NODE_OR,
    NODE_NOT,
    NODE_PARENTHESIS,
    NODE_IF,
    NODE_ARG_LIST,
    NODE_FUNC_CALL,
};

std::string NodeTypeString(const NodeType type);

typedef std::variant<NodeType, SymbolTableEntry> NodeValue;

typedef struct Node
{   
public:
    typedef std::shared_ptr<struct Node> NodePtr;
    typedef std::vector<NodePtr> NodeList;
    
private:
    NodeValue value;
    NodeList children;

public:
    Node(SymbolTableEntry symbol, NodeList children = {})
        : value(symbol), children(children) {}

    Node(NodeType type, NodeList children = {})
        : value(type), children(children) {}

    void add_child(NodePtr child);

    std::string to_string() const;
    std::string tree_string(size_t level = 0);
} Node;

typedef Node::NodePtr NodePtr;
typedef Node::NodeList NodeList;

NodePtr make_node(NodeType type, NodeList children = {});

NodePtr make_node(SymbolTableEntry symbol, NodeList children = {});

void print_tree(NodePtr);
