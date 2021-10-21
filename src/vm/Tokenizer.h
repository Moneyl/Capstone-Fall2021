#pragma once
#include "Typedefs.h"
#include <string_view>
#include <functional>
#include <vector>

enum Token;
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
enum Token
{
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    CMP,
    JMP,
    JEQ,
    JNE,
    JGR,
    JLS,
    CALL,
    RET,
    AND,
    OR,
    XOR,
    NEG,
    LOAD,
    STORE,
    PUSH,
    POP,
    REGISTER0,
    REGISTER1,
    REGISTER2,
    REGISTER3,
    REGISTER4,
    REGISTER5,
    REGISTER6,
    REGISTER7,
    NUMBER,
    UNKNOWN
};

//Tokenizer output. Has the token type and the string data for that token.
struct TokenData
{
    Token Type;
    std::string_view String;

    //Returns true if the token is a register
    bool Register() const
    {
        return Type >= REGISTER0 && Type <= REGISTER7;
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
    case MOV:
        return "MOV";
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case MUL:
        return "MUL";
    case DIV:
        return "DIV";
    case CMP:
        return "CMP";
    case JMP:
        return "JMP";
    case JEQ:
        return "JEQ";
    case JNE:
        return "JNE";
    case JGR:
        return "JGR";
    case JLS:
        return "JLS";
    case CALL:
        return "CALL";
    case RET:
        return "RET";
    case AND:
        return "AND";
    case OR:
        return "OR";
    case XOR:
        return "XOR";
    case NEG:
        return "NEG";
    case LOAD:
        return "LOAD";
    case STORE:
        return "STORE";
    case PUSH:
        return "PUSH";
    case POP:
        return "POP";
    case REGISTER0:
        return "REGISTER0";
    case REGISTER1:
        return "REGISTER1";
    case REGISTER2:
        return "REGISTER2";
    case REGISTER3:
        return "REGISTER3";
    case REGISTER4:
        return "REGISTER4";
    case REGISTER5:
        return "REGISTER5";
    case REGISTER6:
        return "REGISTER6";
    case REGISTER7:
        return "REGISTER7";
    case NUMBER:
        return "NUMBER";
    case UNKNOWN:
        return "UNKNOWN";
    default:
        return "INVALID ENUM";
    }
}