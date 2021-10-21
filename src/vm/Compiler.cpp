#include "Compiler.h"
#include "utility/File.h"
#include "utility/String.h"
#include <stdexcept>
#include <string>

Result<std::vector<Instruction>, CompilerError> Compiler::CompileToMemory(const std::vector<TokenData> tokens)
{
    //Parse tokens and output machine code
    std::vector<Instruction> out;
    u32 pos = 0;
    while (pos < out.size())
    {
        const TokenData* cur = &tokens[0];
        const TokenData* next = &tokens[pos + 1];
        const TokenData* next2 = &tokens[pos + 2]; //Todo: Come up with a better way of handling this so we can't go out of bounds
        Instruction instruction;

        switch (cur->Type)
        {
        case MOV:
            if (next->Register() && next2->Register())
            {
                instruction.RegisterOp.Opcode = (u16)Opcodes::MOV;
                instruction.RegisterOp.Reg0 = GetRegisterIndex(next->Type);
                instruction.RegisterOp.Reg1 = GetRegisterIndex(next2->Type);
            }
            else if (next->Register() && next2->Type == NUMBER)
            {
                instruction.RegisterValueOp.Opcode = (u16)Opcodes::MOV_VAL;
                instruction.RegisterValueOp.Reg0 = GetRegisterIndex(next->Type);
                instruction.RegisterValueOp.Value = String::ToShort(next2->String);
            }
            else
                return Error(CompilerError{ InvalidSyntax, "Invalid mov syntax! Expects 'mov reg reg' or 'mov reg value'" });

            break;
        case ADD:
            break;
        case SUB:
            break;
        case MUL:
            break;
        case DIV:
            break;
        case CMP:
            break;
        case JMP:
            break;
        case JEQ:
            break;
        case JNE:
            break;
        case JGR:
            break;
        case JLS:
            break;
        case CALL:
            break;
        case RET:
            break;
        case AND:
            break;
        case OR:
            break;
        case XOR:
            break;
        case NEG:
            break;
        case LOAD:
            break;
        case STORE:
            break;
        case PUSH:
            break;
        case POP:
            break;
        case REGISTER0:
        case REGISTER1:
        case REGISTER2:
        case REGISTER3:
        case REGISTER4:
        case REGISTER5:
        case REGISTER6:
        case REGISTER7:
            break;
        case NUMBER:
            break;
        default:
            break;
        }

        out.push_back(instruction);
    }

    return Success(out);
}

Result<std::vector<Instruction>, CompilerError> Compiler::CompileToMemory(std::string_view inputFilePath)
{
    //Read file to string, tokenize, and compile
    std::string fileString = File::ReadAll(inputFilePath);
    std::vector<TokenData> tokens = Tokenizer::Tokenize(fileString);
    return CompileToMemory(tokens);
}

Result<void, CompilerError> Compiler::CompileToFile(const std::vector<TokenData> tokens, std::string_view outputFilePath)
{
    //Compile from tokens
    Result<std::vector<Instruction>, CompilerError> compileResult = CompileToMemory(tokens);

    //Handle compile errors
    if (compileResult.Error())
        return Error(compileResult.ErrorData());

    //No compile errors, write machine code to output file
    File::WriteAllBytes<Instruction>(outputFilePath, compileResult.SuccessData());
    return Success<void>();
}

Result<void, CompilerError> Compiler::CompileToFile(std::string_view inputFilePath, std::string_view outputFilePath)
{
    //Read file to string, tokenize, and compile
    std::string fileString = File::ReadAll(inputFilePath);
    std::vector<TokenData> tokens = Tokenizer::Tokenize(fileString);
    Result<std::vector<Instruction>, CompilerError> compileResult = CompileToMemory(tokens);
    
    //Handle compile errors
    if (compileResult.Error())
        return Error(compileResult.ErrorData());

    //No compile errors, write machine code to output file
    File::WriteAllBytes<Instruction>(outputFilePath, compileResult.SuccessData());
    return Success<void>();
}

i32 Compiler::GetRegisterIndex(Token token)
{
    if (token < REGISTER0 || token > REGISTER7)
        throw std::runtime_error("Invalid register token enum '" + std::to_string(token) + "' passed to Compiler::GetRegisterIndex()");

    return token - REGISTER0;
}
