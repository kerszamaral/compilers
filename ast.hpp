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
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_VEC_DECL,
    NODE_VEC_DEF,
    NODE_VEC_INIT,
    NODE_FUN_DECL,
    NODE_PARAM_LIST,
    NODE_PARAM_DECL,
    NODE_CMD_BLOCK,
    NODE_CMD_LIST,
    NODE_ATRIB,
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
    NODE_FUN_CALL,
    NODE_ARG_LIST,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_READ,
    NODE_PRINT,
    NODE_RETURN,

    NODE_VEC,
    NODE_KW_INT,
    NODE_KW_REAL,
    NODE_KW_BYTE,
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
    std::string tree_string(size_t level = 0) const;
} Node;

typedef Node::NodePtr NodePtr;
typedef Node::NodeList NodeList;

NodePtr make_node(NodeType type, NodeList children = {});

NodePtr make_node(SymbolTableEntry symbol, NodeList children = {});

NodePtr make_node();

std::string print_tree(NodePtr);
