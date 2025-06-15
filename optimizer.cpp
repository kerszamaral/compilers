#include "tac.hpp"

#include <variant>
#include <algorithm>
#include <numeric>

std::pair<TACList, bool> constant_folding(TACList tac_list, SymbolTable &symbol_table);

std::pair<TACList, bool> constant_propagation(TACList tac_list, SymbolTable &symbol_table);

std::pair<TACList, bool> peephole_opt(const TACList &tac_list, SymbolTable &symbol_table);

SymbolTable remove_unused_symbols(const TACList &final_tacs, const SymbolTable &current_table);

std::pair<TACList, SymbolTable> TAC::optimize(TACList tac_list, const SymbolTable &original_symbol_table)
{
    auto optimized_symbol_table = original_symbol_table;
    auto optimized_tac_list = tac_list;

    bool changed_in_pass = true;
    while (changed_in_pass)
    {
        const auto [fold_result, fold_changed] = constant_folding(optimized_tac_list, optimized_symbol_table);
        optimized_tac_list = fold_result;

        const auto [prop_result, prop_changed] = constant_propagation(optimized_tac_list, optimized_symbol_table);
        optimized_tac_list = prop_result;

        const auto [peep_result, peep_changed] = peephole_opt(optimized_tac_list, optimized_symbol_table);
        optimized_tac_list = peep_result;

        changed_in_pass = fold_changed || prop_changed || peep_changed;
    }

    optimized_symbol_table = remove_unused_symbols(optimized_tac_list, optimized_symbol_table);

    return {optimized_tac_list, optimized_symbol_table};
}

struct Fraction
{
    int num;
    int den;

    // Simplify the fraction by dividing by the greatest common divisor
    void simplify();

    // Friend functions for operator overloading
    friend Fraction operator+(const Fraction &a, const Fraction &b);
    friend Fraction operator-(const Fraction &a, const Fraction &b);
    friend Fraction operator*(const Fraction &a, const Fraction &b);
    friend Fraction operator/(const Fraction &a, const Fraction &b);

    friend bool operator<(const Fraction &a, const Fraction &b);
    friend bool operator>(const Fraction &a, const Fraction &b);
    friend bool operator<=(const Fraction &a, const Fraction &b);
    friend bool operator>=(const Fraction &a, const Fraction &b);
    friend bool operator==(const Fraction &a, const Fraction &b);
    friend bool operator!=(const Fraction &a, const Fraction &b);

    friend std::ostream &operator<<(std::ostream &os, const Fraction &f);
};

using LiteralValue = std::variant<int, Fraction>;

LiteralValue parse_to_variant(const SymbolTableEntry &symbol)
{
    const std::string text = symbol->get_text();
    switch (symbol->get_data_type())
    {
    case TYPE_REAL:
    {
        const auto slash_pos = text.find('/');
        if (slash_pos != std::string::npos)
        {
            const auto dividend = std::stoi(text.substr(0, slash_pos));
            const auto divisor = std::stoi(text.substr(slash_pos + 1));
            return Fraction{dividend, divisor};
        }
        throw std::invalid_argument("Invalid real number format: " + text);
    }
    case TYPE_CHAR:
        return static_cast<int>(text[1]); // skip quotes
    case TYPE_INT:
    case TYPE_BOOL:
        return std::stoi(text);
    default:
        throw std::invalid_argument("Unsupported data type for constant folding: " + data_type_to_str(symbol->get_data_type()));
    }
}

struct OperationVisitor
{
    TacType op;

