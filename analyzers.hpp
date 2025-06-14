#pragma once

// analyzers.hpp file made by Ian Kersz Amaral - 2025/1

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include "symbol.hpp"

struct Error
{
    std::string error_type;
    LineNumber line_number;
    std::string message;

    std::string to_string() const
    {
        return error_type + " Error at line " + std::to_string(line_number) + ": " + message;
    }
};

class Analyzer
{
private:
    virtual std::string get_error_type() = 0;
    std::vector<Error> errors;

public:
    void add_error(LineNumber line_number, const std::string &message, const bool discard = false)
    {
        if (discard)
            return; // If discard is true, do not add the error
        errors.push_back({get_error_type(), line_number, message});
    }

    std::string generate_error_messages() const
    {
        std::stringstream ss;
        for (const auto &error : errors)
        {
            ss << error.to_string() << "\n";
        }
        return ss.str();
    }

    bool has_errors() const
    {
        return !errors.empty();
    }

    size_t error_count() const
    {
        return errors.size();
    }

    void order_errors()
    {
        std::sort(errors.begin(), errors.end(), [](const Error &a, const Error &b) {
            return a.line_number < b.line_number;
        });
    }
};

class SemanticAnalyzer final : public Analyzer
{
private:
    std::string get_error_type() override
    {
        return "Semantic";
    }
};

class SyntaxAnalyzer final : public Analyzer
{
private:
    std::string get_error_type() override
    {
        return "Syntax";
    }
};
