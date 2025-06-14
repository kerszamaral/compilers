#include "asm.hpp"

#include <sstream>
#include <iomanip>
#include <cstring>
#include <string_view>

std::string variables_asm(const TACList tac_list);

std::string functions_asm(const TACList tac_list);

std::string literals_asm(const SymbolTable &symbol_table);

std::string temporaries_asm(const SymbolTable &symbol_table);

std::string generate_asm(const TACList tac_list, const SymbolTable &symbol_table)
{
    std::stringstream asm_stream;
    
    asm_stream << "\n\n## Functions\n";
    asm_stream << functions_asm(tac_list);
    
    asm_stream << "\n\n## Variables\n";
    asm_stream << variables_asm(tac_list);

    asm_stream << "\n\n## Temporary Variables\n";
    asm_stream << temporaries_asm(symbol_table);

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
    case DataType::TYPE_BOOL:
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
    case DataType::TYPE_BOOL:
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

std::string get_printf_label(const DataType data_type, const SymbolTableEntry symbol, const bool is_scanf = false)
{
    switch (data_type)
    {
    case DataType::TYPE_INT:
    case DataType::TYPE_BOOL:
        return ".L.str.int";
    case DataType::TYPE_CHAR:
        return is_scanf ? ".L.str.scanf_char" : ".L.str.char";
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

std::string math_operation_on_datatype(const TacType operation, const DataType data_type)
{
    switch (data_type)
    {
    case DataType::TYPE_INT:
    case DataType::TYPE_CHAR:
        switch (operation)
        {
        case TacType::TAC_ADD: return "add";
        case TacType::TAC_SUB: return "sub";
        case TacType::TAC_MUL: return "imul";
        case TacType::TAC_MOD: return "idiv";
        case TacType::TAC_DIV: return "idiv";
        default: throw std::runtime_error("Unsupported integer operation.");
        }
    case DataType::TYPE_REAL:
        switch (operation)
        {
        case TacType::TAC_ADD: return "addss";
        case TacType::TAC_SUB: return "subss";
        case TacType::TAC_MUL: return "mulss";
        case TacType::TAC_DIV: return "divss";
        default: throw std::runtime_error("Unsupported real operation.");
        }
    default:
        throw std::runtime_error("Unsupported data type for math operation.");
    }
}


#include <iostream>

std::string cmp_operation_on_datatype(const TacType operation, const DataType data_type)
{

    switch (data_type)
    {
    case DataType::TYPE_INT:
    case DataType::TYPE_CHAR:
        switch (operation)
        {
        case TacType::TAC_LT: return "setl";
        case TacType::TAC_GT: return "setg";
        case TacType::TAC_LE: return "setle";
        case TacType::TAC_GE: return "setge";
        case TacType::TAC_EQ: return "sete";
        case TacType::TAC_DIF: return "setne";
        default: throw std::runtime_error("Unsupported integer comparison operation.");
        }
        break;
    case DataType::TYPE_REAL:
        switch (operation)
        {
        case TacType::TAC_LT: return "setb"; // Below
        case TacType::TAC_GT: return "seta"; // Above
        case TacType::TAC_LE: return "setbe"; // Below or Equal
        case TacType::TAC_GE: return "setae"; // Above or Equal
        case TacType::TAC_EQ: return "sete";
        case TacType::TAC_DIF: return "setne";
        default: throw std::runtime_error("Unsupported real comparison operation.");
        }
        break;
    default:
        throw std::runtime_error("Unsupported data type for comparison operation.");
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
                case DataType::TYPE_BOOL:
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
                asm_stream << "    call printf\n";
                break;
            }
        case TacType::TAC_MOVE:
        case TacType::TAC_ARG:
            {
                const auto move_to_var = tac->get_result();
                const auto move_to_text = get_label_or_text(move_to_var);

                const auto moved_var = tac->get_first_operator();
                const auto moved_text = get_label_or_text(moved_var);

                const auto move_to_type = move_to_var->get_data_type();

                switch (move_to_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    mov eax, dword ptr [rip + " << moved_text << "]\n";
                    asm_stream << "    mov dword ptr [rip + " << move_to_text << "], eax\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movzx eax, byte ptr [rip + " << moved_text << "]\n";
                    asm_stream << "    mov byte ptr [rip + " << move_to_text << "], al\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss xmm0, dword ptr [rip + " << moved_text << "]\n";
                    asm_stream << "    movss dword ptr [rip + " << move_to_text << "], xmm0\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for assignment operation.");
                }
                break;
            }
        case TacType::TAC_ADD:
        case TacType::TAC_SUB:
        case TacType::TAC_MUL:
        case TacType::TAC_DIV:
        case TacType::TAC_MOD:
            {
                const auto result_var = tac->get_result();
                const auto result_text = get_label_or_text(result_var);
                const auto first_op = tac->get_first_operator();
                const auto first_op_text = get_label_or_text(first_op);
                const auto second_op = tac->get_second_operator();
                const auto second_op_text = get_label_or_text(second_op);

                const auto result_type = result_var->get_data_type();

                const auto operation = math_operation_on_datatype(tac->get_type(), result_type);
                switch (result_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    mov eax, dword ptr [rip + " << first_op_text << "]\n";
                    asm_stream << "    " << operation << " eax, dword ptr [rip + " << second_op_text << "]\n";
                    asm_stream << "    mov dword ptr [rip + " << result_text << "], ";
                    asm_stream << (tac->get_type() != TacType::TAC_MOD ? "eax\n" : "edx\n");
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movzx eax, byte ptr [rip + " << first_op_text << "]\n";
                    asm_stream << "    movzx ebx, byte ptr [rip + " << second_op_text << "]\n";
                    asm_stream << "    " << operation << " eax, ebx\n";
                    asm_stream << "    mov byte ptr [rip + " << result_text << "], ";
                    asm_stream << (tac->get_type() != TacType::TAC_MOD ? "al\n" : "dl\n");
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss xmm0, dword ptr [rip + " << first_op_text << "]\n";
                    asm_stream << "    " << operation << " xmm0, dword ptr [rip + " << second_op_text << "]\n";
                    asm_stream << "    movss dword ptr [rip + " << result_text << "], xmm0\n";
                    if (tac->get_type() == TacType::TAC_MOD)
                    {
                        throw std::runtime_error("Real numbers do not support mod operation.");
                    }
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for addition operation.");
                }
                break;
            }
        case TacType::TAC_LT:
        case TacType::TAC_GT:
        case TacType::TAC_LE:
        case TacType::TAC_GE:
        case TacType::TAC_EQ:
        case TacType::TAC_DIF:
            {
                const auto result_var = tac->get_result();
                const auto result_text = get_label_or_text(result_var);
                const auto first_op = tac->get_first_operator();
                const auto first_op_text = get_label_or_text(first_op);
                const auto first_type = first_op->get_data_type();
                const auto second_op = tac->get_second_operator();
                const auto second_op_text = get_label_or_text(second_op);
                const auto second_type = second_op->get_data_type();

                const auto first_load_type = first_type == DataType::TYPE_CHAR ? "byte" : "dword";
                const auto second_load_type = second_type == DataType::TYPE_CHAR ? "byte" : "dword";

                const auto first_op_data_type = first_op->get_data_type();

                const auto mov_type = first_type == DataType::TYPE_CHAR ? "movzx" : "mov";
                switch (first_op_data_type)
                {
                case DataType::TYPE_INT:
                case DataType::TYPE_CHAR:
                    asm_stream << "    " << mov_type << " eax, " << first_load_type <<" ptr [rip + " << first_op_text << "]\n";
                    asm_stream << "    " << mov_type << " ebx, " << second_load_type <<" ptr [rip + " << second_op_text << "]\n";
                    asm_stream << "    cmp eax, ebx\n";
                    asm_stream << "    " << cmp_operation_on_datatype(tac->get_type(), first_op_data_type) << " al\n";
                    asm_stream << "    and al, 1\n";
                    asm_stream << "    mov byte ptr [rip + " << result_text << "], al\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss xmm0, dword ptr [rip + " << first_op_text << "]\n";
                    asm_stream << "    ucomiss xmm0, dword ptr [rip + " << second_op_text << "]\n";
                    asm_stream << "    " << cmp_operation_on_datatype(tac->get_type(), first_op_data_type) << " al\n";
                    asm_stream << "    mov byte ptr [rip + " << result_text << "], al\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for comparison operation.");
                }
                break;
            }
        case TacType::TAC_AND:
        case TacType::TAC_OR:
            {
                const auto result_var = tac->get_result();
                const auto result_text = get_label_or_text(result_var);
                const auto first_op = tac->get_first_operator();
                const auto first_op_text = get_label_or_text(first_op);
                const auto second_op = tac->get_second_operator();
                const auto second_op_text = get_label_or_text(second_op);
                const auto function = tac->get_type() == TacType::TAC_AND ? "and" : "or";

                asm_stream << "    movzx eax, byte ptr [rip + " << first_op_text << "]\n";
                asm_stream << "    movzx ebx, byte ptr [rip + " << second_op_text << "]\n";
                asm_stream << "    " << function << " al, bl\n";
                asm_stream << "    mov byte ptr [rip + " << result_text << "], al\n";
                break;
            }
        case TacType::TAC_NOT:
            {
                const auto result_var = tac->get_result();
                const auto result_text = get_label_or_text(result_var);
                const auto condition_var = tac->get_first_operator();
                const auto condition_text = get_label_or_text(condition_var);
            
                asm_stream << "    movzx eax, byte ptr [rip + " << condition_text << "]\n";
                asm_stream << "    xor eax, 1\n"; // The not instruction results in -1 or 0, but xor gives 1 or 0
                asm_stream << "    mov byte ptr [rip + " << result_text << "], al\n";
                break;
            }
        case TacType::TAC_LABEL:
            {
                const auto label = tac->get_result()->get_text();
                asm_stream << label << ":\n";
                break;
            }
        case TacType::TAC_JUMP:
            {
                const auto jump_label = tac->get_result()->get_text();
                asm_stream << "    jmp " << jump_label << "\n";
                break;
            }
        case TacType::TAC_IFZ:
            {
                const auto condition_var = tac->get_first_operator();
                const auto condition_text = get_label_or_text(condition_var);
                const auto jump_label = tac->get_result()->get_text();
                // Conditions will always be of type bool (char)
                asm_stream << "    movzx eax, byte ptr [rip + " << condition_text << "]\n";
                asm_stream << "    cmp eax, 0\n";
                asm_stream << "    je " << jump_label << "\n"; // Jump if zero
                break;
            }
        case TacType::TAC_RET:
            {
                const auto ret_val = tac->get_result();
                const auto ret_val_text = get_label_or_text(ret_val);
                const auto ret_type = ret_val->get_data_type();
                switch (ret_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    mov eax, dword ptr [rip + " << ret_val_text << "]\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movzx al, byte ptr [rip + " << ret_val_text << "]\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss xmm0, dword ptr [rip + " << ret_val_text << "]\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for return operation.");
                }
                break;
            }
        case TacType::TAC_CALL:
            {
                const auto func_name = tac->get_first_operator()->get_text();
                asm_stream << "    call " << func_name << "\n";
                // If the function returns a value, we need to move it to the result variable
                const auto result_var = tac->get_result();
                const auto result_text = get_label_or_text(result_var);
                const auto result_type = result_var->get_data_type();
                switch (result_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    mov dword ptr [rip + " << result_text << "], eax\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    mov byte ptr [rip + " << result_text << "], al\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss dword ptr [rip + " << result_text << "], xmm0\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for function call result.");
                }
                break;
            }
        case TacType::TAC_READ:
            {
                const auto read_var = tac->get_result();
                const auto read_text = get_label_or_text(read_var);
                const auto read_type = read_var->get_data_type();

                asm_stream << "    lea rdi, [rip + " << get_printf_label(read_type, read_var, true) << "]\n";
                asm_stream << "    lea rsi, [rip + " << read_text << "]\n";
                asm_stream << "    mov al, 0\n";
                asm_stream << "    call scanf\n";
                break;
            }
        case TacType::TAC_VECLOAD:
            {
                const auto result_var = tac->get_result();
                const auto result_text = get_label_or_text(result_var);
                const auto vec_var = tac->get_first_operator();
                const auto vec_text = get_label_or_text(vec_var);

                const auto index_var = tac->get_second_operator();
                const auto index_text = get_label_or_text(index_var);
                const auto index_type = index_var->get_data_type();

                switch (index_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    movsxd rcx, dword ptr [rip + " << index_text << "]\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movzx rcx, byte ptr [rip + " << index_text << "]\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for vector index.");
                }

                asm_stream << "    lea rax, [rip + " << vec_text << "]\n";

                const auto vec_type = vec_var->get_data_type();
                switch (vec_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    mov eax, dword ptr [rax + rcx * " << get_data_type_size(vec_type) << "]\n";
                    asm_stream << "    mov dword ptr [rip + " << result_text << "], eax\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movzx eax, byte ptr [rax + rcx * " << get_data_type_size(vec_type) << "]\n";
                    asm_stream << "    mov byte ptr [rip + " << result_text << "], al\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss xmm0, dword ptr [rax + rcx * " << get_data_type_size(vec_type) << "]\n";
                    asm_stream << "    movss dword ptr [rip + " << result_text << "], xmm0\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for vector load operation.");
                }
                break;   
            }
        case TacType::TAC_VECSTORE:
            {
                const auto vec_var = tac->get_result();
                const auto vec_text = get_label_or_text(vec_var);

                const auto index_var = tac->get_second_operator();
                const auto index_text = get_label_or_text(index_var);
                const auto index_type = index_var->get_data_type();

                switch (index_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    movsxd rcx, dword ptr [rip + " << index_text << "]\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movzx rcx, byte ptr [rip + " << index_text << "]\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for vector index.");
                }

                asm_stream << "    lea rax, [rip + " << vec_text << "]\n";

                const auto value_var = tac->get_first_operator();
                const auto value_text = get_label_or_text(value_var);
                const auto value_type = value_var->get_data_type();

                switch (value_type)
                {
                case DataType::TYPE_INT:
                    asm_stream << "    mov edx, dword ptr [rip + " << value_text << "]\n";
                    asm_stream << "    mov dword ptr [rax + rcx * " << get_data_type_size(value_type) << "], edx\n";
                    break;
                case DataType::TYPE_CHAR:
                    asm_stream << "    movzx edx, byte ptr [rip + " << value_text << "]\n";
                    asm_stream << "    mov byte ptr [rax + rcx * " << get_data_type_size(value_type) << "], dl\n";
                    break;
                case DataType::TYPE_REAL:
                    asm_stream << "    movss xmm0, dword ptr [rip + " << value_text << "]\n";
                    asm_stream << "    movss dword ptr [rax + rcx * " << get_data_type_size(value_type) << "], xmm0\n";
                    break;
                default:
                    throw std::runtime_error("Unsupported data type for vector store operation.");
                }
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

    asm_stream << "\n.L.str.scanf_char:\n";
    asm_stream << "    .asciz \" %c\"\n";
    asm_stream << "    .size .L.str.scanf_char, 3\n";

    asm_stream << "\n.L.str.real:\n";
    asm_stream << "    .asciz \"%f\"\n";
    asm_stream << "    .size .L.str.real, 3\n\n";

    return asm_stream.str();
}

std::string temporaries_asm(const SymbolTable &symbol_table)
{
    std::stringstream asm_stream;
    asm_stream << "    .bss\n"; // Uninitialized data section for temporaries

    const auto temp_filter = [](const SymbolTableEntry &entry) {
        return (entry->ident_type == IdentType::IDENT_VAR && entry->type == SymbolType::SYMBOL_TEMP)
            || entry->ident_type == IdentType::IDENT_PARAM;
    };

    const auto temporaries = filtered_table_entries(symbol_table, temp_filter);

    for (const auto &entry : temporaries)
    {
        const auto &symbol = entry;
        const auto temp_name = symbol->get_text();
        const auto temp_type = symbol->get_data_type();
        const auto size_in_bytes = get_data_type_size(temp_type);
        asm_stream << temp_name << ":\n";
        asm_stream << "    " << get_storage_type(temp_type) << " 0\n"; // Initialize to zero
        asm_stream << "    .size " << temp_name << ", " << size_in_bytes << "\n";
    }

    return asm_stream.str();
}