    template <typename T, typename U>
    LiteralValue operator()(T val1, U val2) const
    {
        // Path 1: Both operands are integers.
        if constexpr (std::is_same_v<T, int> && std::is_same_v<U, int>)
        {
            // Perform pure integer arithmetic and return an int.
            switch (op)
            {
            case TAC_ADD:
                return val1 + val2;
            case TAC_SUB:
                return val1 - val2;
            case TAC_MUL:
                return val1 * val2;
            case TAC_DIV:
                return (val2 != 0) ? val1 / val2 : 0;
            case TAC_MOD:
                return (val2 != 0) ? val1 % val2 : 0;
            case TAC_LT:
                return static_cast<int>(val1 < val2);
            case TAC_GT:
                return static_cast<int>(val1 > val2);
            case TAC_LE:
                return static_cast<int>(val1 <= val2);
            case TAC_GE:
                return static_cast<int>(val1 >= val2);
            case TAC_EQ:
                return static_cast<int>(val1 == val2);
            case TAC_DIF:
                return static_cast<int>(val1 != val2);
            default:
                throw std::invalid_argument("Unsupported operation for integer operands: " + std::to_string(op));
            }
        }
        else // Path 2: At least one operand is a Fraction.
        {
            // Promote both to Fraction and perform rational arithmetic.
            Fraction f1;
            if constexpr (std::is_same_v<T, int>)
            {
                f1 = Fraction{val1, 1};
            }
            else
            { // T must be Fraction
                f1 = val1;
            }

            Fraction f2;
            if constexpr (std::is_same_v<U, int>)
            {
                f2 = Fraction{val2, 1};
            }
            else
            { // U must be Fraction
                f2 = val2;
            }

            switch (op)
            {
            case TAC_ADD:
                return f1 + f2; // Fraction addition
            case TAC_SUB:
                return f1 - f2;
            case TAC_MUL:
                return f1 * f2;
            case TAC_DIV:
                return f1 / f2;
            case TAC_LT:
                return static_cast<int>(f1 < f2);
            case TAC_GT:
                return static_cast<int>(f1 > f2);
            case TAC_LE:
                return static_cast<int>(f1 <= f2);
            case TAC_GE:
                return static_cast<int>(f1 >= f2);
            case TAC_EQ:
                return static_cast<int>(f1 == f2);
            case TAC_DIF:
                return static_cast<int>(f1 != f2);
            case TAC_MOD: // Modulo is undefined for fractions in our context
            default:
                throw std::invalid_argument("Unsupported operation for fraction operands: " + std::to_string(op));
            }
        }
        return 0; // Should not be reached
    }
};

std::string reverse_type(const SymbolType type, std::string &result_str)
{
    switch (type)
    {
    case SYMBOL_INT:
        std::reverse(result_str.begin(), result_str.end());
        return result_str;
    case SYMBOL_REAL:
    {
        const auto slash_pos = result_str.find('/');
        if (slash_pos != std::string::npos)
        {
            std::reverse(result_str.begin(), result_str.begin() + static_cast<long>(slash_pos));
            std::reverse(result_str.begin() + static_cast<long>(slash_pos) + 1, result_str.end());
        }
        return result_str;
    }
    default:
        return result_str; // No change for other types
    }
}

std::pair<TACList, bool> constant_folding(TACList tac_list, SymbolTable &symbol_table)
{
    bool changed = false;
    const std::vector<TacType> optimizable_ops = {
        TAC_ADD, TAC_SUB, TAC_MUL, TAC_DIV, TAC_MOD,
        TAC_LT, TAC_GT, TAC_LE, TAC_GE, TAC_EQ, TAC_DIF};

    const std::vector<TacType> comparison_ops = {
        TAC_LT, TAC_GT, TAC_LE, TAC_GE, TAC_EQ, TAC_DIF};

    TACList new_tac_list;
    new_tac_list.reserve(tac_list.size()); // Reserve space for efficiency

    for (const auto &tac : tac_list)
    {
        const bool is_optimizable_op = std::find(optimizable_ops.begin(), optimizable_ops.end(), tac->get_type()) != optimizable_ops.end();

        if (!is_optimizable_op)
        {
            new_tac_list.push_back(tac); // Keep the original TAC if it's not optimizable
            continue;
        }

        auto op1 = tac->get_first_operator();
        auto op2 = tac->get_second_operator();

        // Check if both operands are valid literals
        if (!(op1 && op2 && op1->ident_type == IDENT_LIT && op2->ident_type == IDENT_LIT))
        {
            new_tac_list.push_back(tac); // Keep the original TAC if operands are not literals
            continue;
        }

        LiteralValue parsed_val1 = parse_to_variant(op1);
        LiteralValue parsed_val2 = parse_to_variant(op2);

        LiteralValue result_val = std::visit(OperationVisitor{tac->get_type()}, parsed_val1, parsed_val2);

        std::string result_str;
        DataType result_data_type;
        SymbolType result_symbol_type;

        const auto dataTypeVisitor = [&](auto &&value)
        {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int>)
            {
                result_str = std::to_string(value);
                result_data_type = TYPE_INT;
                result_symbol_type = SYMBOL_INT;
            }
            else if constexpr (std::is_same_v<T, Fraction>)
            {
                result_data_type = TYPE_REAL;
                result_symbol_type = SYMBOL_REAL;
                // Correctly format the fraction back to a string
                result_str = std::to_string(value.num) + "/" + std::to_string(value.den);
            }
        };

        std::visit(dataTypeVisitor, result_val);

        // If it was a comparison, the data type is bool, even though it's stored as an int
        const bool is_comparison = std::find(comparison_ops.begin(), comparison_ops.end(), tac->get_type()) != comparison_ops.end();
        if (is_comparison)
        {
            result_data_type = TYPE_BOOL;
        }

        result_str = reverse_type(result_symbol_type, result_str);
        auto new_literal_symbol = register_symbol(symbol_table, result_symbol_type, result_str, 0);
        new_literal_symbol->set_types(result_data_type, IDENT_LIT);

        auto source_tac = make_tac_symbol(symbol_table, new_literal_symbol);
        auto move_tac = make_tac(symbol_table, TAC_MOVE, tac->get_result(), source_tac);
        new_tac_list.push_back(source_tac);
        new_tac_list.push_back(move_tac);
        changed = true;
    }

    return {new_tac_list, changed};
}

