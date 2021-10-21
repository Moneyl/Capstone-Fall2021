#include "Tokenizer.h"
#include "utility/String.h"
#include <iostream>

//Detect token with boolean func
TokenizerRule Rule(Token token, TokenizerMatchFunction func)
{
    return { func, token };
}

//Detect token with string matching
TokenizerRule Match(Token token, std::string_view match)
{
    return { [=](std::string_view str) -> bool { return match == str; }, token };
}

//Tokenizer rules
const std::vector<TokenizerRule> Tokenizer::Rules =
{
    Match(MOV, "mov"),
    Match(ADD, "add"),
    Match(SUB, "sub"),
    Match(MUL, "mul"),
    Match(DIV, "div"),
    Match(CMP, "cmp"),
    Match(JMP, "jmp"),
    Match(JEQ, "jeq"),
    Match(JNE, "jne"),
    Match(JGR, "jgr"),
    Match(JLS, "jls"),
    Match(CALL, "call"),
    Match(RET, "ret"),
    Match(AND, "and"),
    Match(OR, "or"),
    Match(XOR, "xor"),
    Match(NEG, "neg"),
    Match(LOAD, "load"),
    Match(STORE, "store"),
    Match(PUSH, "push"),
    Match(POP, "pop"),
    Match(REGISTER0, "r0"),
    Match(REGISTER1, "r1"),
    Match(REGISTER2, "r2"),
    Match(REGISTER3, "r3"),
    Match(REGISTER4, "r4"),
    Match(REGISTER5, "r5"),
    Match(REGISTER6, "r6"),
    Match(REGISTER7, "r7"),
    Rule(NUMBER, [](std::string_view str) -> bool { return String::IsNumber(str); }),
};

std::vector<TokenData> Tokenizer::Tokenize(std::string_view str)
{
    //Tokenize each line
    std::vector<TokenData> tokens = {};
    std::vector<std::string_view> lines = String::Split(str, "\n");
    for (std::string_view line : lines)
    {
        //Ignore anything following semicolons (comments)
        auto semicolonIndex = line.find_first_of(';');
        if (semicolonIndex != std::string_view::npos)
            line = line.substr(0, semicolonIndex);

        //Split string by space characters check for tokens
        std::vector<std::string_view> substrings = String::Split(line, " ");
        for (std::string_view str : substrings)
        {
            //Run each tokenizer rule on the lowercase string
            const std::string strLowercase = String::ToLower(str);
            bool match = false;
            for (const TokenizerRule& rule : Rules)
            {
                if (rule.Function(strLowercase))
                {
                    tokens.push_back({ rule.Type, str });
                    match = true;
                }
            }

            //String doesn't match any rules
            if (!match)
            {
                //Todo: Add proper logging to files
                std::cout << "Unknown token: \"" << strLowercase << "\". Ignoring...\n";
                tokens.push_back({ UNKNOWN, str });
            }
        }
    }

    return tokens;
}