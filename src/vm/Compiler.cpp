#include "Compiler.h"
#include "utility/File.h"
#include "utility/String.h"
#include "Constants.h"
#include "VM.h"
#include <stdexcept>
#include <string>
#include <array>

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
    _tokens = tokens;
    _curTokenIndex = 0;
    std::vector<Instruction> instructions = {};
    std::vector<Label> labels = {}; //Used over std::map<> for insertion based ordering. Easier to debug.
    std::vector<Variable> variables = {};
    std::vector<VmConfig> config = {};

    //Instructions that need to be patched in step 2
    std::vector<Patch> labelPatches = {};
    std::vector<Patch> variablePatches = {};

    //Add built in constants
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
        Tokens are parsed and instructions are generated from them. Labels and variables recorded for step 2.
    */
    while (_curTokenIndex < tokens.size())
    {
        const TokenData cur = tokens[_curTokenIndex]; //Current token
        Instruction instruction = { 0 }; //Next instruction to generate. If `continue` or `break` are encountered the instruction gets discarded

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
            //Expect<N> returns a std::optional<std::array<TokenData, N>>. If the pattern isn't matched it'll return an empty and the if statement will fail.
            if (auto pattern = Expect<3>({ Token::Register, Token::Register, Token::Newline })) //op register register
            {
                //pattern.value() is a std::array<TokenData, 3> here. Structured binding is used to extract all 3 tokens in one line.
                auto [regA, regB, newline] = pattern.value();
                instruction.OpRegisterRegister.Opcode = (u16)(cur.Type);
                instruction.OpRegisterRegister.RegA = GetRegisterIndex(regA);
                instruction.OpRegisterRegister.RegB = GetRegisterIndex(regB);
            }
            else if (auto pattern = Expect<3>({ Token::Register, Token::Value, Token::Newline })) //op register value
            {
                auto [reg, value, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)(cur.Type) + 1; //The Token enum is equivalent to the Opcode enum for easy conversion. Can convert from Mov -> MovVal by adding 1.
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                instruction.OpRegisterValue.Value = String::ToShort(value.String);
            }
            else if (auto pattern = Expect<3>({ Token::Register, Token::VarName, Token::Newline })) //op register variable
            {
                auto [reg, var, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)(cur.Type) + 1;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                instruction.OpRegisterValue.Value = 0; //Patched in compile step 2
                variablePatches.push_back({ instructions.size(), var.String }); //Mark instruction for constant patching in step 2
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " register (register|value)'" });

            break;

            //Load instruction
        case Token::Load:
            if (auto pattern = Expect<3>({ Token::Register, Token::Value, Token::Newline })) //Load from constant address
            {
                auto [reg, value, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Load;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                instruction.OpRegisterValue.Value = String::ToShort(value.String);
            }
            else if (auto pattern = Expect<3>({ Token::Register, Token::VarName, Token::Newline })) //Load the value of a variable or from a constant address
            {
                auto [reg, var, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Load;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                instruction.OpRegisterValue.Value = 0; //Patched in compile step 2
                variablePatches.push_back({ instructions.size(), var.String });
            }
            else if (auto pattern = Expect<3>({ Token::Register, Token::Register, Token::Newline }))
            {
                auto [regA, regB, newline] = pattern.value();
                instruction.OpRegisterRegister.Opcode = (u16)Opcode::LoadP;
                instruction.OpRegisterRegister.RegA = GetRegisterIndex(regA);
                instruction.OpRegisterRegister.RegB = GetRegisterIndex(regB);
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid store syntax. Expects 'load register register' or 'load register address'" });

            break;

            //Store instruction
        case Token::Store:
            if (auto pattern = Expect<3>({ Token::Value, Token::Register, Token::Newline })) //Store register value in constant address
            {
                auto [value, reg, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Store;
                instruction.OpRegisterValue.Value = String::ToShort(value.String);
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
            }
            else if (auto pattern = Expect<3>({ Token::VarName, Token::Register, Token::Newline })) //Store register value in variable or from a constant address
            {
                auto [var, reg, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Store;
                instruction.OpRegisterValue.Value = 0; //Patched in compile step 2
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                variablePatches.push_back({ instructions.size(), var.String });
            }
            else if (auto pattern = Expect<3>({ Token::Register, Token::Register, Token::Newline }))
            {
                auto [regA, regB, newline] = pattern.value();
                instruction.OpRegisterRegister.Opcode = (u16)Opcode::StoreP;
                instruction.OpRegisterRegister.RegA = GetRegisterIndex(regA);
                instruction.OpRegisterRegister.RegB = GetRegisterIndex(regB);
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
            if (auto pattern = Expect<2>({ Token::Value, Token::Newline }))
            {
                auto [value, newline] = pattern.value();
                instruction.OpAddress.Opcode = (u16)cur.Type;
                instruction.OpAddress.Address = String::ToShort(value.String);
            }
            else if (auto pattern = Expect<2>({ Token::Label, Token::Newline }))
            {
                auto [label, newline] = pattern.value();
                instruction.OpAddress.Opcode = (u16)cur.Type;
                instruction.OpAddress.Address = 0; //Patched in compile step 2
                labelPatches.push_back({ instructions.size(), label.String });
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " address'" });

            break;

            //Return instruction
        case Token::Ret:
            if (auto pattern = Expect<1>({ Token::Newline }))
            {
                instruction.Op.Opcode = (u16)cur.Type;
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid 'ret' syntax. Expects no arguments." });

            break;

            //Instructions using 'op register' syntax
        case Token::Neg:
        case Token::Push:
        case Token::Pop:
            if (auto pattern = Expect<2>({ Token::Register, Token::Newline }))
            {
                auto [reg, newline] = pattern.value();
                instruction.OpRegister.Opcode = (u16)cur.Type;
                instruction.OpRegister.Reg = GetRegisterIndex(reg);
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. Expects '" + to_string(cur.Type) + " register'" });

            break;

            //Read from port
        case Token::Ipo:
            if (auto pattern = Expect<3>({ Token::Register, Token::VarName, Token::Newline })) //ipo register constant
            {
                auto [reg, var, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Ipo;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                instruction.OpRegisterValue.Value = 0; //Patched in stage 2
                variablePatches.push_back({ instructions.size(), var.String, true /*ConstantsOnly*/ });
            }
            else if (auto pattern = Expect<3>({ Token::Register, Token::Value, Token::Newline })) //ipo register value
            {
                auto [reg, value, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Ipo;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                instruction.OpRegisterValue.Value = String::ToShort(value.String);
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid ipo syntax. Expects `ipo register port`, where port is a port constant." });
            
            break;

            //Write to port
        case Token::Opo:
            if (auto pattern = Expect<3>({ Token::VarName, Token::Register, Token::Newline })) //opo port register
            {
                auto [var, reg, newline] = pattern.value();
                instruction.OpRegisterValue.Opcode = (u16)Opcode::Opo;
                instruction.OpRegisterValue.RegA = GetRegisterIndex(reg);
                instruction.OpRegisterValue.Value = 0; //Patched in stage 2
                variablePatches.push_back({ instructions.size(), var.String });
            }
            else if (auto pattern = Expect<3>({ Token::VarName, Token::Value, Token::Newline })) //opo port value
            {
                auto [var, value, newline] = pattern.value();
                instruction.OpPortValue.Opcode = (u16)Opcode::OpoVal;
                instruction.OpPortValue.Port = 0; //Patched in stage 2
                instruction.OpPortValue.Value = String::ToShort(value.String);
                variablePatches.push_back({ instructions.size(), var.String, true /*ConstantsOnly*/, true /*PatchPort*/ });
            }
            else if (auto pattern = Expect<3>({ Token::VarName, Token::VarName, Token::Newline })) //opo port constant
            {
                auto [var, port, newline] = pattern.value();
                instruction.OpPortValue.Opcode = (u16)Opcode::OpoVal;
                instruction.OpPortValue.Port = 0; //Patched in stage 2
                instruction.OpPortValue.Value = 0; //Patched in stage 2
                variablePatches.push_back({ instructions.size(), var.String, true /*ConstantsOnly*/, true /*PatchPort*/ });
                variablePatches.push_back({ instructions.size(), port.String, true /*ConstantsOnly*/, false /*PatchPort*/ });
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid opo syntax. Expects `opo port register|value|constant`, where port is a port constant." });

            break;

        case Token::Nop:
            if (auto pattern = Expect<1>({ Token::Newline }))
                instruction.Op.Opcode = (u32)Opcode::Nop;
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid nop syntax. Expects `nop` with no other arguments" });

            break;

            //Ignore blank lines
        case Token::Newline:
            _curTokenIndex++;
            continue;

        case Token::Var:
            if (auto pattern = Expect<3>({ Token::VarName, Token::Value, Token::Newline }))
            {
                auto [var, value, newline] = pattern.value();

                //Don't allow duplicate variables
                for (Variable& variable : variables)
                    if(variable.Name == var.String)
                        return Error(CompilerError{ CompilerErrorCode::DuplicateVariable, "Variable \"" + std::string(cur.String) + "\" duplicated!" });

                //Count variables to determine new variables relative address
                u32 variableCount = 0;
                for (Variable& variable : variables)
                    if (!variable.Constant)
                        variableCount++;

                //Add to variables list
                Variable variable;
                variable.Name = var.String;
                variable.Address = variableCount * sizeof(VmValue); //Address relative to start of variables block
                variable.InitialValue = String::ToShort(value.String);
                variable.Constant = false;
                variables.push_back(variable);
                _curTokenIndex++;
                continue; //Doesn't generate an instruction
            }
            break;

        case Token::Constant:
            if (auto pattern = Expect<3>({ Token::VarName, Token::Value, Token::Newline }))
            {
                auto [var, value, newline] = pattern.value();

                //Don't allow duplicate constants
                for (Variable& variable : variables)
                    if (variable.Name == var.String)
                        return Error(CompilerError{ CompilerErrorCode::DuplicateVariable, "Variable \"" + std::string(cur.String) + "\" duplicated!" });

                //Add to variables list
                Variable variable;
                variable.Name = var.String;
                variable.Address = -1; //Constants are compile time only
                variable.InitialValue = String::ToShort(value.String);
                variable.Constant = true;
                variables.push_back(variable);
                _curTokenIndex++;
                continue; //Doesn't generate an instruction
            }
            break;

        case Token::Config:
            if (auto pattern = Expect<3>({ Token::VarName, Token::Value, Token::Newline }))
            {
                auto [var, value, newline] = pattern.value();
                VmConfig& configVal = config.emplace_back();
                configVal.Name = String::ToLower(var.String); //Names are case insensitive
                configVal.Value = String::ToShort(value.String);
                _curTokenIndex++;
                continue; //Doesn't generate an instruction
            }
            break;

        case Token::Label:
            if (auto pattern = Expect<1>({ Token::Newline }))
            {
                //Don't allow duplicate labels
                for (Label& label : labels)
                    if (label.Name == cur.String)
                        return Error(CompilerError{ CompilerErrorCode::DuplicateVariable, "Variable \"" + std::string(cur.String) + "\" duplicated!" });

                //Map label name to its address
                labels.push_back(Label{ cur.String, VM::RESERVED_BYTES + instructions.size() * sizeof(Instruction) });
                _curTokenIndex++;
                continue; //Doesn't generate an instruction
            }
            else
                return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid label syntax. Expects `!labelName:` followed by a newline." });

            break;

            //These tokens shouldn't be standalone
        case Token::Register:
        case Token::VarName:
        case Token::Value:
            return Error(CompilerError{ CompilerErrorCode::InvalidSyntax, "Invalid syntax. '" + to_string(cur.Type) + "' should only be used as an instruction argument." });

        default:
            return Error(CompilerError{ CompilerErrorCode::UnsupportedToken, "Unknown token '" + std::string(cur.String) + "' passed to compiler." });
        }

        _curTokenIndex++; //Next instruction
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
        Step 3, Generate program binary:
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
    _tokens.clear();
    return Success(program);
}

Result<VmProgram, CompilerError> Compiler::Compile(std::string_view source)
{
    //Tokenize source
    Result<std::vector<TokenData>, TokenizerError> tokenizeResult = Tokenizer::Tokenize(source);
    if (tokenizeResult.Error())
        return Error(CompilerError{ CompilerErrorCode::TokenizationError, tokenizeResult.Error().value().Message });

    _tokens = tokenizeResult.Success().value();
    Result<VmProgram, CompilerError> compileResult = Compile(tokenizeResult.Success().value());
    _tokens.clear();
    return compileResult;
}

Result<VmProgram, CompilerError> Compiler::CompileFile(std::string_view inputFilePath)
{
    //Read file to string, tokenize, and compile
    return Compile(File::ReadAll(inputFilePath) + "\r\n");
}

i32 Compiler::GetRegisterIndex(const TokenData& token)
{
    if (token.String == "r0")
        return 0;
    if (token.String == "r1")
        return 1;
    if (token.String == "r2")
        return 2;
    if (token.String == "r3")
        return 3;
    if (token.String == "r4")
        return 4;
    if (token.String == "r5")
        return 5;
    if (token.String == "r6")
        return 6;
    if (token.String == "r7")
        return 7;
    
    throw std::runtime_error("Invalid register token enum '" + std::string(token.String) + "' passed to Compiler::GetRegisterIndex()");
}

template<size_t numTokens>
std::optional<std::array<TokenData, numTokens>> Compiler::Expect(std::initializer_list<Token> pattern)
{
    //Make sure we won't peek out of bounds
    assert(pattern.size() == numTokens, "Size mismatch with Compiler::Peek<numTokens>(pattern). pattern isn't the same size as numTokens. They must be the same");
    size_t peekMax = _curTokenIndex + numTokens;
    if (peekMax >= _tokens.size())
        return {};

    std::array<TokenData, numTokens> out;
    size_t i = 0;

    //Check if the next N tokens match the pattern
    for (Token token : pattern)
    {
        size_t peekIndex = _curTokenIndex + i + 1;
        if (peekIndex >= _tokens.size() || _tokens[peekIndex].Type != token)
            return {}; //Pattern mismatch, return empty
        else
            out[i] = _tokens[peekIndex];

        i++;
    }

    //Pattern matched. Return token data and advance token stream
    _curTokenIndex = peekMax;
    return out;
}