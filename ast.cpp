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

std::string Node::export_tree(size_t level) const
{
    return std::visit([this, level](auto &&value)
    {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, NodeType>)
        {
            return this->export_node(value, level);
        }
        else if constexpr (std::is_same_v<T, SymbolTableEntry>)
        {
            return this->export_symbol(value);
        }
    }, this->value);
}

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string operator_string(NodeType node)
{
    switch (node)
    {
        case NODE_ADD:
            return "+";
        case NODE_SUB:
            return "-";
        case NODE_MUL:
            return "*";
        case NODE_DIV:
            return "/";
        case NODE_MOD:
            return "%";
        case NODE_LT:
            return "<";
        case NODE_GT:
            return ">";
        case NODE_LE:
            return "<=";
        case NODE_GE:
            return ">=";
        case NODE_EQ:
            return "==";
        case NODE_DIF:
            return "!=";
        case NODE_AND:
            return "&";
        case NODE_OR:
            return "|";
        default:
            return "";
    }
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string Node::export_node(NodeType node, size_t level) const
{
    std::stringstream ss;
    switch (node)
    {
    case NODE_UNKNOWN:
        {
            ss << "UNKOWN_EXPORTED_NODE\n";
            break;
        }
    case NODE_PROGRAM:
        {
            for (const auto &child : children)
            {
                ss << child->export_tree(level);
            }
            break;   
        }
    case NODE_VAR_DECL:
        {
            const auto type = this->children[0];
            const auto symbol = this->children[1];
            const auto init_val = this->children[2];
            ss << type->export_tree(level);
            ss << " " << symbol->export_tree(level);
            ss << " = " << init_val->export_tree(level);
            ss << ";\n";
            break;
        }
    case NODE_VEC_DECL:
        {
            const auto vec_def = this->children[0];
            ss << vec_def->export_tree(level);
            if (this->children.size() > 1)
            {
                const auto vec_init = this->children[1];
                ss << " = ";
                ss << vec_init->export_tree(level);
            }
            ss << ";\n";
            break;
        }
    case NODE_VEC_DEF:
        {
            const auto type = this->children[0];
            const auto symbol = this->children[1];
            const auto size = this->children[2];

            ss << type->export_tree(level);
            ss << " " << symbol->export_tree(level);
            ss << "[" << size->export_tree(level) << "]";
            break;
        }
    case NODE_VEC_INIT:
        {
            for (const auto &child : this->children)
            {
                ss << child->export_tree(level);
                if (child != this->children.back())
                {
                    ss << ", ";
                }
            }
            break;
        }
    case NODE_FUN_DECL:
        {
            const auto ret_type = this->children[0];
            const auto symbol = this->children[1];
            const auto param_list = this->children[2];
            const auto fun_body = this->children[3];
            ss << ret_type->export_tree(level);
            ss << " " << symbol->export_tree(level);
            ss << param_list->export_tree(level);
            ss << " ";
            ss << fun_body->export_tree(level);
            break;
        }
    case NODE_PARAM_LIST:
        {
            ss << "(";
            for (const auto &child : this->children)
            {
                ss << child->export_tree(level);
                if (child != this->children.back())
                {
                    ss << ", ";
                }
            }
            ss << ")";
            break;
        }
    case NODE_PARAM_DECL:
        {
            const auto type = this->children[0];
            const auto symbol = this->children[1];
            ss << type->export_tree(level);
            ss << " " << symbol->export_tree(level);
            break;
        }
    case NODE_CMD_BLOCK:
        {
            ss << std::string(level*2, ' ') << "{\n";
            for (const auto &child : this->children)
            {
                ss << child->export_tree(level+1);
            }
            ss << std::string(level*2, ' ') << "}\n";
            break;
        }
        case NODE_CMD_LIST:
        {           
            for (const auto &child : this->children)
            {
                ss << std::string(level*2, ' ');
                ss << child->export_tree(level);
            }
            break;
        }
    case NODE_ATRIB:
        {
            const auto symbol = this->children[0];
            const auto expr = this->children[1];
            ss << symbol->export_tree(level);
            ss << " = ";
            ss << expr->export_tree(level);
            ss << ";\n";
            break;
        }
    case NODE_VEC:
        {
            const auto symbol = this->children[0];
            const auto expr = this->children[1];
            ss << symbol->export_tree(level);
            ss << "[";
            ss << expr->export_tree(level);
            ss << "]";
            break;
        }
    case NODE_ADD:
    case NODE_SUB:
    case NODE_MUL:
    case NODE_DIV:
    case NODE_MOD:
    case NODE_LT:
    case NODE_GT:
    case NODE_LE:
    case NODE_GE:
    case NODE_EQ:
    case NODE_DIF:
    case NODE_AND:
    case NODE_OR:
        {
            const auto op = operator_string(node);
            const auto left = this->children[0];
            const auto right = this->children[1];
            ss << left->export_tree(level);
            ss << " " << op << " ";
            ss << right->export_tree(level);
            break;
        }
    case NODE_NOT:
        {
            ss << "~";
            const auto expr = this->children[0];
            ss << expr->export_tree(level);
            break;   
        }
    case NODE_PARENTHESIS:
        {
            ss << "(";
            const auto expr = this->children[0];
            ss << expr->export_tree(level);
            ss << ")";
            break;   
        }
    case NODE_FUN_CALL:
        {
            const auto symbol = this->children[0];
            ss << symbol->export_tree(level);
            const auto args = this->children[1];
            ss << args->export_tree(level);
            break;   
        }
    case NODE_ARG_LIST:
        {
            ss << "(";
            for (const auto &child : this->children)
            {
                ss << child->export_tree(level);
                if (child != this->children.back())
                {
                    ss << ", ";
                }
            }
            ss << ")";
            break;   
        }
    case NODE_IF:
        {
            ss << "if (";
            const auto condition = this->children[0];
            ss << condition->export_tree(level);
            ss << ") ";
            const auto if_block = this->children[1];
            ss << if_block->export_tree(level);
            if (this->children.size() > 2)
            {
                ss << "else ";
                const auto else_block = this->children[2];
                ss << else_block->export_tree(level);
            }
            break;
        }
    case NODE_WHILE:
        {
            ss << "while ";
            const auto condition = this->children[0];
            ss << condition->export_tree(level);
            ss << " do\n";
            const auto while_block = this->children[1];
            ss << while_block->export_tree(level);
            break;
        }
    case NODE_DO_WHILE:
        {
            ss << "do ";
            const auto do_block = this->children[0];
            ss << do_block->export_tree(level);
            ss << std::string(level*2, ' ') << "while ";
            const auto condition = this->children[1];
            ss << condition->export_tree(level);
            ss << ";\n";
            break;
        }
    case NODE_READ:
        {
            ss << "read ";
            const auto read_var = this->children[0];
            ss << read_var->export_tree(level);
            ss << ";\n";
            break;
        }
    case NODE_PRINT:
        {
            ss << "print ";
            const auto print_list = this->children;
            for (const auto &child : print_list)
            {
                ss << child->export_tree(level);
                if (child != print_list.back())
                {
                    ss << " ";
                }
            }
            ss << ";\n";
            break;
        }
    case NODE_RETURN:
        {
            ss << "return ";
            const auto ret_val = this->children[0];
            ss << ret_val->export_tree(level);
            ss << ";\n";
            break;
        }
    case NODE_KW_INT:
        {
            ss << "int";
            break;
        }
    case NODE_KW_REAL:
        {
            ss << "real";
            break;
        }
    case NODE_KW_BYTE:
        {
            ss << "byte";
            break;
        }
    }

        // Find all \n and replace them with \n + level*2 spaces
        std::string str = ss.str();
        // size_t pos = 0;
        // while ((pos = str.find("\n", pos)) != std::string::npos)
        // {
        //     str.replace(pos, 1, "\n" + std::string(level*2, ' '));
        //     pos += level*2 + 1;
        // }
        // Add indentation to the beginning of the string
        return str;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch" // Makes switch exhaustive
std::string Node::export_symbol(SymbolTableEntry symbol) const
{
    switch (symbol->type)
    {
    case SYMBOL_IDENTIFIER:
    case SYMBOL_REAL:
    case SYMBOL_INT:
    case SYMBOL_CHAR:
    case SYMBOL_STRING:
    case SYMBOL_OTHER:
    case SYMBOL_INVALID:
        return symbol->get_text();
    }
}
#pragma clang diagnostic pop