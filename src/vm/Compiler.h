#pragma once
#include "Typedefs.h"
#include "Tokenizer.h"
#include "utility/Span.h"
#include "utility/Result.h"
#include "Instruction.h"
#include "VmProgram.h"
#include <magic_enum.hpp>
#include <array>

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
    //Get index of a register from the corresponding token (e.g. Token::Register0, Token::Register1, etc)
    i32 GetRegisterIndex(const TokenData& token);
    //Checks if the provided pattern is next in the stream. If true it returns the taken data and increments _curTokenIndex.
    template<size_t numTokens>
    std::optional<std::array<TokenData, numTokens>> Expect(std::initializer_list<Token> pattern);

    std::vector<TokenData> _tokens = {};
    size_t _curTokenIndex = 0;
};

enum class CompilerErrorCode
{
    None,
    InvalidSyntax,
    UnsupportedToken,
    TokenizationError,
    DuplicateLabel,
    DuplicateVariable,
    DuplicateConstant,
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