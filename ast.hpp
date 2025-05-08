#pragma once
// ast.hpp file made by Ian Kersz Amaral - 2025/1

#include <cstdint>
#include <memory>
#include <vector>
#include <variant>

#include "symbol.hpp"

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
    NODE_SYMBOL,
};
std::string NodeTypeString(const NodeType type);

typedef struct Node
{   
public:
    typedef std::shared_ptr<struct Node> NodePtr;
    typedef std::vector<NodePtr> NodeList;
    
protected:
    NodeList children;

private:
    LineNumber line_number;

public:
    Node(LineNumber line_number, NodeList children = {})
        : children(children), line_number(line_number) {}

    void add_child(NodePtr child);

    std::string tree_string(size_t level = 0) const;
    LineNumber get_line_number() const;

    virtual std::string to_string() const = 0;
    virtual std::string export_tree(size_t level = 0) const = 0;
    virtual DataType check_expr_type() const = 0;
} Node;

typedef Node::NodePtr NodePtr;
typedef Node::NodeList NodeList;
NodePtr make_node();
std::string print_tree(NodePtr);



typedef struct ASTNode final : public Node
{
private:
    NodeType node_type;

public:
    ASTNode(NodeType type, LineNumber line_number, NodeList children = {})
        : Node(line_number, children), node_type(type) {}

    std::string to_string() const override;
    std::string export_tree(size_t level = 0) const override;
    DataType check_expr_type() const override;
} ASTNode;
NodePtr make_node(NodeType type, NodeList children = {});
std::shared_ptr<ASTNode> to_ast_node(NodePtr);


typedef struct SymbolNode final : public Node
{
private:
    SymbolTableEntry symbol;

public:
    SymbolNode(SymbolTableEntry symbol, LineNumber line_number, NodeList children = {})
        : Node(line_number, children), symbol(symbol) {}

    std::string to_string() const override;
    std::string export_tree(size_t level = 0) const override;
    DataType check_expr_type() const override;
    bool set_data_type(DataType type) const;

    std::string get_text() const
    {
        return symbol->get_text();
    }

    bool is_valid() const
    {
        return symbol->is_valid();
    }

} SymbolNode;
NodePtr make_node(SymbolTableEntry symbol, NodeList children = {});
std::shared_ptr<SymbolNode> to_symbol_node(NodePtr);
