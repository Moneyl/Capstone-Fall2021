#pragma once
#include "Typedefs.h"
#include "Tokenizer.h"
#include "utility/Span.h"
#include "utility/Result.h"
#include "Instruction.h"
#include "VmProgram.h"
#include <magic_enum.hpp>

struct CompilerError;

//Compiles assembly to a program that the VM can run
class Compiler
{
public:
    //Compile assembly file into program binary
    Result<VmProgram, CompilerError> Compile(const std::vector<TokenData>& tokens);
    Result<VmProgram, CompilerError> Compile(std::string_view source);
    Result<VmProgram, CompilerError> CompileFile(std::string_view inputFilePath);

private:
    i32 GetRegisterIndex(Token token);
};

enum class CompilerErrorCode
{
    None,
    InvalidSyntax,
    UnsupportedToken,
    TokenizationError,
    DuplicateLabel,
    DuplicateVariable,
};

struct CompilerError
{
    CompilerErrorCode Code;
    std::string Message;
};

static std::string to_string(CompilerErrorCode value)
{
    return std::string(magic_enum::enum_name(value));
}