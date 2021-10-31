#include <iostream>
#include "Application.h"
#include "vm/Tokenizer.h"
#include "vm/Compiler.h"
#include "vm/VM.h"


//Todo: Move to test file
//Program that tests out several instructions, the stack, labels, and variables
const std::string testString =
"jmp !label1\n"

"!label0\n"
"sub r0 88\n"
"jmp !label2\n"
"add r1 99\n"

"!label1\n"
"add r2 10\n"
"jmp !label0\n"
"add r3 21\n"

"!label2\n"
"add r4 67\n"
"add r5 100\n"
"store a r5\n"
"load r6 a\n"
"\n"
"store x_1234 r0\n"
"load r7 x_1234\n"
"\n"
"load r1 _my_var10\n"

"push r0\n"
"pop r3\n"

"var a 2\n" //Defining these at the end to ensure they work properly. It's not really recommended for clean code
"var x_1234 -99\n"
"var _my_var10 15000\n"
"\n"
"\n";

//Expected result:
//  r0: -88
//  r1: 15000
//  r2: 10
//  r3: -88
//  r4: 67
//  r5: 100
//  r6: 100
//  r7: -88

int main(int argc, char* argv[])
{
    //Tokenize code
    Result<std::vector<TokenData>, TokenizerError> tokenizeResult = Tokenizer::Tokenize(testString);
    if (tokenizeResult.Error())
    {
        TokenizerError error = tokenizeResult.ErrorData();
        std::cout << "Tokenizer error '" << to_string(error.Code) << "'. Message: \"" << error.Message << "\"\n";
        return EXIT_FAILURE;
    }

    //Print out tokens
    const std::vector<TokenData>& tokens = tokenizeResult.SuccessData();
    printf("\nTokens:");
    for (const TokenData& token : tokens)
        std::cout << "Token: " << to_string(token.Type) << ", String: \"" << token.String << "\"\n";

    //Compile tokens to instructions
    Compiler compiler;
    Result<VmProgram, CompilerError> compileResult = compiler.Compile(tokens);
    if (compileResult.Error())
    {
        CompilerError error = compileResult.ErrorData();
        std::cout << "Compiler error '" << to_string(error.Code) << "'. Message: \"" << error.Message << "\"\n";
        return EXIT_FAILURE;
    }

    //Disassemble binary
    printf("\nProgram binary:");
    for (const Instruction& instruction : compileResult.SuccessData().Instructions)
        std::cout << to_string(instruction) << "\n";

    //Create VM and load test program into it
    VM* vm = new VM();
    Result<void, VMError> programLoadResult = vm->LoadProgram(compileResult.SuccessData());
    if (programLoadResult.Error())
    {
        VMError error = programLoadResult.ErrorData();
        std::cout << "Program load error '" << to_string(error.Code) << "'. Message: \"" << error.Message << "\"\n";
        return EXIT_FAILURE;
    }

    //Execute whole program
    u32 cycle = 0;
    while (vm->PC < vm->InstructionsSize())
    {
        Result<void, VMError> cycleResult = vm->Cycle();
        if (cycleResult.Error())
        {
            VMError& error = cycleResult.ErrorData();
            std::cout << "VM error on cycle " + std::to_string(cycle) + ": '" << to_string(error.Code) << "'. Message: \"" << error.Message << "\"\n";
            break;
        }
        cycle++;
    }

    //Print VM state
    printf("\nVM State:\n");
    printf("Sign Flag: %s\n", vm->FlagSign ? "true" : "false");
    printf("Zero Flag: %s\n", vm->FlagZero ? "true" : "false");
    printf("Registers:\n");
    for (u32 i = 0; i < VM::NUM_REGISTERS; i++)
        printf("\tr%d = %d\n", i, vm->Registers[i]);

    printf("\tPC = %d\n", vm->PC);
    printf("\tSP = %d\n", vm->SP);

    delete vm;

    Application app;
    bool result = app.Run();
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
