#pragma once
#include "Typedefs.h"
#include "Instruction.h"
#include <string_view>
#include <functional>
#include <vector>

enum class Token;
struct TokenData;
struct TokenizerRule;

//Todo: Consider making this non static and exposing Rules to support more than 1 tokenizer config
//Identifies tokens within a string
class Tokenizer
{
public:
    //Returns a list of tokens and their substrings. The string must stay alive while the substrings are in use.
    static std::vector<TokenData> Tokenize(std::string_view str);
    static const std::vector<TokenizerRule> Rules;
};

//Valid tokens. See Tokenizer.cpp for the rules that detect them.
enum class Token
{
    //Tokens are set to the value of different opcodes so the compiler doesn't need to convert them
    Mov = (u32)Opcode::Mov,
    Add = (u32)Opcode::Add,
    Sub = (u32)Opcode::Sub,
    Mul = (u32)Opcode::Mul,
    Div = (u32)Opcode::Div,
    Cmp = (u32)Opcode::Cmp,
    Jmp = (u32)Opcode::Jmp,
    Jeq = (u32)Opcode::Jeq,
    Jne = (u32)Opcode::Jne,
    Jgr = (u32)Opcode::Jgr,
    Jls = (u32)Opcode::Jls,
    Call = (u32)Opcode::Call,
    Ret = (u32)Opcode::Ret,
    And = (u32)Opcode::And,
    Or = (u32)Opcode::Or,
    Xor = (u32)Opcode::Xor,
    Neg = (u32)Opcode::Neg,
    Load = (u32)Opcode::Load,
    Store = (u32)Opcode::Store,
    Push = (u32)Opcode::Push,
    Pop = (u32)Opcode::Pop,
    Register0,
    Register1,
    Register2,
    Register3,
    Register4,
    Register5,
    Register6,
    Register7,
    Value,
    Newline,
    Unknown,
    None
};

//Tokenizer output. Has the token type and the string data for that token.
struct TokenData
{
    Token Type;
    std::string_view String;

    //Returns true if the token is a register
    bool IsRegister() const
    {
        return Type >= Token::Register0 && Type <= Token::Register7;
    }
};

//Used to identify tokens. Returns true if the match function detects its token
using TokenizerMatchFunction = std::function<bool(std::string_view)>;
struct TokenizerRule
{
    TokenizerMatchFunction Function;
    Token Type;
};

static std::string to_string(Token token)
{
    switch (token)
    {
    case Token::Mov:
        return "MOV";
    case Token::Add:
        return "ADD";
    case Token::Sub:
        return "SUB";
    case Token::Mul:
        return "MUL";
    case Token::Div:
        return "DIV";
    case Token::Cmp:
        return "CMP";
    case Token::Jmp:
        return "JMP";
    case Token::Jeq:
        return "JEQ";
    case Token::Jne:
        return "JNE";
    case Token::Jgr:
        return "JGR";
    case Token::Jls:
        return "JLS";
    case Token::Call:
        return "CALL";
    case Token::Ret:
        return "RET";
    case Token::And:
        return "AND";
    case Token::Or:
        return "OR";
    case Token::Xor:
        return "XOR";
    case Token::Neg:
        return "NEG";
    case Token::Load:
        return "LOAD";
    case Token::Store:
        return "STORE";
    case Token::Push:
        return "PUSH";
    case Token::Pop:
        return "POP";
    case Token::Register0:
        return "REGISTER0";
    case Token::Register1:
        return "REGISTER1";
    case Token::Register2:
        return "REGISTER2";
    case Token::Register3:
        return "REGISTER3";
    case Token::Register4:
        return "REGISTER4";
    case Token::Register5:
        return "REGISTER5";
    case Token::Register6:
        return "REGISTER6";
    case Token::Register7:
        return "REGISTER7";
    case Token::Value:
        return "NUMBER";
    case Token::Unknown:
        return "UNKNOWN";
    case Token::None:
        return "NONE";
    case Token::Newline:
        return "NEWLINE";
    default:
        return "INVALID ENUM";
    }
}