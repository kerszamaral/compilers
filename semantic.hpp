#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <sstream>

#include "symbol.hpp"

struct SemanticError
{
    LineNumber line_number;
    std::string message;

    std::string to_string() const
    {
        return "Error at line " + std::to_string(line_number) + ": " + message;
    }
};

class SemanticAnalyzer
{
private:
    std::vector<SemanticError> errors;

public:
    void add_error(LineNumber line_number, const std::string &message)
    {
        errors.push_back({line_number, message});
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
};
