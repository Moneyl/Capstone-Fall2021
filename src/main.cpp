#include <iostream>
#include "Application.h"
#include "vm/Tokenizer.h"
#include "vm/Compiler.h"
#include "vm/VM.h"

//Todo: Move to test file
const std::string testString =
//"jmp 2\n"
//"var cnt 26\n" //Todo: support var
"load r0 1\n"
//";mov r1 'a'\n" //Todo: Support character values
"store 0xFF r1\n"
"add r1 1\n"
"sub r0 1\n"
"cmp r0 0\n"
//"jgr 4\n"
"mov r3 0x0D\n"
"mov r4 0x0A\n"
"push r3\n"
"push r4\n"
"pop r6\n"
"pop r5\n"
"store 0xFF r5\n"
"store 0xFF r6\n"
"load r7 0xFE\n"
"add r7 3\n"
"store 0xFF r7\n"
"ret\n";

int main(int argc, char* argv[])
{
    //Tokenize code
    std::vector<TokenData> tokens = Tokenizer::Tokenize(testString);
    printf("\nTokens:");
    for (TokenData& token : tokens)
        std::cout << "Token: " << to_string(token.Type) << ", String: \"" << token.String << "\"\n";

    //Compile tokens to instructions
    Compiler compiler;
    Result<std::vector<Instruction>, CompilerError> compileResult = compiler.CompileToMemory(tokens);
    if (compileResult.Error())
    {
        CompilerError error = compileResult.ErrorData();
        std::cout << "Compiler error '" << to_string(error.Code) << "'. Message: \"" << error.Message << "\"\n";
        return EXIT_FAILURE;
    }

    //Disassemble binary
    printf("\nProgram binary:");
    for (Instruction& instruction : compileResult.SuccessData())
        std::cout << to_string(instruction) << "\n";

    //Run VM
    VM* vm = new VM();
    vm->Run(compileResult.SuccessData());

    //Print VM state
    printf("\nVM State:");
    printf("Registers:\n");
    for (u32 i = 0; i < VM::NUM_REGISTERS; i++)
        printf("\tr%d = %d\n", i, vm->Registers[i]);

    delete vm;

    Application app;
    bool result = app.Run();
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
