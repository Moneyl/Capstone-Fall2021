#pragma once
#include "Typedefs.h"
#include "utility/String.h"
#include <magic_enum.hpp>
#include <string>
#include <cmath>

//Number of bits/bytes per value. Addresses are the same size.
const u32 INSTRUCTION_VALUE_BITS = 16;
//The number of bytes per value. Rounded up so the VM doesn't need to deal with sub-byte data sizes if non byte aligned values are ever tried.
const u32 INSTRUCTION_VALUE_BYTES = u32(std::ceil(f32(INSTRUCTION_VALUE_BITS / 8)));

using VmValue = i16; //VM variable size
using Register = VmValue; //VM register size

//Instruction layout based on SUNY AT instructions. Modified to allow for larger addresses and values, and more variables
union Instruction
{
    //(mov|add|sub|mul|div|cmp|and|or|xor|load|store) register register
    struct
    {
        u32 Opcode : 5; //Note: Allows up to 32 opcodes
        u32 RegA : 3;
        u32 RegB : 3;
    } OpRegisterRegister;

    //(mov|add|sub|mul|div|cmp|and|or|xor|load|store) register value
    struct
    {
        u32 Opcode : 5;
        u32 RegA : 3;
        i32 Value : INSTRUCTION_VALUE_BITS; //Range: [-32768, 32767]
    } OpRegisterValue;

    //(jmp|jeq|jne|jgr|jls|call) address
    struct
    {
        u32 Opcode : 5;
        u32 Unused : 3;
        u32 Address : INSTRUCTION_VALUE_BITS; //Range: [-32768, 32767]
    } OpAddress;

    //(neg|push|pop) register
    struct
    {
        u32 Opcode : 5;
        u32 Reg : 3;
    } OpRegister;

    //ret
    struct
    {
        u32 Opcode : 5;
    } Op;

    //Split into upper and lower bits
    struct
    {
        u32 Upper : 16;
        u32 Lower : 16;
    } Split;

    //Split into bytes
    struct
    {
        u32 Byte0 : 8;
        u32 Byte1 : 8;
        u32 Byte2 : 8;
        u32 Byte3 : 8;
    } Bytes;

    //Interpret as an integer
    u32 Value;
};
static_assert(sizeof(Instruction) == 4, "sizeof(Instruction) must be 4 bytes");

//IMPORTANT: Don't change the values of these without updating the Compiler. It relies on the enums having certain values to shorten the code.
//Used to identify instructions
enum class Opcode
{
    Mov = 0,        //mov register register
    MovVal = 1,     //mov register value
    Add = 2,        //add register register
    AddVal = 3,     //add register value
    Sub = 4,        //sub register register
    SubVal = 5,     //sub register value
    Mul = 6,        //mul register register
    MulVal = 7,     //mul register value
    Div = 8,        //div register register
    DivVal = 9,     //div register value
    Cmp = 10,       //cmp register register
    CmpVal = 11,    //cmp register value
    Jmp = 12,       //jmp address
    Jeq = 13,       //jeq address
    Jne = 14,       //jne address
    Jgr = 15,       //jgr address
    Jls = 16,       //jls address
    Call = 17,      //call address
    Ret = 18,       //ret
    And = 19,       //and register register
    AndVal = 20,    //and register value
    Or = 21,        //or register register
    OrVal = 22,     //or register value
    Xor = 23,       //xor register register
    XorVal = 24,    //xor register value
    Neg = 25,       //neg register
    Load = 26,      //load register address
    LoadP = 27,     //load register register
    Store = 28,     //store address register
    StoreP = 29,    //store register register
    Push = 30,      //push register
    Pop = 31        //pop register
};

static std::string to_string(Opcode opcode, bool useRealOpcodeNames = false)
{
    std::string str(magic_enum::enum_name(opcode));

    //Make lowercase and strip extra specifiers for disassembler use. E.g. Jmp -> jmp, MovVal -> mov
    if (!useRealOpcodeNames)
    {
        //Make lowercase
        str = String::ToLower(str);

        //Strip extra specifiers like Val and P
        if (str.find("val") != std::string::npos)
        {
            str.pop_back();
            str.pop_back();
            str.pop_back();
        }
        if (opcode == Opcode::LoadP || opcode == Opcode::StoreP)
        {
            str.pop_back();
        }
    }

    return str;
}

static std::string to_string(const Instruction& instruction, bool useRealOpcodeNames = false)
{
    switch ((Opcode)instruction.Op.Opcode)
    {
    //Instructions that use OpRegisterRegister
    case Opcode::Mov:
    case Opcode::Add:
    case Opcode::Sub:
    case Opcode::Mul:
    case Opcode::Div:
    case Opcode::Cmp:
    case Opcode::And:
    case Opcode::Or:
    case Opcode::Xor:
    case Opcode::LoadP:
    case Opcode::StoreP:
        return to_string((Opcode)instruction.Op.Opcode, useRealOpcodeNames) + " "
               + "r" + std::to_string(instruction.OpRegisterRegister.RegA) + " "
               + "r" + std::to_string(instruction.OpRegisterRegister.RegB);

    //Instructions that use OpRegisterValue
    case Opcode::MovVal:
    case Opcode::AddVal:
    case Opcode::SubVal:
    case Opcode::MulVal:
    case Opcode::DivVal:
    case Opcode::CmpVal:
    case Opcode::AndVal:
    case Opcode::OrVal:
    case Opcode::XorVal:
    case Opcode::Load:
        return to_string((Opcode)instruction.Op.Opcode, useRealOpcodeNames) + " "
               + "r" + std::to_string(instruction.OpRegisterValue.RegA) + " "
               + std::to_string(instruction.OpRegisterValue.Value);

    //Special case for this variant of store
    case Opcode::Store:
        return to_string((Opcode)instruction.Op.Opcode, useRealOpcodeNames) + " "
               + std::to_string(instruction.OpRegisterValue.Value) + " "
               + "r" + std::to_string(instruction.OpRegisterValue.RegA);

    //Instructions that use OpAddress
    case Opcode::Jmp:
    case Opcode::Jeq:
    case Opcode::Jne:
    case Opcode::Jgr:
    case Opcode::Jls:
    case Opcode::Call:
        return to_string((Opcode)instruction.Op.Opcode, useRealOpcodeNames) + " "
               + std::to_string(instruction.OpAddress.Address);

    //Instructions that use Op
    case Opcode::Ret:
        return to_string((Opcode)instruction.Op.Opcode, useRealOpcodeNames);

    //Instructions that use OpRegister
    case Opcode::Neg:
    case Opcode::Push:
    case Opcode::Pop:
        return to_string((Opcode)instruction.Op.Opcode, useRealOpcodeNames) + " "
               + "r" + std::to_string(instruction.OpRegister.Reg);

    default:
        return "Unsupported opcode " + std::to_string((u32)instruction.Op.Opcode);
    }
}