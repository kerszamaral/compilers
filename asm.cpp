#include "asm.hpp"

#include <sstream>

std::string variables_asm(const TACList tac_list);

std::string generate_asm(const TACList tac_list, const SymbolTable &symbol_table)
{
    std::stringstream asm_stream;
    asm_stream << variables_asm(tac_list);

    return asm_stream.str();
}


std::string variables_asm(const TACList tac_list)
{
    std::stringstream asm_stream;
    asm_stream << ".data\n";

    for (const auto &tac : tac_list)
    {
        switch (tac->get_type())
        {
        case TacType::TAC_VARBEGIN:
            {
                asm_stream << "\n";
                const auto var = tac->get_result();
                const auto var_name = var->get_text();
                asm_stream << "    .globl " << var_name << "\n";
                
                switch (var->get_data_type())
                {
                case DataType::TYPE_INT:
                    asm_stream << "    .p2align 2\n";
                    break;
                case DataType::TYPE_CHAR:
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    .p2align 2\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for variable initialization in assembly generation.");
                }
                
                asm_stream << var_name << ":\n";
                
                // Need to determine the data type of the variable to emit the correct data type
                switch (var->get_data_type())
                {
                case DataType::TYPE_INT:
                    asm_stream << "    .long ";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    .byte ";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    .long ";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for variable initialization in assembly generation.");
                }
                break;
            }
        case TacType::TAC_VARINIT:
            {
                // Do not need to convert characters from ASCII
                const auto init_value = tac->get_result();
                asm_stream << init_value->get_text() << "\n";
                break;
            }
        case TacType::TAC_VAREND:
            {
                const auto var = tac->get_result();
                asm_stream << "    .size ";
                asm_stream << var->get_text() << ", ";
                switch (var->get_data_type())
                {
                case DataType::TYPE_INT:
                    asm_stream << "4\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "1\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "8\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for variable end in assembly generation.");
                }
                break;
            }
        default:
            break;
        }
    }

    return asm_stream.str();
}