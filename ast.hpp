#pragma once

#include <cstdint>
#include <memory>

#include "scanner.hpp"

typedef struct Node
{
    SymbolTableEntry symbol;
    std::shared_ptr<Node> brother;
    std::shared_ptr<Node> child;

    Node(SymbolTableEntry symbol, std::shared_ptr<Node> brother = nullptr, std::shared_ptr<Node> child = nullptr)
        : symbol(symbol), brother(brother), child(child) {}

    Node(Lexeme label, std::shared_ptr<Node> brother = nullptr, std::shared_ptr<Node> child = nullptr)
        : symbol(std::make_shared<Symbol>(SymbolType::SYMBOL_OTHER, label, 0)), brother(brother), child(child) {}

    void add_child(std::shared_ptr<Node> child);

    std::string tree_string(size_t depth);
    std::string to_string();

private:
    std::shared_ptr<Node> find_last_brother();

} Node;
