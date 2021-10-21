#include "Compiler.h"

Result<Span<u8>, CompilerError> Compiler::CompileToMemory(std::string_view inputFilePath)
{
    Span<u8> span = { nullptr, 0 };
    return Success(span);
}

Result<Span<u8>, CompilerError> Compiler::CompileToMemory(const std::vector<TokenData> tokens)
{
    Span<u8> span = { nullptr, 0 };
    return Success(span);
}

Result<void, CompilerError> Compiler::CompileToFile(std::string_view inputFilePath, std::string_view outputFilePath)
{
    return Success<void>();
}

Result<void, CompilerError> Compiler::CompileToFile(const std::vector<TokenData> tokens, std::string_view outputFilePath)
{
    return Success<void>();
}
