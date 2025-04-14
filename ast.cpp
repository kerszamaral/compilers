#include "ast.hpp"


void Node::add_child(std::shared_ptr<Node> child)
{
    if (this->child == nullptr)
    {
        this->child = child;
    }
    else
    {
        this->find_last_brother()->brother = child;
    }
}


std::shared_ptr<Node> Node::find_last_brother()
{
    std::shared_ptr<Node> last_brother = this->child;
    while (last_brother->brother != nullptr)
    {
        last_brother = last_brother->brother;
    }
    return last_brother;
}

std::string Node::tree_string(size_t depth)
{
    std::string result = std::string(depth, ' ') + this->to_string() + "\n";
    if (this->child != nullptr)
    {
        result += this->child->tree_string(depth + 2);
    }
    if (this->brother != nullptr)
    {
        result += this->brother->tree_string(depth);
    }
    return result;
}

std::string Node::to_string()
{
    if (this->symbol != nullptr)
    {
        return std::string(symbolName(std::get<0>(*this->symbol))) + " " + std::get<1>(*this->symbol) + " " + std::to_string(std::get<2>(*this->symbol));
    }
    else
    {
        return "Node";
    }
}