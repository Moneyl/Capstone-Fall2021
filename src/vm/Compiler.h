#pragma once
#include "Typedefs.h"
#include "Tokenizer.h"
#include "utility/Span.h"
#include "utility/Result.h"

enum CompilerErrorCode;
struct CompilerError;

class Compiler
{
public:
    //Todo: Look into making an owning version of Span<T> to automatically handle de-allocation.
    //Compile assembly file into machine code to be run by VM. Caller must cleanup returned Span<u8> if the functions succeeds
    Result<Span<u8>, CompilerError> CompileToMemory(std::string_view inputFilePath);
    Result<Span<u8>, CompilerError> CompileToMemory(const std::vector<TokenData> tokens);

    //Compile assembly file into machine code to be run by VM, then saves it to another file.
    Result<void, CompilerError> CompileToFile(std::string_view inputFilePath, std::string_view outputFilePath);
    Result<void, CompilerError> CompileToFile(const std::vector<TokenData> tokens, std::string_view outputFilePath);
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