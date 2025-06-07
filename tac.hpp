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

private:
    TacType type;
    SymbolTableEntry result;
    SymbolTableEntry first_operator;
    SymbolTableEntry second_operator;
    TACptr next;
    TACptr prev;

public:
    TAC(TacType type, SymbolTableEntry result) : type(type), result(result), first_operator(nullptr), second_operator(nullptr), next(nullptr), prev(nullptr)
    {
        if (!result)
        {
            this->result = register_temp();
        }
    }

    TAC(TacType type, const TACptr result = nullptr, const TACptr first = nullptr, const TACptr second = nullptr) : type(type), next(nullptr), prev(nullptr)
    {
        if (!result)
        {
            this->result = register_temp();
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

    static TACptr generate_code(NodePtr node);

    static TACptr generate_vars(NodePtr node);

    static TACptr generate_tacs(NodePtr node);

    static TACptr join(const TACptr &first, const TACptr &second);

    static TACptr join(const std::vector<TACptr> &tac_list);

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

    static std::string tac_string(TACptr tac);

    static std::string tac_string_backwards(TACptr tac);

    static TACptr build_forward_links(TACptr tac);

} TAC;

typedef TAC::TACptr TACptr;

TACptr make_tac_symbol(const SymbolTableEntry result = nullptr);

TACptr make_tac_temp(const TacType type, const TACptr first, const TACptr second = nullptr);

TACptr make_tac(const TacType type,  const TACptr result, const TACptr first, const TACptr second = nullptr);

TACptr make_tac(const TacType type, const SymbolTableEntry result, const TACptr first, const TACptr second = nullptr);

TACptr make_tac(const TacType type, const SymbolTableEntry symbol);

TACptr make_tac_label();
