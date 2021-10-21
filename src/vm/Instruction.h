#pragma once
#include "Typedefs.h"

//Instruction layout based on SUNY AT instructions. Modified to allow for larger addresses and values, and more variables
union Instruction
{
    //(mov|add|sub|mul|div|cmp|and|or|xor|load|store) reg reg
    struct
    {
        u16 Opcode : 5; //Note: Allows 32 instruction types
        u16 Reg0 : 3;
        u16 Reg1 : 3;
    } RegisterOp;
    //(mov|add|sub|mul|div|cmp|and|or|xor|load|store) reg value
    struct
    {
        u16 Opcode : 5;
        u16 Reg0 : 3;
        u16 Value : 8; //Note: Only allows [-128, 127]
    } RegisterValueOp;
    //(jmp|jeq|jne|jgr|jls) address
    struct
    {
        u16 Opcode : 5;
        u16 Empty0 : 3;
        u16 Address : 8;
    } Jump;
    //(neg|push|pop) reg
    struct
    {
        u16 Opcode : 5;
        u16 Reg : 3;
    } Negate;
    //Split into upper and lower bits
    struct
    {
        u16 Upper : 8;
        u16 Lower : 8;
    } Split;

    u16 Word;
};

static_assert(sizeof(Instruction) == 2, "sizeof(Instruction) must be 2 bytes");

//Used to identify instructions
enum class Opcodes
{
    MOV = 0,        //mov reg reg
    MOV_VAL = 1,    //mov reg value
    ADD = 2,        //add reg reg
    ADD_VAL = 3,    //add reg value
    SUB = 4,        //sub reg reg
    SUB_VAL = 5,    //sub reg value
    MUL = 6,        //mul reg reg
    MUL_VAL = 7,    //mul reg value
    DIV = 8,        //div reg reg
    DIV_VAL = 9,    //div reg value
    CMP = 10,       //cmp reg reg
    CMP_VAL = 11,   //cmp reg value
    JMP = 12,       //jmp address
    JEQ = 13,       //jeq address
    JNE = 14,       //jne address
    JGR = 15,       //jgr address
    JLS = 16,       //jls address
    CALL = 17,      //call address
    RET = 18,       //ret
    AND = 19,       //and reg reg
    AND_VAL = 20,   //and reg value
    OR = 21,        //or reg reg
    OR_VAL = 22,    //or reg value
    XOR = 23,       //xor reg reg
    XOR_VAL = 24,   //xor reg value
    NEG = 25,       //neg reg
    LOAD = 26,      //load reg address
    LOAD_P = 27,    //load reg reg
    STORE = 28,     //store reg address
    STORE_P = 29,   //store reg reg
    PUSH = 30,      //push reg
    POP = 31        //pop reg
};