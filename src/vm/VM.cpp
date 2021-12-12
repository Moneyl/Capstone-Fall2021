#include "VM.h"
#include "Compiler.h"
#include <stdexcept>

/*Common error checks used while executing instructions*/
//Prevent divide by zero
#define DIVIDE_BY_ZERO_CHECK() if (value == Registers[regB])\
{return Error(VMError{ VMErrorCode::DivideByZero, "Divide by zero attempt by instruction at address " + std::to_string(lastPC) });}

//Prevent accessing data outside of VM memory
#define OUT_OF_BOUNDS_MEMORY_CHECK() if (address >= VM::MEMORY_SIZE)\
{return Error(VMError{ VMErrorCode::OutOfBoundsMemoryAccess, "Out of bounds memory access by instruction at address " + std::to_string(lastPC) + ". Instruction.Address = " + std::to_string(address) });}

//Prevent stack from growing into variable/program memory
#define STACK_OVERFLOW_CHECK() if (SP <= VM::RESERVED_BYTES + InstructionsSize() + VariablesSize())\
{return Error(VMError{ VMErrorCode::StackOverflow, "Stack underflow by instruction at address " + std::to_string(lastPC) + ". SP = " + std::to_string(SP) });}

//Prevent stack from shrinking past the end of VM memory
#define STACK_UNDERFLOW_CHECK() if (SP >= VM::MEMORY_SIZE)\
{return Error(VMError{ VMErrorCode::StackUnderflow, "Stack underflow by instruction at address " + std::to_string(lastPC) }); }

Result<void, VMError> VM::LoadProgram(const VmProgram& program)
{
    //Zero out memory
    memset(Memory, 0, VM::MEMORY_SIZE);

    //Copy instructions into memory
    _instructionsSizeBytes = program.Header.InstructionsSize;
    memcpy(Memory + VM::RESERVED_BYTES, program.Instructions.data(), program.Header.InstructionsSize);

    //Copy variables into memory
    _variablesSizeBytes = program.Header.VariablesSize;
    memcpy(Memory + VM::RESERVED_BYTES + _instructionsSizeBytes, program.Variables.data(), program.Header.VariablesSize);

    //Copy misc data from program
    Config = program.Config;

    //Reset flags and registers
    FlagSign = false;
    FlagZero = false;
    PC = 0;
    SP = VM::MEMORY_SIZE;
    for (u32 i = 0; i < VM::NUM_REGISTERS; i++)
        Registers[i] = 0;

    return Success<void>();
}

Result<void, VMError> VM::LoadProgram(std::string_view inFilePath)
{
    Result<VmProgram, std::string> program = VmProgram::Read(inFilePath);
    if (program.Error())
        return Error(VMError{ VMErrorCode::ProgramFileLoadFailure, program.Error().value() });

    return LoadProgram(program.Success().value());
}

Result<void, VMError> VM::LoadProgramFromSource(std::string_view inFilePath)
{
    //Compile source code to VM program
    Compiler compiler;
    Result<VmProgram, CompilerError> compileResult = compiler.CompileFile(inFilePath);
    if (compileResult.Error())
    {
        CompilerError error = compileResult.Error().value();
        return Error(VMError{ VMErrorCode::ProgramFileLoadFailure, error.Message });
    }

    return LoadProgram(compileResult.Success().value());
}

Result<void, VMError> VM::Cycle(f32 deltaTime)
{
    if (_instruction) //Continue executing instruction
    {
        if (_instructionCyclesRemaining <= 1) //All cycles elapsed, execute instruction
        {
            Result<void, VMError> result = Execute(deltaTime);
            _instruction = nullptr;
            _instructionCyclesRemaining = 0;
            return result;
        }
        else //More cycles, decrement and continue
        {
            _instructionCyclesRemaining--;
        }
    }
    else //Fetch next instruction
    {
        //Reset PC to first instruction if it's out of bounds
        if (PC < VM::RESERVED_BYTES || PC >= VM::RESERVED_BYTES + InstructionsSize())
            PC = VM::RESERVED_BYTES;

        //Fetch next instruction
        _instruction = (Instruction*)(&Memory[PC]);

        //Get number of cycles the instruction takes to execute
        _instructionCyclesRemaining = GetInstructionDuration(*_instruction);
        if (_instructionCyclesRemaining == 0xFFFFFFFF)
            return Error(VMError{ VMErrorCode::UnsupportedInstruction, "Failed to get instruction duration in VM. Opcode: " + std::to_string((u32)_instruction->Op.Opcode) });

        if (_instructionCyclesRemaining <= 1) //Execute the instruction now if it takes 1 cycle
        {
            Result<void, VMError> result = Execute(deltaTime);
            _instruction = nullptr;
            _instructionCyclesRemaining = 0;
            return result;
        }
        else //Otherwise decrement the cycle counter
        {
            _instructionCyclesRemaining--;
        }
    }

    return Success<void>();
}

