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
    TAC_ATRIB,
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

} TAC;

typedef TAC::TACptr TACptr;

/*
typedef struct TACList
{
private:
    std::deque<TACptr> tacs;

public:
    template<typename... OtherTACLists>
    void join(const OtherTACLists&... others) {
        // Static assert to ensure all arguments are actually TACList types.
        // std::decay_t removes const/references for type checking.
        static_assert((std::is_same_v<TACList, std::decay_t<OtherTACLists>> && ...),
                      "All arguments to join must be TACList types.");

        // C++17 fold expression over the comma operator.
        // For each list in 'other_lists', its internal deque ('tacs_deque')
        // is inserted at the end of the current list's 'tacs_deque'.
        (this->tacs.insert(this->tacs_deque.end(),
                                 others.tacs.begin(),
                                 others.tacs.end()), ...);
    }

    const std::deque<TACptr>& get_instructions() const {
        return tacs;
    }

    auto begin() const { return tacs.begin(); }
    auto end() const { return tacs.end(); }
    auto begin() { return tacs.begin(); }
    auto end() { return tacs.end(); }

    bool empty() const { return tacs.empty(); }
    size_t size() const { return tacs.size(); }

} TACList;
*/
