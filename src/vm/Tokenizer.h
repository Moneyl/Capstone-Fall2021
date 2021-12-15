#pragma once
#include "Typedefs.h"
#include "Instruction.h"
#include <string_view>
#include "utility/Result.h"
#include <magic_enum.hpp>
#include <functional>
#include <vector>

enum class Token : u8;
struct TokenData;
struct TokenizerRule;
struct TokenizerError;
enum class TokenizerErrorCode;

//Identifies tokens within a string
class Tokenizer
{
public:
    //Returns a list of tokens and their substrings. The string must stay alive while the substrings are in use.
    static Result<std::vector<TokenData>, TokenizerError> Tokenize(std::string_view str);
    static const std::vector<TokenizerRule> Rules;
};

//Valid tokens. See Tokenizer.cpp for the rules that detect them.
enum class Token : u8
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
    Ipo = (u32)Opcode::Ipo,
    Opo = (u32)Opcode::Opo,
    Config,
    Register,
    Var,
    VarName,
    Label,
    Value,
    Constant,
    Newline,
    None //Used by the compiler when it does token lookahead and goes out of bounds
};

//Tokenizer output. Has the token type and the string data for that token.
struct TokenData
{
    std::string_view String;
    Token Type;
};

//Used to identify tokens. Returns true if the match function detects its token
using TokenizerMatchFunction = std::function<bool(std::string_view)>;
struct TokenizerRule
{
    TokenizerMatchFunction Function;
    Token Type;
};

enum class TokenizerErrorCode
{
    UnsupportedToken,
};

struct TokenizerError
{
    TokenizerErrorCode Code;
    std::string Message;
};

static std::string to_string(Token token)
{
    return std::string(magic_enum::enum_name(token));
}

static std::string to_string(TokenizerErrorCode value)
{
    return std::string(magic_enum::enum_name(value));
}