std::pair<TACList, bool> constant_propagation(TACList tac_list, SymbolTable &symbol_table)
{
    bool changed = false;
    // A map from a temporary variable to the literal constant it holds
    std::map<SymbolTableEntry, SymbolTableEntry> temp_to_literal_map;
    TACList new_tac_list;
    new_tac_list.reserve(tac_list.size()); // Reserve space for efficiency

    for (const auto &tac : tac_list)
    {
        // Check if this TAC redefines (writes to) a temporary that we are tracking.
        // If so, that temporary's value is no longer constant.
        // This shouldn't happen, but we handle it just in case we add another optimization later that might change the value of a temporary.
        if (temp_to_literal_map.count(tac->get_result()))
        {
            temp_to_literal_map.erase(tac->get_result());
        }

        // Check if this is a MOVE from a literal to a temporary. This is an opportunity.
        if (tac->get_type() == TAC_MOVE)
        {
            auto dest = tac->get_result();
            auto source = tac->get_first_operator();

            if (dest && source && dest->type == SYMBOL_TEMP && source->ident_type == IDENT_LIT)
            {
                temp_to_literal_map[dest] = source;
                // Since we will propagate this, we can eliminate the MOVE instruction itself.
                // do NOT add this TAC to our new_tac_list.
                continue;
            }
        }

        // Propagate constants into the operands of the current TAC.
        auto op1 = tac->get_first_operator();
        auto op2 = tac->get_second_operator();
        bool an_operand_was_changed = false;

        if (op1 && temp_to_literal_map.count(op1))
        {
            op1 = temp_to_literal_map.at(op1); // Replace temp with literal
            an_operand_was_changed = true;
        }
        if (op2 && temp_to_literal_map.count(op2))
        {
            op2 = temp_to_literal_map.at(op2); // Replace temp with literal
            an_operand_was_changed = true;
        }

        // Add the modified TAC to the new list.
        if (an_operand_was_changed)
        {
            // Recreate the TAC with the new, propagated operands.
            // We need TACs for the operands, not just symbols.
            auto op1_tac = op1 ? make_tac_symbol(symbol_table, op1) : nullptr;
            auto op2_tac = op2 ? make_tac_symbol(symbol_table, op2) : nullptr;
            auto new_tac = make_tac(symbol_table, tac->get_type(), tac->get_result(), op1_tac, op2_tac);

            // We must also include the new operand TACs in the stream if they weren't there before
            if (op1_tac)
                new_tac_list.push_back(op1_tac);
            if (op2_tac)
                new_tac_list.push_back(op2_tac);
            new_tac_list.push_back(new_tac);
            changed = true;
        }
        else
        {
            // No changes, just add the original tac.
            new_tac_list.push_back(tac);
        }
    }

    return {new_tac_list, changed};
}

