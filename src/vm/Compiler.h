#pragma once
#include "Typedefs.h"
#include "Tokenizer.h"
#include "utility/Span.h"
#include "utility/Result.h"
#include "Instruction.h"

enum CompilerErrorCode;
struct CompilerError;

class Compiler
{
public:
    //Compile assembly file into machine code
    Result<std::vector<Instruction>, CompilerError> CompileToMemory(const std::vector<TokenData> tokens);
    Result<std::vector<Instruction>, CompilerError> CompileToMemory(std::string_view inputFilePath);

    //Compile assembly file into machine code then save it to another file.
    Result<void, CompilerError> CompileToFile(const std::vector<TokenData> tokens, std::string_view outputFilePath);
    Result<void, CompilerError> CompileToFile(std::string_view inputFilePath, std::string_view outputFilePath);

private:
    i32 GetRegisterIndex(Token token);
};

enum CompilerErrorCode
{
    None,
    InvalidSyntax,
    InvalidToken
};

struct CompilerError
{
    CompilerErrorCode Code;
    std::string Message;
};

static std::string to_string(CompilerErrorCode value)
{
    switch (value)
    {
    case None:
        return "None";
    case InvalidSyntax:
        return "InvalidSyntax";
    case InvalidToken:
        return "InvalidToken";
    default:
        return "Invalid enum";
    }
}