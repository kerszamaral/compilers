#pragma once

// tac.hpp file made by Ian Kersz Amaral - 2025/1
#include "symbol.hpp"
#include "ast.hpp"
#include <deque>
#include <list>
#include <optional>
#include <memory>

enum TacType
{
    TAC_INVALID,
    TAC_SYMBOL,
    TAC_MOVE,
    TAC_ADD,
    TAC_SUB,
    TAC_MUL,
    TAC_DIV,
    TAC_MOD,
    TAC_LT,
    TAC_GT,
    TAC_LE,
    TAC_GE,
    TAC_EQ,
    TAC_DIF,
    TAC_AND,
    TAC_OR,
    TAC_NOT,
    TAC_LABEL,
    TAC_BEGINFUN,
    TAC_ENDFUN,
    TAC_IFZ,
    TAC_JUMP,
    TAC_CALL,
    TAC_ARG,
    TAC_RET,
    TAC_PRINT,
    TAC_READ,
    TAC_VECLOAD,
    TAC_VECSTORE,
    TAC_BEGINVARS,
    TAC_BEGINCODE,
    TAC_VARBEGIN,
    TAC_VARINIT,
    TAC_VAREND,
    TAC_VECBEGIN,
    TAC_VECINIT,
    TAC_VECZEROS,
    TAC_VECEND
};

std::string tac_type_to_string(TacType type);

typedef struct TAC
{
public:
    typedef std::shared_ptr<TAC> TACptr;
    typedef std::vector<TACptr> TACList;

private:
    TacType type;
    SymbolTableEntry result;
    SymbolTableEntry first_operator;
    SymbolTableEntry second_operator;
    TACptr prev;

public:
    TAC(SymbolTable& symbol_table, TacType type, SymbolTableEntry result, const DataType data_type = DataType::TYPE_OTHER) : type(type), result(result), first_operator(nullptr), second_operator(nullptr), prev(nullptr)
    {
        if (!result)
        {
            this->result = register_temp(symbol_table, data_type);
        }
    }

    TAC(SymbolTable& symbol_table, TacType type, const TACptr result = nullptr, const TACptr first = nullptr, const TACptr second = nullptr, const DataType data_type = DataType::TYPE_OTHER) : type(type), prev(nullptr)
    {
        if (!result)
        {
            this->result = register_temp(symbol_table, data_type);
        }
        else
        {
            this->result = result->get_result();
        }

        if (first)
        {
            first_operator = first->get_result();
        }

        if (second)
        {
            second_operator = second->get_result();
        }
    }

    SymbolTableEntry get_result() const { return this->result; }

    static TACptr generate_code(NodePtr node, SymbolTable &symbol_table);

    static TACptr generate_vars(NodePtr node, SymbolTable &symbol_table);

    static TACptr generate_tacs(NodePtr node, SymbolTable &symbol_table);

    static TACptr join(const TACptr &first, const TACptr &second);

    static TACptr join(const TACList &tac_list);

    template<typename... OtherTACs>
    static TACptr join(const OtherTACs&... others) {
        // Static assert to ensure all arguments are actually TAC types.
        // std::decay_t removes const/references for type checking.
        static_assert((std::is_same_v<TACptr, std::decay_t<OtherTACs>> && ...),
                      "All arguments to join must be TACptr types.");

        // C++17 fold expression over the comma operator.
        // For each tac in others, apply the join method
        // to combine them into a single TACptr.
        return join({others...});
    }

    TacType get_type() const { return this->type; }

    std::string to_string() const;

    static std::string tac_string(TACList tac_list);

    static std::string tac_string_backwards(TACptr tac);

    static TACList build_forward_links(TACptr tac);

    const SymbolTableEntry get_first_operator() const
    {
        return this->first_operator;
    }

    const SymbolTableEntry get_second_operator() const
    {
        return this->second_operator;
    }

    static std::pair<TACList, SymbolTable> optimize(TACList tac_list, const SymbolTable& original_symbol_table);

    static size_t tac_size(const TACList &tac_list)
    {
        size_t size = 0;
        for (const auto &tac : tac_list)
        {
            if (tac->get_type() != TAC_SYMBOL) // Skip symbol TACs
            {
                size++;
            }
        }
        return size;
    }

} TAC;

typedef TAC::TACptr TACptr;
typedef TAC::TACList TACList;

TACptr make_tac_symbol(SymbolTable &symbol_table, const SymbolTableEntry result);

TACptr make_tac_temp(SymbolTable &symbol_table, const TacType type, const DataType data_type, const TACptr first, const TACptr second = nullptr);

TACptr make_tac(SymbolTable &symbol_table, const TacType type, const TACptr result, const TACptr first, const TACptr second = nullptr);

TACptr make_tac(SymbolTable &symbol_table, const TacType type, const SymbolTableEntry result, const TACptr first, const TACptr second = nullptr);

TACptr make_tac(SymbolTable &symbol_table, const TacType type, const SymbolTableEntry symbol);

TACptr make_tac_label(SymbolTable &symbol_table);
