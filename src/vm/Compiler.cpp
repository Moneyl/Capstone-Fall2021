#include "Compiler.h"
#include "utility/File.h"
#include "utility/String.h"
#include <stdexcept>
#include <string>

Result<std::vector<Instruction>, CompilerError> Compiler::CompileToMemory(const std::vector<TokenData> tokens)
{
    //Index of the current token the compiler is at
    u32 curTokenIndex = 0;
    //Gets data for next token. Returns Token::NONE if curTokenIndex + distance is out of bounds
    auto PeekToken = [&](u32 distance) -> TokenData { return (curTokenIndex + distance < tokens.size()) ? tokens[curTokenIndex + distance] : TokenData{ Token::None, "" }; };

    //Parse tokens and output instructions
    std::vector<Instruction> instructions;
    while (curTokenIndex < tokens.size())
    {
        Instruction instruction = { 0 };

        //Get token data
        const TokenData cur = tokens[curTokenIndex];
        const TokenData next = PeekToken(1);
        const TokenData next2 = PeekToken(2); //Is Token::NONE if pos + 2 is out of bounds
        const TokenData next3 = PeekToken(3); //Is Token::NONE if pos + 3 is out of bounds
        const TokenData next4 = PeekToken(4); //Is Token::NONE if pos + 4 is out of bounds

        //Parse tokens and output instructions
        switch (cur.Type)
        {
        //Instructions using 'op register (register|value|address)' syntax
        case Token::Mov:
        case Token::Add:
        case Token::Sub:
        case Token::Mul:
        case Token::Div:
        case Token::Cmp:
        case Token::And:
        case Token::Or:
        case Token::Xor:
            if (next.IsRegister() && next2.IsRegister() && next3.Type == Token::Newline)
            {
                instruction.OpRegisterRegister.Opcode = (u16)(cur.Type);
                instruction.OpRegisterRegister.Reg0 = GetRegisterIndex(next.Type);
                instruction.OpRegisterRegister.Reg1 = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else if (next.IsRegister() && next2.Type == Token::Value && next3.Type == Token::Newline)
            {
                //The Token enum is equivalent to the Opcode enum for easy conversion
                //Opcodes that have register and value variants can be converted by adding 1. E.g. Mov = 0, MovVal = Mov + 1
                instruction.OpRegisterValue.Opcode = (u16)(cur.Type) + 1;
                instruction.OpRegisterValue.Reg0 = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = String::ToShort(next2.String);
                curTokenIndex += 4;
            }
            else
                return Error(CompilerError{ InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " register (register|value)'" });

            break;

        //Load instruction
        case Token::Load:
            if (next.IsRegister() && next2.Type == Token::Value && next3.Type == Token::Newline)
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Load;
                instruction.OpRegisterValue.Reg0 = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = String::ToShort(next2.String);
                curTokenIndex += 4;
            }
            else if (next.IsRegister() && next2.IsRegister() && next3.Type == Token::Newline)
            {
                instruction.OpRegisterRegister.Opcode = (u16)Opcode::LoadP;
                instruction.OpRegisterRegister.Reg0 = GetRegisterIndex(next.Type);
                instruction.OpRegisterRegister.Reg1 = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else
                return Error(CompilerError{ InvalidSyntax, "Invalid store syntax. Expects 'load register register' or 'load register address'" });

            break;

        //Store instruction
        case Token::Store:
            if (next.Type == Token::Value && next2.IsRegister() && next3.Type == Token::Newline)
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Store;
                instruction.OpRegisterValue.Value = String::ToShort(next.String);
                instruction.OpRegisterValue.Reg0 = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else if (next.IsRegister() && next2.IsRegister() && next3.Type == Token::Newline)
            {
                instruction.OpRegisterRegister.Opcode = (u16)Opcode::StoreP;
                instruction.OpRegisterRegister.Reg0 = GetRegisterIndex(next.Type);
                instruction.OpRegisterRegister.Reg1 = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else
                return Error(CompilerError{ InvalidSyntax, "Invalid store syntax. Expects 'store register register' or 'store address register'" });

            break;

        //Instructions using 'op address' syntax
        case Token::Jmp:
        case Token::Jeq:
        case Token::Jne:
        case Token::Jgr:
        case Token::Jls:
        case Token::Call:
            if (next.Type == Token::Value && next2.Type == Token::Newline)
            {
                instruction.OpAddress.Opcode = (u16)cur.Type;
                instruction.OpAddress.Address = String::ToShort(next.String);
                curTokenIndex += 3;
            }
            else
                return Error(CompilerError{ InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " address'" });

            break;

        //Return instruction
        case Token::Ret:
            if (next.Type == Token::Newline)
            {
                instruction.Op.Opcode = (u16)cur.Type;
                curTokenIndex += 2;
            }
            else 
                return Error(CompilerError{ InvalidSyntax, "Invalid 'ret' syntax. Expects no arguments." });

            break;

        //Instructions using 'op register' syntax
        case Token::Neg:
        case Token::Push:
        case Token::Pop:
            if (next.IsRegister() && next2.Type == Token::Newline)
            {
                instruction.OpRegister.Opcode = (u16)cur.Type;
                instruction.OpRegister.Reg = GetRegisterIndex(next.Type);
                curTokenIndex += 3;
            }
            else
                return Error(CompilerError{ InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " register'" });

            break;

        //Ignore blank lines
        case Token::Newline:
            curTokenIndex += 1;
            continue;

        //These tokens shouldn't be standalone
        case Token::Register0:
        case Token::Register1:
        case Token::Register2:
        case Token::Register3:
        case Token::Register4:
        case Token::Register5:
        case Token::Register6:
        case Token::Register7:
        case Token::Value:
            return Error(CompilerError{ InvalidSyntax, "Invalid syntax. '" + to_string(cur.Type) + "' should only be used as an instruction argument." });

        default:
            return Error(CompilerError{ InvalidToken, "Unknown token '" + std::string(cur.String) + "' passed to compiler." });
        }

        instructions.push_back(instruction);
    }

    return Success(instructions);
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
    if (token < Token::Register0 || token > Token::Register7)
        throw std::runtime_error("Invalid register token enum '" + to_string(token) + "' passed to Compiler::GetRegisterIndex()");

    return (u16)token - (u16)Token::Register0;
}