//Decodes and executes _instruction
//_instruction is fetched by VM::Cycle()
Result<void, VMError> VM::Execute(f32 deltaTime)
{
    u32 lastPC = PC; //Store before incrementing for error messages
    PC += sizeof(Instruction);

    //Decode
    Opcode opcode = (Opcode)_instruction->Op.Opcode;
    //Only some of these are valid depending on the opcode. See vm/Instructions.h for info on the data used by each instruction.
    u16 regA = _instruction->OpRegisterRegister.RegA;
    u16 regB = _instruction->OpRegisterRegister.RegB;
    i16 value = _instruction->OpRegisterValue.Value;
    u16 address = _instruction->OpAddress.Address;

    //Execute
    switch (opcode)
    {
    case Opcode::Mov:
        Registers[regA] = Registers[regB];
        break;
    case Opcode::MovVal:
        Registers[regA] = value;
        break;
    case Opcode::Add:
        Registers[regA] += Registers[regB];
        SetFlags(Registers[regA]);
        break;
    case Opcode::AddVal:
        Registers[regA] += value;
        SetFlags(Registers[regA]);
        break;
    case Opcode::Sub:
        Registers[regA] -= Registers[regB];
        SetFlags(Registers[regA]);
        break;
    case Opcode::SubVal:
        Registers[regA] -= value;
        SetFlags(Registers[regA]);
        break;
    case Opcode::Mul:
        Registers[regA] *= Registers[regB];
        SetFlags(Registers[regA]);
        break;
    case Opcode::MulVal:
        Registers[regA] *= value;
        SetFlags(Registers[regA]);
        break;
    case Opcode::Div:
        if (value == Registers[regB])
            return Error(VMError{ VMErrorCode::DivideByZero, "Divide by zero attempt by instruction at address " + std::to_string(lastPC) });

        Registers[regA] /= Registers[regB];
        SetFlags(Registers[regA]);
        break;
    case Opcode::DivVal:
        if (value == 0)
            return Error(VMError{ VMErrorCode::DivideByZero, "Divide by zero attempt by instruction at address " + std::to_string(lastPC) });

        Registers[regA] /= value;
        SetFlags(Registers[regA]);
        break;
    case Opcode::Cmp:
        SetFlags(Registers[regA] - Registers[regB]); //Update flags with difference
        break;
    case Opcode::CmpVal:
        SetFlags(Registers[regA] - value); //Update flags with difference
        break;
    case Opcode::Jmp:
        if (address >= VM::MEMORY_SIZE)
            return Error(VMError{ VMErrorCode::OutOfBoundsMemoryAccess, "Out of bounds memory access by instruction at address " + std::to_string(lastPC) + ". Instruction.Address = " + std::to_string(address) });

        PC = address; //Set next instruction to be executed
        break;
    case Opcode::Jeq:
        if (FlagZero)
            PC = address;
        break;
    case Opcode::Jne:
        if (!FlagZero)
            PC = address;
        break;
    case Opcode::Jgr:
        if (!FlagZero && !FlagSign)
            PC = address;
        break;
    case Opcode::Jls:
        if (FlagSign)
            PC = address;
        break;
    case Opcode::Call:
        STACK_OVERFLOW_CHECK();
        OUT_OF_BOUNDS_MEMORY_CHECK();
        //Push PC onto the stack and set it to the new address
        Push(PC);
        PC = address;
        break;
    case Opcode::Ret:
        STACK_UNDERFLOW_CHECK();
        OUT_OF_BOUNDS_MEMORY_CHECK();
        //Pop old PC value off the stack
        PC = Pop();
        break;
    case Opcode::And:
        Registers[regA] &= Registers[regB];
        break;
    case Opcode::AndVal:
        Registers[regA] &= value;
        break;
    case Opcode::Or:
        Registers[regA] |= Registers[regB];
        break;
    case Opcode::OrVal:
        Registers[regA] |= value;
        break;
    case Opcode::Xor:
        Registers[regA] ^= Registers[regB];
        break;
    case Opcode::XorVal:
        Registers[regA] ^= value;
        break;
    case Opcode::Neg:
        Registers[regA] *= -1;
        break;
    case Opcode::Load:
        OUT_OF_BOUNDS_MEMORY_CHECK();
        Registers[regA] = Load(address);
        break;
    case Opcode::LoadP:
        OUT_OF_BOUNDS_MEMORY_CHECK();
        Registers[regA] = Load(Registers[regB]);
        break;
    case Opcode::Store:
        OUT_OF_BOUNDS_MEMORY_CHECK();
        Store(address, Registers[regA]);
        break;
    case Opcode::StoreP:
        OUT_OF_BOUNDS_MEMORY_CHECK();
        Store(Registers[regA], Registers[regB]);
        break;
    case Opcode::Push:
        STACK_OVERFLOW_CHECK();
        //Push the value of regA onto the stack
        Push(Registers[regA]);
        break;
    case Opcode::Pop:
        STACK_UNDERFLOW_CHECK();
        //Pop a value off the stack and store it in register A
        Registers[regA] = Pop();
        break;
    case Opcode::Ipo:
        OnPortRead((Port)value, deltaTime);
        Registers[regA] = GetPort((Port)value); //Value contains the port index. Set via the built in port constants
        break;
    case Opcode::Opo:
        OnPortWrite((Port)value, Registers[regA], deltaTime); //Write the value of registerA to the port
        break;
    case Opcode::OpoVal:
        OnPortWrite((Port)_instruction->OpPortValue.Port, _instruction->OpPortValue.Value, deltaTime); //Write value to the port. The callback is allowed to discard the value.
        break;
    default:
        return Error(VMError{ VMErrorCode::UnsupportedInstruction, "Unsupported opcode '" + std::to_string((u32)_instruction->Op.Opcode) + "' decoded by VM." });
    }

    return Success<void>();
}


