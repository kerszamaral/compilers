#include "asm.hpp"

#include <sstream>
#include <iomanip>
#include <cstring>

std::string variables_asm(const TACList tac_list);

std::string generate_asm(const TACList tac_list, const SymbolTable &symbol_table)
{
    std::stringstream asm_stream;
    asm_stream << "\n\n## Variables\n";
    asm_stream << variables_asm(tac_list);

    return asm_stream.str();
}

int get_data_type_size(const DataType data_type)
{
    switch (data_type)
    {
    case DataType::TYPE_INT:
        return 4; // 4 bytes for int
    case DataType::TYPE_CHAR:
        return 1; // 1 byte for char
    case DataType::TYPE_REAL:
        return 4; // 4 bytes for real (float)
    default:
        throw std::runtime_error("Unsupported data type size requested.");
    }
}

// Function taken from https://stackoverflow.com/a/5100745
template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}

std::string value_representation(const std::string &value, const DataType data_type)
{
    switch (data_type)
    {
    case DataType::TYPE_INT:
        return value; // Integer values are represented as is
    case DataType::TYPE_CHAR:
        return value; // Character values are represented as is
    case DataType::TYPE_REAL:
        {
            const auto slash_pos = value.find('/');
            const auto dividend = std::stoi(value.substr(0, slash_pos));
            const auto divisor = std::stoi(value.substr(slash_pos + 1));
            if (divisor == 0)
            {
                throw std::runtime_error("Division by zero in real value representation.");
            }
            const float real_value = static_cast<float>(dividend) / static_cast<float>(divisor);
            // Convert float to binary representation
            uint32_t float_bits;
            std::memcpy(&float_bits, &real_value, sizeof(float_bits));
            return int_to_hex(float_bits); // Return the hex representation of the float
        }
    default:
        throw std::runtime_error("Unsupported data type for value representation.");
    }
}

std::string variables_asm(const TACList tac_list)
{
    std::stringstream asm_stream;
    asm_stream << "    .data\n";

    DataType current_data_type = DataType::TYPE_INVALID;

    for (const auto &tac : tac_list)
    {
        switch (tac->get_type())
        {
        case TacType::TAC_VARBEGIN:
        case TacType::TAC_VECBEGIN:
            {
                asm_stream << "\n";
                const auto var = tac->get_result();
                const auto var_name = var->get_text();
                asm_stream << "    .globl " << var_name << "\n";

                if (tac->get_type() == TAC_VECBEGIN)
                {
                    // Vectors are aligned to 4 bytes
                    asm_stream << "    .p2align 4\n";
                }
                else
                {
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
                }
                
                asm_stream << var_name << ":\n";
                current_data_type = var->get_data_type();
                break;
            }
        case TacType::TAC_VARINIT:
        case TacType::TAC_VECINIT:
            {
                // Do not need to convert characters from ASCII
                const auto init_value = tac->get_result();
                // Need to determine the data type of the variable to emit the correct data type
                switch (current_data_type)
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
                asm_stream << value_representation(init_value->get_text(), current_data_type) << "\n";
                break;
            }
        case TacType::TAC_VAREND:
        case TacType::TAC_VECEND:
            {
                const auto is_vector = tac->get_type() == TacType::TAC_VECEND;

                const auto var = tac->get_result();
                const auto var_name = var->get_text();

                const auto size = !is_vector ? 1 : std::stoi(tac->get_first_operator()->get_text());
                const auto size_in_bytes = get_data_type_size(current_data_type) * size;

                asm_stream << "    .size " << var_name << ", " << size_in_bytes << "\n";
                current_data_type = DataType::TYPE_INVALID; // Reset current data type
                break;
            }
        case TacType::TAC_VECZEROS:
            {
                const auto vec_size = tac->get_result()->get_text();
                const auto size_in_bytes = get_data_type_size(current_data_type) * std::stoi(vec_size);
                asm_stream << "    .zero " << size_in_bytes << "\n";
                break;
            }
        default:
            break;
        }
    }

    return asm_stream.str();
}