#include "asm.hpp"

#include <sstream>
#include <iomanip>
#include <cstring>
#include <string_view>

std::string variables_asm(const TACList tac_list);

std::string functions_asm(const TACList tac_list);

std::string literals_asm(const SymbolTable &symbol_table);

std::string generate_asm(const TACList tac_list, const SymbolTable &symbol_table)
{
    std::stringstream asm_stream;
    asm_stream << "\n\n## Variables\n";
    asm_stream << variables_asm(tac_list);

    asm_stream << "\n\n## Functions\n";
    asm_stream << functions_asm(tac_list);
    
    asm_stream << "\n\n## Literals\n";
    asm_stream << literals_asm(symbol_table);

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

std::string get_storage_type(const DataType data_type)
{
    switch (data_type)
    {
    case DataType::TYPE_INT:
        return ".long";
    case DataType::TYPE_CHAR:
        return ".byte";
    case DataType::TYPE_REAL:
        return ".long";
    default:
        throw std::runtime_error("Unsupported data type for storage type.");
    }
}

std::string get_label_type(const DataType data_type)
{
    switch (data_type)
    {
    case DataType::TYPE_INT:
        return "int";
    case DataType::TYPE_CHAR:
        return "char";
    case DataType::TYPE_REAL:
        return "real";
    case DataType::TYPE_STRING:
        return "str";
    default:
        throw std::runtime_error("Unsupported data type for label type.");
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
                asm_stream << "    " << get_storage_type(current_data_type) << " ";
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

std::string get_printf_label(const DataType data_type, const SymbolTableEntry symbol)
{
    switch (data_type)
    {
    case DataType::TYPE_INT:
        return ".L.str.int";
    case DataType::TYPE_CHAR:
        return ".L.str.char";
    case DataType::TYPE_REAL:
        return ".L.str.real";
    case DataType::TYPE_STRING:
        {
            const auto text_label = std::hash<std::string>()(symbol->get_text());
            return ".L.str." + std::to_string(text_label);
        }
    default:
        throw std::runtime_error("Unsupported data type for printf label.");
    }
}

std::string get_label_or_text(const SymbolTableEntry &symbol)
{
    if (symbol->ident_type == IdentType::IDENT_LIT)
    {
        const auto text_label = std::hash<std::string>()(symbol->get_text());
        return ".L." + get_label_type(symbol->get_data_type()) + "." + std::to_string(text_label);
    }
    else
    {
        return symbol->get_text();
    }
}

std::string functions_asm(const TACList tac_list)
{
    std::stringstream asm_stream;
    asm_stream << "    .text\n";
    asm_stream << "    .p2align 4\n";

    for (const auto &tac : tac_list)
    {
        switch (tac->get_type())
        {
        case TacType::TAC_BEGINFUN:
            {
                const auto func_name = tac->get_result()->get_text();
                asm_stream << "    .globl " << func_name << "\n";
                asm_stream << func_name << ":\n";
                if (func_name == "_main")
                {
                    asm_stream << "    .global main\n";
                    asm_stream << "main:\n";
                }

                asm_stream << "    .cfi_startproc\n";
                asm_stream << "    push rbp\n";
                break;
            }
        case TacType::TAC_ENDFUN:
            {
                static size_t func_counter = 0;
                const auto func_name = tac->get_result()->get_text();
                asm_stream << "    pop rbp\n";
                asm_stream << "    ret\n";
                // asm_stream << ".Lfunc_end" << func_counter << ":\n";
                // asm_stream << "    .size " << func_name << ", .Lfunc_end" << func_counter << " - " << func_name << "\n";
                asm_stream << "    .cfi_endproc\n";
                func_counter++;
                break;
            }
        case TacType::TAC_PRINT:
            {
                // currently only supports printing strings
                const auto print_var = tac->get_result();
                const auto print_type = print_var->get_data_type();
                switch (print_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    mov esi, dword ptr [rip + " << print_var->get_text() << "]\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movsx esi, byte ptr [rip + " << print_var->get_text() << "]\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss xmm0, dword ptr [rip + " << print_var->get_text() << "]\n";
                    asm_stream << "    cvtss2sd xmm0, xmm0\n"; // Convert float to double for printf
                    break;
                case DataType::TYPE_STRING:
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for print operation.");
                }

                const auto label = get_printf_label(print_var->get_data_type(), print_var);
                asm_stream << "    lea rdi, [rip + " << label << "]\n";

                asm_stream << "    mov al, " << (print_type == DataType::TYPE_REAL ? '1' : '0') << "\n";
                asm_stream << "    call printf@PLT\n";
                break;
            }
        default:
            break;
        }
    }

    return asm_stream.str();
}


int get_processed_string_length(std::string_view s) {
    std::string_view content_view = s;

    // Remove leading and trailing quotes if present
    if (content_view.length() >= 2 && content_view.front() == '"' && content_view.back() == '"') {
        content_view.remove_prefix(1);
        content_view.remove_suffix(1);
    }

    // Calculate length, accounting for escape sequences
    int length = 0;
    for (size_t i = 0; i < content_view.length(); ++i) {
        length++;
        if (content_view[i] == '\\' && i + 1 < content_view.length()) {
            i++;
        }
    }
    return length;
}

std::string literals_asm(const SymbolTable &symbol_table)
{
    std::stringstream asm_stream;
    // asm_stream << ".data\n";
    asm_stream << ".section .rodata\n"; // Read-only data section for literals

    const auto literal_filter = [](const SymbolTableEntry &entry) {
        return entry->ident_type == IdentType::IDENT_LIT;
    };

    const auto literals = filtered_table_entries(symbol_table, literal_filter);

    for (const auto &entry : literals)
    {
        const auto &symbol = entry;
        const auto text = symbol->get_text();
        const auto data_type = symbol->get_data_type();
        const auto label = get_label_or_text(symbol);
        const auto size_in_bytes = symbol->type == SymbolType::SYMBOL_STRING
            ? get_processed_string_length(text) + 1 // +1 for null terminator
            : get_data_type_size(data_type);

        asm_stream << "\n" << label << ":\n";
        if (symbol->type == SymbolType::SYMBOL_STRING)
        {
            asm_stream << "    .asciz " << text << "\n";
        }
        else if (symbol->type == SymbolType::SYMBOL_INT || symbol->type == SymbolType::SYMBOL_CHAR || symbol->type == SymbolType::SYMBOL_REAL)
        {
            asm_stream << "    " << get_storage_type(data_type) << " " << value_representation(text, data_type) << "\n";
        }

        asm_stream << "    .size " << label << ", " << size_in_bytes << "\n";
    }

    // return ".L.str.int";
    // return ".L.str.char";
    // return ".L.str.real";
    // Always preload the printf strings
    asm_stream << "\n.L.str.int:\n";
    asm_stream << "    .asciz \"%d\"\n";
    asm_stream << "    .size .L.str.int, 3\n";

    asm_stream << "\n.L.str.char:\n";
    asm_stream << "    .asciz \"%c\"\n";
    asm_stream << "    .size .L.str.char, 3\n";

    asm_stream << "\n.L.str.real:\n";
    asm_stream << "    .asciz \"%f\"\n";
    asm_stream << "    .size .L.str.real, 3\n\n";

    return asm_stream.str();
}
