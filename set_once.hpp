#pragma once

// set_once.hpp file made by Ian Kersz Amaral - 2025/1
// This file defines a SetOnce class that allows setting a value only once.
// Useful for cases where you want to ensure a value is set only once, similar to std::optional.
// Adapted from the my function created earlier by Google Gemini

#include <optional>
#include <utility>     // For std::forward, std::move
#include <type_traits> // For SFINAE helpers like std::is_constructible_v, std::decay_t, etc.
#include <stdexcept>   // For std::bad_optional_access

template<typename T>
class SetOnce {
private:
    std::optional<T> m_value;

public:
    // Constructors
    SetOnce() = default; // Creates an empty SetOnce object
    explicit SetOnce(const T& initial_value) : m_value(initial_value) {}
    explicit SetOnce(T&& initial_value) : m_value(std::move(initial_value)) {}

    // Constructor from a type U convertible to T (for initialization)
    // This allows SetOnce<int> s("123"); if T(U) is valid.
    template <
        typename U,
        typename = std::enable_if_t<
            std::is_constructible_v<T, U&&> &&
            !std::is_same_v<std::decay_t<U>, T> && // Avoid ambiguity with T constructors
            !std::is_base_of_v<SetOnce, std::decay_t<U>> // Avoid conflict with copy/move
        >
    >
    explicit SetOnce(U&& initial_value_convertible) : m_value(T(std::forward<U>(initial_value_convertible))) {}

    // Default copy and move constructors are fine, they copy/move the underlying optional
    SetOnce(const SetOnce& other) = default;
    SetOnce(SetOnce&& other) noexcept = default;

    // --- Assignment Operators (Implement "set only if not already set") ---

    // Assign from T (lvalue)
    SetOnce& operator=(const T& value) {
        if (!m_value.has_value()) {
            m_value = value;
        }
        return *this;
    }

    // Assign from T (rvalue)
    SetOnce& operator=(T&& value) {
        if (!m_value.has_value()) {
            m_value = std::move(value);
        }
        return *this;
    }

    // Assign from a generic type ValType (forwarding reference)
    // This operator handles types convertible to T, mimicking the original function's U&&.
    // SFINAE is used to ensure:
    // 1. T can be constructed from ValType.
    // 2. ValType is not T itself (to avoid ambiguity with T overloads).
    // 3. ValType is not SetOnce<T> (to avoid ambiguity with SetOnce overloads).
    template <
        typename ValType,
        typename = std::enable_if_t<
            std::is_constructible_v<T, ValType&&> &&
            !std::is_same_v<std::decay_t<ValType>, T> &&
            !std::is_same_v<std::decay_t<ValType>, SetOnce<T>>
        >
    >
    SetOnce& operator=(ValType&& rhs_value) {
        if (!m_value.has_value()) {
            // Construct T from rhs_value, then assign to m_value.
            // This matches the original function: opt = T(std::forward<U>(value));
            m_value = T(std::forward<ValType>(rhs_value));
        }
        return *this;
    }

    // Assign from another SetOnce<T> (copy assignment)
    SetOnce& operator=(const SetOnce& other) {
        if (this == &other) {
            return *this;
        }
        if (!m_value.has_value() && other.m_value.has_value()) {
            m_value = other.m_value; // Copy the contained optional's value
        }
        return *this;
    }

    // Assign from another SetOnce<T> (move assignment)
    SetOnce& operator=(SetOnce&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (!m_value.has_value() && other.m_value.has_value()) {
            m_value = std::move(other.m_value); // Move the contained optional's value
                                               // other.m_value is now in a valid but unspecified state
        }
        // If this->m_value was already set, 'other' is untouched.
        return *this;
    }

    // --- Accessors and Utility Methods (mimicking std::optional) ---
    bool has_value() const noexcept { return m_value.has_value(); }
    explicit operator bool() const noexcept { return m_value.has_value(); }

    // value() accessors: throw std::bad_optional_access if no value
    const T& value() const& { return m_value.value(); }
    T& value() & { return m_value.value(); }
    const T&& value() const&& { return std::move(m_value).value(); }
    T&& value() && { return std::move(m_value).value(); }

    // operator* (dereference): Undefined behavior if no value
    const T& operator*() const& { return *m_value; }
    T& operator*() & { return *m_value; }
    const T&& operator*() const&& { return std::move(*m_value); }
    T&& operator*() && { return std::move(*m_value); }
    
    // operator-> (member access): Undefined behavior if no value
    const T* operator->() const { return m_value.operator->(); }
    T* operator->() { return m_value.operator->(); }

    template<typename U>
    T value_or(U&& default_value) const& {
        return m_value.value_or(std::forward<U>(default_value));
    }
    template<typename U>
    T value_or(U&& default_value) && {
        return std::move(m_value).value_or(std::forward<U>(default_value));
    }

    void reset() noexcept { m_value.reset(); }

    // Conditional emplace: constructs the value in place if not already set
    // Returns true if value was emplaced, false otherwise.
    template<typename... Args>
    bool try_emplace(Args&&... args) {
        if (!m_value.has_value()) {
            m_value.emplace(std::forward<Args>(args)...);
            return true;
        }
        return false;
    }


    template <typename U> // U is the element type of the std::vector
    bool try_emplace(std::vector<U> value, size_t&& index) // 1. vector by value, index by rvalue-ref
    {
        // Optional: Add a static_assert for clarity if T must be constructible from U&&
        // static_assert(std::is_constructible_v<T, U&&>, "T must be constructible from U&& for this emplace overload");

        if (!m_value.has_value()) { // 2. Checks if SetOnce is already set
            if (index < value.size()) { // 3. Bounds check (lazy evaluation of access)
                // 4. Emplaces T using the vector element
                // value[index] is U&
                // std::forward<U>(value[index]) effectively becomes std::move(value[index])
                // This means T will be constructed with T(U&&) if such a constructor exists.
                m_value.emplace(std::forward<U>(value[index]));
                return true; // Successfully emplaced
            } else {
                // 5. Throws if index is out of range
                throw std::out_of_range("Index out of range for SetOnce::try_emplace(vector, index)");
            }
        }
        return false; // Already had a value, did nothing
    }
};