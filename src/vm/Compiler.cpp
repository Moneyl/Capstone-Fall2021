#include "Compiler.h"
#include "utility/File.h"

Result<std::vector<Instruction>, CompilerError> Compiler::CompileToMemory(const std::vector<TokenData> tokens)
{
    //Parse tokens and output machine code
    std::vector<Instruction> out;
    u32 pos = 0;
    while (pos < out.size())
    {
        Instruction* cur = &out[0];
        Instruction* next = &out[pos + 1];


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