VmValue VM::Load(VmValue address)
{
    if (address >= VM::MEMORY_SIZE) //Caller is required to do their own bounds checking. Overkill to use Result<VmValue, VmError> for this func.
        throw std::runtime_error("Out of bounds address passed to VM::Load().");

    return *(VmValue*)(&Memory[address]);
}

void VM::Store(VmValue address, VmValue value)
{
    if (address >= VM::MEMORY_SIZE) //Caller is required to do their own bounds checking. Overkill to use Result<VmValue, VmError> for this func.
        throw std::runtime_error("Out of bounds address passed to VM::Store().");

    *(VmValue*)(&Memory[address]) = value;
}

void VM::Push(VmValue value)
{
    if (SP <= VM::RESERVED_BYTES + InstructionsSize() + VariablesSize()) //Prevent stack from growing into variable/program memory
        throw std::runtime_error("Stack overflow caused by VM::Push() call. SP = " + std::to_string(PC));

    //Grow stack down into memory and push a value onto it
    SP -= INSTRUCTION_NUM_VALUE_BYTES;
    Store(SP, value);
}

VmValue VM::Pop()
{
    if (SP >= VM::MEMORY_SIZE)
        throw std::runtime_error("Stack underflow caused by VM::Pop() call. SP = " + std::to_string(PC));

    //Pop a value off the top of stack and shrink it up towards the end of memory
    VmValue value = Load(SP);
    SP += INSTRUCTION_NUM_VALUE_BYTES;
    return value;
}

void VM::SetFlags(VmValue result)
{
    FlagZero = (result == 0);
    FlagSign = (result < 0);
}

VmValue& VM::GetPort(Port port)
{
    VmValue address = (VmValue)port * sizeof(VmValue);
    return *(VmValue*)(&Memory[address]);
}

u32 VM::GetInstructionDuration(const Instruction& instruction) const
{
    Opcode opcode = (Opcode)instruction.Op.Opcode;
    if (opcode == Opcode::Ipo || opcode == Opcode::Opo || opcode == Opcode::OpoVal)
    {
        //Port instruction times vary by port
        Port port;
        if (opcode == Opcode::Ipo || opcode == Opcode::Opo)
            port = (Port)instruction.OpRegisterValue.Value;
        else if (opcode == Opcode::OpoVal)
            port = (Port)instruction.OpPortValue.Port;

        auto search = PortDurations.find(port);
        if (search == PortDurations.end())
        {
            printf("Unsupported port %d passed to VM::GetInstructionTime()\n", (u32)port);
            return 0xFFFFFFFF;
        }

        return search->second;
    }
    else
    {
        //All other instructions use InstructionDurations
        auto search = InstructionDurations.find(opcode);
        if (search == InstructionDurations.end())
        {
            printf("Unsupported opcode '%d' passed to VM::GetInstructionTime()\n", (u32)opcode);
            return 0xFFFFFFFF;
        }

        return search->second;
    }
}

std::optional<VmValue> VM::GetConfig(const std::string& name)
{
    const std::string nameLower = String::ToLower(name);
    auto search = std::find_if(Config.begin(), Config.end(), [nameLower](const VmConfig& config) { return config.Name == nameLower; });
    bool found = search != Config.end();
    return found ? search->Value : std::optional<VmValue>{};
}

VmValue VM::GetConfigOr(const std::string& name, VmValue or)
{
    const std::string nameLower = String::ToLower(name);
    auto search = std::find_if(Config.begin(), Config.end(), [nameLower](const VmConfig& config) { return config.Name == nameLower; });
    bool found = search != Config.end();
    return found ? search->Value : or;
}
