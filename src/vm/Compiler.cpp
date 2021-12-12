#include "Compiler.h"
#include "utility/File.h"
#include "utility/String.h"
#include "Constants.h"
#include "VM.h"
#include <stdexcept>
#include <string>

struct Label
{
    std::string_view Name;
    size_t Address;
};
struct Variable
{
    std::string_view Name;
    VmValue Address; //Address relative to the start of the variable block of the program
    VmValue InitialValue; //Initial value set at compile time
    bool Constant;
};
struct Patch //Info about an instruction that needs to be patched in compile step 2
{
    size_t Index; //Index of the instruction to patch
    std::string_view Name; //Name of the label, variable, or constant that needs to be patched in
    bool ConstantsOnly = false; //If true, variable patches will only work for constant variables. Doesn't do anything for label patches.
    //Todo: Come up with a better way of handling this + OpoVal encoding. All the special cases it causes makes the code messy.
    bool PatchPort = false; //Special variable used for OpoVal. If true the port set, if false the value is set
};

Result<VmProgram, CompilerError> Compiler::Compile(const std::vector<TokenData>& tokens)
{
    /*
        Compilation steps:
            1) Parse tokens: parses all tokens and generates instructions from them when.
            2) Patch addresses: replace variables and labels with their addresses.
            3) Write program binary: generate the program binary that the VM can run.
    */
    //Index of the current token the compiler is at
    u32 curTokenIndex = 0;
    //Instructions generated from tokens
    std::vector<Instruction> instructions = {};

    //Data collected during step 1 and used in step 2
    std::vector<Label> labels = {}; //Used over std::map<> for insertion based ordering. Easier to debug.
    std::vector<Variable> variables = {};

    //Instructions that need to be patched in step 2
    std::vector<Patch> labelPatches = {};
    std::vector<Patch> variablePatches = {};

    //Config values
    std::vector<VmConfig> config = {};

    //Peek ahead in the token list. Returns Token::NONE if curTokenIndex + distance is out of bounds
    auto PeekToken = [&](u32 distance) -> TokenData { return (curTokenIndex + distance < tokens.size()) ? tokens[curTokenIndex + distance] : TokenData{ Token::None, "" }; };

    //Add built in constants to list
    for (auto& kv : BuiltInConstants)
    {
        std::string_view name = kv.first;
        VmValue address = kv.second;
        Variable& var = variables.emplace_back();
        var.Name = name;
        var.Address = 0;
        var.InitialValue = address;
        var.Constant = true;
    }

    /*
        Step 1, Parse tokens:
        Tokens are parsed and instructions are generated from them. Label and variable usage is recorded for step 2.
    */
    while (curTokenIndex < tokens.size())
    {
        Instruction instruction = { 0 };

        //Get token data. PeekToken() returns Token::None if it goes out of bounds.
        const TokenData cur = tokens[curTokenIndex];
        const TokenData next = PeekToken(1);
        const TokenData next2 = PeekToken(2);
        const TokenData next3 = PeekToken(3);
        const TokenData next4 = PeekToken(4);

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
                instruction.OpRegisterRegister.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterRegister.RegB = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else if (next.IsRegister() && next2.Type == Token::Value && next3.Type == Token::Newline) //op register value
            {
                //The Token enum is equivalent to the Opcode enum for easy conversion
                //Opcodes that have register and value variants can be converted by adding 1. E.g. Mov = 0, MovVal = Mov + 1
                instruction.OpRegisterValue.Opcode = (u16)(cur.Type) + 1;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = String::ToShort(next2.String);
                curTokenIndex += 4;
            }
            else if (next.IsRegister() && next2.Type == Token::VarName && next3.Type == Token::Newline) //op register constant
            {
                instruction.OpRegisterValue.Opcode = (u16)(cur.Type) + 1;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = 0; //Patched in compile step 2
                curTokenIndex += 4;

                //Mark instruction for constant patching in step 2
                variablePatches.push_back({ instructions.size(), next2.String });
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " register (register|value)'" });

            break;

            //Load instruction
        case Token::Load:
            if (next.IsRegister() && next2.Type == Token::Value && next3.Type == Token::Newline) //Load from constant address
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Load;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = String::ToShort(next2.String);
                curTokenIndex += 4;
            }
            else if (next.IsRegister() && next2.Type == Token::VarName && next3.Type == Token::Newline) //Load the value of a variable or from a constant address
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Load;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = 0; //Patched in compile step 2
                curTokenIndex += 4;

                //Mark instruction for variable address patching
                variablePatches.push_back({ instructions.size(), next2.String });
            }
            else if (next.IsRegister() && next2.IsRegister() && next3.Type == Token::Newline)
            {
                instruction.OpRegisterRegister.Opcode = (u16)Opcode::LoadP;
                instruction.OpRegisterRegister.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterRegister.RegB = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid store syntax. Expects 'load register register' or 'load register address'" });

            break;

            //Store instruction
        case Token::Store:
            if (next.Type == Token::Value && next2.IsRegister() && next3.Type == Token::Newline) //Store register value in constant address
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Store;
                instruction.OpRegisterValue.Value = String::ToShort(next.String);
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else if (next.Type == Token::VarName && next2.IsRegister() && next3.Type == Token::Newline) //Store register value in variable or from a constant address
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Store;
                instruction.OpRegisterValue.Value = 0; //Patched in compile step 2
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;

                //Mark instruction for variable address patching
                variablePatches.push_back({ instructions.size(), next.String });
            }
            else if (next.IsRegister() && next2.IsRegister() && next3.Type == Token::Newline)
            {
                instruction.OpRegisterRegister.Opcode = (u16)Opcode::StoreP;
                instruction.OpRegisterRegister.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterRegister.RegB = GetRegisterIndex(next2.Type);
                curTokenIndex += 4;
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid store syntax. Expects 'store register register' or 'store address register'" });

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
            else if (next.Type == Token::Label && next2.Type == Token::Newline)
            {
                instruction.OpAddress.Opcode = (u16)cur.Type;
                instruction.OpAddress.Address = 0; //Patched in compile step 2
                curTokenIndex += 3;

                //Mark instruction for label address patching
                labelPatches.push_back({ instructions.size(), next.String });
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " address'" });

            break;

            //Return instruction
        case Token::Ret:
            if (next.Type == Token::Newline)
            {
                instruction.Op.Opcode = (u16)cur.Type;
                curTokenIndex += 2;
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid 'ret' syntax. Expects no arguments." });

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
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " register'" });

            break;

            //Read from port
        case Token::Ipo:
            if (next.IsRegister() && next2.Type == Token::VarName && next3.Type == Token::Newline) //ipo register constant
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Ipo;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = 0; //Patched in stage 2
                curTokenIndex += 4;

                //Mark instruction for variable address patching
                variablePatches.push_back({ instructions.size(), next2.String, true /*ConstantsOnly*/ });
            }
            else if (next.IsRegister() && next2.Type == Token::Value && next3.Type == Token::Newline) //ipo register value
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Ipo;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next.Type);
                instruction.OpRegisterValue.Value = String::ToShort(next2.String);
                curTokenIndex += 4;
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid ipo syntax. Expects `ipo register port`, where port is a port constant." });

            break;

            //Write to port
        case Token::Opo:
            if (next.Type == Token::VarName && next2.IsRegister() && next3.Type == Token::Newline) //opo port register
            {
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Opo;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(next2.Type);
                instruction.OpRegisterValue.Value = 0; //Patched in stage 2
                curTokenIndex += 4;

                //Mark instruction for variable address patching
                variablePatches.push_back({ instructions.size(), next.String });
            }
            else if (next.Type == Token::VarName && next2.Type == Token::Value && next3.Type == Token::Newline) //opo port value
            {
                instruction.OpPortValue.Opcode = (u16)Opcode::OpoVal;
                instruction.OpPortValue.Port = 0; //Patched in stage 2
                instruction.OpPortValue.Value = String::ToShort(next2.String);
                curTokenIndex += 4;
                
                variablePatches.push_back({ instructions.size(), next.String, true /*ConstantsOnly*/, true /*PatchPort*/ });
            }
            else if (next.Type == Token::VarName && next2.Type == Token::VarName && next3.Type == Token::Newline) //opo port constant
            {
                instruction.OpPortValue.Opcode = (u16)Opcode::OpoVal;
                instruction.OpPortValue.Port = 0; //Patched in stage 2
                instruction.OpPortValue.Value = 0; //Patched in stage 2
                curTokenIndex += 4;

                variablePatches.push_back({ instructions.size(), next.String, true /*ConstantsOnly*/, true /*PatchPort*/ });
                variablePatches.push_back({ instructions.size(), next2.String, true /*ConstantsOnly*/, false /*PatchPort*/ });
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid opo syntax. Expects `opo port register|value|constant`, where port is a port constant." });

            break;

            //Ignore blank lines
        case Token::Newline:
            curTokenIndex += 1;
            continue;

        case Token::Var:
            if (next.Type == Token::VarName && next2.Type == Token::Value && next3.Type == Token::Newline)
            {
                //Don't allow duplicate variables
                for (Variable& var : variables)
                    if(var.Name == next.String)
                        return Error(CompilerError{ CompilerErrorCode::DuplicateVariable, "Variable \"" + std::string(cur.String) + "\" duplicated!" });

                //Count variables to determine new variables relative address
                u32 variableCount = 0;
                for (Variable& var : variables)
                    if (!var.Constant)
                        variableCount++;

                //Add to variables list
                Variable var;
                var.Name = next.String;
                var.Address = variableCount * sizeof(VmValue); //Address relative to start of variables block
                var.InitialValue = String::ToShort(next2.String);
                var.Constant = false;
                variables.push_back(var);
                curTokenIndex += 4;
                continue; //Doesn't generate an instruction
            }
            break;

        case Token::Constant:
            if (next.Type == Token::VarName && next2.Type == Token::Value && next3.Type == Token::Newline)
            {
                //Don't allow duplicate constants
                for (Variable& var : variables)
                    if (var.Name == next.String)
                        return Error(CompilerError{ CompilerErrorCode::DuplicateVariable, "Variable \"" + std::string(cur.String) + "\" duplicated!" });

                //Add to variables list
                Variable var;
                var.Name = next.String;
                var.Address = -1; //Constants are compile time only
                var.InitialValue = String::ToShort(next2.String);
                var.Constant = true;
                variables.push_back(var);
                curTokenIndex += 4;
                continue; //Doesn't generate an instruction
            }
            break;

        case Token::Config:
            if (next.Type == Token::VarName && next2.Type == Token::Value && next3.Type == Token::Newline)
            {
                VmConfig& configVal = config.emplace_back();
                configVal.Name = String::ToLower(next.String); //Names are case insensitive
                configVal.Value = String::ToShort(next2.String);
                curTokenIndex += 4;
            }
            break;

        case Token::Label:
            if (next.Type == Token::Newline)
            {
                //Don't allow duplicate labels
                for (Label& label : labels)
                    if (label.Name == next.String)
                        return Error(CompilerError{ CompilerErrorCode::DuplicateVariable, "Variable \"" + std::string(cur.String) + "\" duplicated!" });

                //Map label name to its address
                labels.push_back(Label{ cur.String, VM::RESERVED_BYTES + instructions.size() * sizeof(Instruction) });
                curTokenIndex += 2;
                continue; //Doesn't generate an instruction
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid label syntax. Expects `!labelName:` followed by a newline." });

            break;

            //These tokens shouldn't be standalone
        case Token::Register0:
        case Token::Register1:
        case Token::Register2:
        case Token::Register3:
        case Token::Register4:
        case Token::Register5:
        case Token::Register6:
        case Token::Register7:
        case Token::VarName:
        case Token::Value:
            return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. '" + to_string(cur.Type) + "' should only be used as an instruction argument." });

        default:
            return Error(CompilerError{ CompilerErrorCode::UnsupportedToken, "Unknown token '" + std::string(cur.String) + "' passed to compiler." });
        }

        instructions.push_back(instruction);
    }


    /*
        Step 2, Patch variables and labels:
        Labels and variables are replaced with their addresses. Done after parsing since their addresses aren't known until all tokens are parsed.
    */
    //Patch label addresses
    for (Patch& patch : labelPatches)
        for (Label& label : labels)
            if (label.Name == patch.Name)
                instructions[patch.Index].OpAddress.Address = label.Address;

    //Offset of variable block in VM memory
    VmValue variableBlockOffset = VM::RESERVED_BYTES + (instructions.size() * sizeof(Instruction));

    //Patch variables and constants
    for (Patch& patch : variablePatches)
        for (Variable& variable : variables)
            if (variable.Name.compare(patch.Name) == 0) //Using ::compare to compare string_views by character
            {
                //Some opcodes only allow constant variables
                if (patch.ConstantsOnly && !variable.Constant)
                    return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Variable used as an argument in opcode that only accepts constants. Opcode: " + to_string((Opcode)instructions[patch.Index].Op.Opcode) });

                Opcode opcode = (Opcode)instructions[patch.Index].Op.Opcode;
                if (variable.Constant) //Patch constant value
                {
                    if (opcode == Opcode::OpoVal) //Special case since OpoVal uses different variable encoding than other instructions
                    {
                        if (patch.PatchPort)
                            instructions[patch.Index].OpPortValue.Port = variable.InitialValue;
                        else
                            instructions[patch.Index].OpPortValue.Value = variable.InitialValue;
                    }
                    else
                        instructions[patch.Index].OpRegisterValue.Value = variable.InitialValue;
                }
                else //Patch variable address
                    instructions[patch.Index].OpRegisterValue.Value = variableBlockOffset + variable.Address;
            }


    /*
        Step 2, Generate program binary:
        Generate program binary. It's data is laid out as such:
            - Header: Contains info such as the program size, and the sizes of the instruction and variable blocks.
            - Instructions: Instructions to be executed by the VM.
            - Variables: Space for variables set to their default values. Done at compile time so the stack, which grows
                         from the end of VM memory down, is only constrained in size by the number of variables.
    */
    //Write variables to vector set to their initial values
    std::vector<VmValue> finalVariables = {};
    for (Variable& variable : variables)
    {
        if (variable.Constant)
            continue; //Constants are discarded after step 2

        finalVariables.push_back(variable.InitialValue);
    }

    //Calculate size of each data block
    u32 instructionsSizeBytes = instructions.size() * sizeof(Instruction);
    u32 variablesSizeBytes = finalVariables.size() * sizeof(VmValue);
    u32 programSizeBytes = sizeof(ProgramHeader) + instructionsSizeBytes + variablesSizeBytes;

    //Write header
    ProgramHeader header;
    header.Signature = VmProgram::EXPECTED_SIGNATURE; //ASCII string "ATRB"
    header.ProgramSize = programSizeBytes;
    header.InstructionsSize = instructions.size() * sizeof(Instruction);
    header.VariablesSize = variablesSizeBytes;

    //Construct and return vm program instance
    VmProgram program(std::move(header), std::move(instructions), std::move(finalVariables), std::move(config));
    return Success(program);
}

Result<VmProgram, CompilerError> Compiler::Compile(std::string_view source)
{
    //Tokenize source
    Result<std::vector<TokenData>, TokenizerError> tokenizeResult = Tokenizer::Tokenize(source);
    if (tokenizeResult.Error())
        return Error(CompilerError{ CompilerErrorCode::TokenizationError, tokenizeResult.Error().value().Message });

    return Compile(tokenizeResult.Success().value());
}

Result<VmProgram, CompilerError> Compiler::CompileFile(std::string_view inputFilePath)
{
    //Read file to string, tokenize, and compile
    return Compile(File::ReadAll(inputFilePath));
}

i32 Compiler::GetRegisterIndex(Token token)
{
    if (token < Token::Register0 || token > Token::Register7)
        throw std::runtime_error("Invalid register token enum '" + to_string(token) + "' passed to Compiler::GetRegisterIndex()");

    return (u16)token - (u16)Token::Register0;
}