std::pair<TACList, bool> peephole_opt(const TACList &tac_list, SymbolTable &symbol_table)
{
    bool changed = false;
    TACList new_tac_list;
    new_tac_list.reserve(tac_list.size());

    for (const auto &tac : tac_list)
    {
        // Algebraic Simplification (x + 0, x * 1, x - 0, x * 0)
        if (!(tac->get_type() == TAC_ADD || tac->get_type() == TAC_SUB || tac->get_type() == TAC_MUL))
        {
            new_tac_list.push_back(tac); // Keep the original TAC if it's not an algebraic operation
            continue;
        }
        auto op1 = tac->get_first_operator();
        auto op2 = tac->get_second_operator();

        // Check if one operand is a literal and the other is not.
        if (!(op1 && op2 && (op1->ident_type == IDENT_LIT) != (op2->ident_type == IDENT_LIT)))
        {
            new_tac_list.push_back(tac); // Keep the original TAC if the operands are not one literal and one variable
            continue;
        }
        SymbolTableEntry literal_op = (op1->ident_type == IDENT_LIT) ? op1 : op2;
        SymbolTableEntry var_op = (op1->ident_type == IDENT_LIT) ? op2 : op1;

        // We only handle integer identities 0 and 1 for simplicity here.
        if (literal_op->get_data_type() != TYPE_INT)
        {
            new_tac_list.push_back(tac); // Keep the original TAC if the literal is not an integer
            continue;
        }

        int lit_val = std::stoi(literal_op->get_text());

        // Check for identity operations: x + 0, x - 0, x * 1
        if ((tac->get_type() == TAC_ADD && lit_val == 0) ||
            (tac->get_type() == TAC_SUB && lit_val == 0 && op2 == literal_op) || // Ensure it's x - 0, not 0 - x
            (tac->get_type() == TAC_MUL && lit_val == 1))
        {
            // Replace the operation with: MOVE result, var_op
            auto source_tac = make_tac_symbol(symbol_table, var_op);
            auto move_tac = make_tac(symbol_table, TAC_MOVE, tac->get_result(), source_tac);
            new_tac_list.push_back(source_tac);
            new_tac_list.push_back(move_tac);
            changed = true;
        }

        // Check for nullifying operation: x * 0
        if (tac->get_type() == TAC_MUL && lit_val == 0)
        {
            // Replace with: MOVE result, 0
            auto zero_symbol = register_symbol(symbol_table, SYMBOL_INT, "0", 0);
            zero_symbol->set_types(TYPE_INT, IDENT_LIT);
            auto source_tac = make_tac_symbol(symbol_table, zero_symbol);
            auto move_tac = make_tac(symbol_table, TAC_MOVE, tac->get_result(), source_tac);
            new_tac_list.push_back(source_tac);
            new_tac_list.push_back(move_tac);
            changed = true;
        }
    }
    return {new_tac_list, changed};
}

SymbolTable remove_unused_symbols(const TACList &final_tacs, const SymbolTable &current_table)
{
    std::set<SymbolTableEntry> used_symbols;
    for (const auto &tac : final_tacs)
    {
        auto op1 = tac->get_first_operator();
        if (op1)
            used_symbols.insert(op1);

        auto op2 = tac->get_second_operator();
        if (op2)
            used_symbols.insert(op2);

        switch (tac->get_type())
        {
        case TAC_PRINT:
        case TAC_RET:
        case TAC_VECSTORE:
            if (tac->get_result())
                used_symbols.insert(tac->get_result());
            break;
        default:
            break;
        }
    }

    SymbolTable new_symbol_table;
    for (const auto &[lexeme, symbol] : current_table)
    {
        // A symbol is kept if it's NOT a temporary or a literal,
        // OR if it IS a temporary or literal, it MUST be in the 'used_symbols' set.
        const bool is_optimizable = (symbol->type == SYMBOL_TEMP || symbol->ident_type == IDENT_LIT);

        if (!is_optimizable || used_symbols.count(symbol))
        {
            new_symbol_table[lexeme] = symbol;
        }
        // Otherwise, the symbol is an unused temporary or an unused literal and is discarded.
    }

    return new_symbol_table;
}

void Fraction::simplify()
{
    if (den == 0)
        return;
    int common = std::gcd(num, den);
    num /= common;
    den /= common;
    if (den < 0)
    {
        den = -den;
        num = -num;
    }
}

Fraction operator+(const Fraction &a, const Fraction &b)
{
    Fraction result = {a.num * b.den + b.num * a.den, a.den * b.den};
    result.simplify();
    return result;
}

Fraction operator-(const Fraction &a, const Fraction &b)
{
    Fraction result = {a.num * b.den - b.num * a.den, a.den * b.den};
    result.simplify();
    return result;
}

Fraction operator*(const Fraction &a, const Fraction &b)
{
    Fraction result = {a.num * b.num, a.den * b.den};
    result.simplify();
    return result;
}

Fraction operator/(const Fraction &a, const Fraction &b)
{
    Fraction result = {a.num * b.den, a.den * b.num};
    result.simplify();
    return result;
}

// For comparisons, we use cross-multiplication to avoid floating-point division
bool operator<(const Fraction &a, const Fraction &b) { return a.num * b.den < b.num * a.den; }
bool operator>(const Fraction &a, const Fraction &b) { return a.num * b.den > b.num * a.den; }
bool operator<=(const Fraction &a, const Fraction &b) { return a.num * b.den <= b.num * a.den; }
bool operator>=(const Fraction &a, const Fraction &b) { return a.num * b.den >= b.num * a.den; }
bool operator==(const Fraction &a, const Fraction &b) { return a.num * b.den == b.num * a.den; }
bool operator!=(const Fraction &a, const Fraction &b) { return a.num * b.den != b.num * a.den; }

std::ostream &operator<<(std::ostream &os, const Fraction &f)
{
    if (f.den == 1)
        os << f.num; // Integer representation
    else
        os << f.num << '/' << f.den; // Fractional representation
    return os;
}
