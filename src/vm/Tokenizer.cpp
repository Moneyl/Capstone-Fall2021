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
    Match(Token::Mov, "mov"),
    Match(Token::Add, "add"),
    Match(Token::Sub, "sub"),
    Match(Token::Mul, "mul"),
    Match(Token::Div, "div"),
    Match(Token::Cmp, "cmp"),
    Match(Token::Jmp, "jmp"),
    Match(Token::Jeq, "jeq"),
    Match(Token::Jne, "jne"),
    Match(Token::Jgr, "jgr"),
    Match(Token::Jls, "jls"),
    Match(Token::Call, "call"),
    Match(Token::Ret, "ret"),
    Match(Token::And, "and"),
    Match(Token::Or, "or"),
    Match(Token::Xor, "xor"),
    Match(Token::Neg, "neg"),
    Match(Token::Load, "load"),
    Match(Token::Store, "store"),
    Match(Token::Push, "push"),
    Match(Token::Pop, "pop"),
    Match(Token::Register0, "r0"),
    Match(Token::Register1, "r1"),
    Match(Token::Register2, "r2"),
    Match(Token::Register3, "r3"),
    Match(Token::Register4, "r4"),
    Match(Token::Register5, "r5"),
    Match(Token::Register6, "r6"),
    Match(Token::Register7, "r7"),
    Rule(Token::Value, [](std::string_view str) -> bool { return String::IsNumber(str); }),
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
                tokens.push_back({ Token::Unknown, str });
            }
        }
        tokens.push_back({ Token::Newline, "\n" });
    }

    return tokens;
}