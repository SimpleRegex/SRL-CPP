#ifndef SIMPLEREGEXLANGUAGE_PARSER_H_
#define SIMPLEREGEXLANGUAGE_PARSER_H_

#include "spre/ast.hpp"
#include "spre/lexer.hpp"
#include "spre/token.hpp"
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using std::string;
using std::unique_ptr;
using std::make_unique;
using std::vector;

namespace spre
{
class Parser
{
  public:
    explicit Parser(Lexer &lexer);
    ~Parser();
    bool has_error() const;
    void report_error() const;
    vector<unique_ptr<ExprAST>> parse();

  private:
    Lexer &lexer_;
    bool error_flag_;
    string error_msg_;

    unique_ptr<CharacterExprAST> parse_character(const TokenValue &token_value);
    unique_ptr<QuantifierExprAST> parse_quantifier(const TokenValue &token_value);
    unique_ptr<GroupExprAST> parse_group(const TokenValue &token_value);
    unique_ptr<LookAroundExprAST> parse_lookaround(const TokenValue &token_value);
    unique_ptr<FlagExprAST> parse_flag(const TokenValue &token_value);
    unique_ptr<AnchorExprAST> parse_anchor(const TokenValue &token_value);
};

Parser::Parser(Lexer &lexer) : lexer_(lexer), error_flag_(false)
{
    lexer_.get_next_token(); // so now we have the first token
}

Parser::~Parser()
{
}

inline bool Parser::has_error() const
{
    return error_flag_;
}

inline void Parser::report_error() const
{
    if (!has_error())
    {
        return;
    }
    fprintf(stderr, "%s", error_msg_.c_str());
    fprintf(stderr, "\n");
}

inline vector<unique_ptr<ExprAST>> Parser::parse()
{
    vector<unique_ptr<ExprAST>> asts;
    bool eof = false;

    while (!error_flag_ && !eof)
    {
        Token token = lexer_.get_token();

        switch (token.get_token_type())
        {
        case TokenType::CHARACTER:
            asts.push_back(std::move(parse_character(token.get_token_value())));
            break;
        case TokenType::QUANTIFIER:
            asts.push_back(std::move(parse_quantifier(token.get_token_value())));
            break;
        case TokenType::GROUP:
            asts.push_back(std::move(parse_group(token.get_token_value())));
            break;
        case TokenType::LOOKAROUND:
            asts.push_back(std::move(parse_lookaround(token.get_token_value())));
            break;
        case TokenType::FLAG:
            asts.push_back(std::move(parse_flag(token.get_token_value())));
            break;
        case TokenType::ANCHOR:
            asts.push_back(std::move(parse_anchor(token.get_token_value())));
            break;
        case TokenType::END_OF_FILE:
            // we are good
            eof = true;
            break;
        case TokenType::UNDEFINED:
            error_flag_ = true;
            error_msg_ = "so invalid ast met, some errors happened";
            break;
        default:
            break;
        }
    }
    return std::move(asts);
}

inline unique_ptr<CharacterExprAST> Parser::parse_character(const TokenValue &token_value)
{
    unique_ptr<CharacterExprAST> ptr = nullptr;
    if (token_value == TokenValue::LITERALLY || token_value == TokenValue::ONE_OF || token_value == TokenValue::RAW)
    {
        // expect string literal following
        Token next_token = lexer_.get_next_token();
        if (next_token.get_token_type() != TokenType::SRC_STRING)
        {
            error_flag_ = true;
            error_msg_ = "missing string literal";
        }
        else
        {
            string val;
            switch (token_value)
            {
            case TokenValue::LITERALLY:
                val = "(?:" + next_token.get_value() + ")";
                break;
            case TokenValue::ONE_OF:
                val = "[" + next_token.get_value() + "]";
                break;
            case TokenValue::RAW:
                val = next_token.get_value();
                break;
            default:
                break;
            }
            ptr = make_unique<CharacterExprAST>(val);
            lexer_.get_next_token(); // so we eat the leagal token
        }
    }

    else if (token_value == TokenValue::LETTER || token_value == TokenValue::UPPERCASE_LETTER || token_value == TokenValue::DIGIT)
    {
        Token next_token = lexer_.get_next_token();

        if (next_token.get_token_value() == TokenValue::FROM)
        {
            Token next_next_token = lexer_.get_next_token();
            if (next_next_token.get_token_value() == TokenValue::TO)
            {
                // so we have the modifier
                string az = next_next_token.get_value();
                if (az.length() == 2)
                {
                    az.insert(1, "-");
                    az.insert(0, "[");
                    az.append("]");
                    ptr = make_unique<CharacterExprAST>(az);
                    lexer_.get_next_token(); // so we eat the leagal tokens from and to
                }
            }
            else
            {
                error_flag_ = true;
                error_msg_ = "\"from\" found, but \"to\" not found";
            }
        }
        else
        {
            string val;
            switch (token_value)
            {
            case TokenValue::LETTER:
                val = "[a-z]";
                break;
            case TokenValue::UPPERCASE_LETTER:
                val = "[A-Z]";
                break;
            case TokenValue::DIGIT:
                val = "[0-9]";
                break;
            default:
                break;
            }
            ptr = make_unique<CharacterExprAST>(val);
            lexer_.get_next_token(); // so we eat the leagal token
        }
    }
    else
    {
        string val;
        switch (token_value)
        {
        case TokenValue::ANY_CHARACTER:
            val = "\\w";
            break;
        case TokenValue::NO_CHARACTER:
            val = "\\W";
            break;
        case TokenValue::ANYTHING:
            val = ".";
            break;
        case TokenValue::NEW_LINE:
            val = "\\n";
            break;
        case TokenValue::WHITESPACE:
            val = "\\s";
            break;
        case TokenValue::NO_WHITESPACE:
            val = "\\S";
            break;
        case TokenValue::TAB:
            val = "\\t";
            break;
        default:
            break;
        }
        if (val.length() != 0)
        {
            ptr = make_unique<CharacterExprAST>(val);
            lexer_.get_next_token(); // so we eat the leagal token
        }
        else
        {
            error_flag_ = true;
            error_msg_ = "unknown error";
        }
    }

    return std::move(ptr);
}

inline unique_ptr<QuantifierExprAST> Parser::parse_quantifier(const TokenValue &token_value)
{
    unique_ptr<QuantifierExprAST> ptr;
    string val;

    switch (token_value)
    {
    case TokenValue::EXCATLY_X_TIMES:
    {
        Token next_token = lexer_.get_next_token();
        Token next_next_token = lexer_.get_next_token();
        if (next_token.get_token_value() == TokenValue::NUMBER && (next_next_token.get_token_value() == TokenValue::TIME || next_next_token.get_token_value() == TokenValue::TIMES))
        {
            if (next_token.get_value() != "1" && next_next_token.get_token_value() == TokenValue::TIME)
            {
                ptr = nullptr;
                error_flag_ = true;
                error_msg_ = "you should say \"x times\" instead of \"x time\" if x > 1";
            }
            else
            {
                ptr = make_unique<QuantifierExprAST>("{" + next_token.get_value() + "}");
                lexer_.get_next_token(); // eat the trailing "times"
            }
        }
        else
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "the number following \"exactly\" not found";
        }
		break;
    }
    
    case TokenValue::EXACTLY_ONE_TIME:
        val = "{1}";
        ptr = make_unique<QuantifierExprAST>(val);
        lexer_.get_next_token();
        break;
    case TokenValue::ONCE:
        val = "{1}";
        ptr = make_unique<QuantifierExprAST>(val);
        lexer_.get_next_token();
        break;
    case TokenValue::TWICE:
        val = "{2}";
        ptr = make_unique<QuantifierExprAST>(val);
        lexer_.get_next_token();
        break;
    case TokenValue::BETWEEN_X_AND_Y_TIMES:
    {
        Token x = lexer_.get_next_token();
        Token and_token = lexer_.get_next_token();
        Token y = lexer_.get_next_token();
        Token times = lexer_.get_next_token();
        if (x.get_token_value() == TokenValue::NUMBER && and_token.get_token_value() == TokenValue::AND && y.get_token_value() == TokenValue::NUMBER)
        {
            string val = "{";
            val.append(x.get_value());
            val.append(",");
            val.append(y.get_value());
            val.append("}");
            ptr = make_unique<QuantifierExprAST>(val);
            if (times.get_token_value() == TokenValue::TIMES)
            {
                lexer_.get_next_token(); // eat the trailing "times"
                                         // else we do not move forward
            }
        }
        else
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "invalid \"between x and y times\"";
        }

		break;
    }
    case TokenValue::OPTIONAL:
        val = "?";
        ptr = make_unique<QuantifierExprAST>(val);
        lexer_.get_next_token();
        break;
    case TokenValue::ONCE_OR_MORE:
        val = "+";
        ptr = make_unique<QuantifierExprAST>(val);
        lexer_.get_next_token();
        break;
    case TokenValue::NEVER_OR_MORE:
        val = "?";
        ptr = make_unique<QuantifierExprAST>(val);
        lexer_.get_next_token();
        break;
    case TokenValue::AT_LEAST_X_TIMES:
    {
        Token x = lexer_.get_next_token();
        Token times = lexer_.get_next_token();
        if (x.get_token_value() == TokenValue::NUMBER && times.get_token_value() == TokenValue::TIMES)
        {
            ptr = make_unique<QuantifierExprAST>("{" + x.get_value() + "}");
            lexer_.get_next_token();
        }
        else
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "invalid \"at least x times\"";
        }

		break;
    }
    
    default:
        ptr = nullptr;
        error_flag_ = true;
        error_msg_ = "unknown quantifier-like statement";
        break;
    }

    return std::move(ptr);
}

inline unique_ptr<GroupExprAST> Parser::parse_group(const TokenValue &token_value)
{
    unique_ptr<GroupExprAST> ptr;
    switch (token_value)
    {
    case TokenValue::CAPTURE_AS:
    {
        Token group_start = lexer_.get_next_token();
        vector<unique_ptr<ExprAST>> cond = parse();
        // after parsing the sub_query_ptr_vec, current token should be ")"!!!
        Token group_end = lexer_.get_token();
        if (group_start.get_token_value() == TokenValue::GROUP_START && group_end.get_token_value() == TokenValue::GROUP_END)
        {
            ptr = make_unique<GroupExprAST>(std::move(cond));
            // we have a smallest possible ptr now.

            Token next_token = lexer_.get_next_token();
            if (next_token.get_token_value() == TokenValue::AS)
            {
                Token name = lexer_.get_next_token();
                if (name.get_token_value() == TokenValue::STRING)
                {
                    // prefect name!
                    ptr->set_name(name.get_value());
                    next_token = lexer_.get_next_token();
                }
                else
                {
                    ptr = nullptr;
                    error_flag_ = true;
                    error_msg_ = "the name in \"capture (cond) as \"name\"\" is invalid";
                }
            }

            // right now the current token
            // is the one after the whole
            // capture (cond) [as "name"]
        }
        else
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "the condition in \"capture (cond)\" is invalid";
        }

		break;
    }
 
    case TokenValue::UNTIL:
    {
        Token str_or_group_start = lexer_.get_next_token();
        switch (str_or_group_start.get_token_value())
        {
        case TokenValue::STRING:
            // TODO
            break;
        case TokenValue::GROUP_START:
            // TODO
            break;
        default:
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "the thing after \"until\" should be string or sub group";
            break;
        }

		break;
    }
  
    case TokenValue::ANY_OF:
        // TODO
        break;
    default:
        break;
    }
    return std::move(ptr);
}

inline unique_ptr<LookAroundExprAST> Parser::parse_lookaround(const TokenValue &token_value)
{
    unique_ptr<LookAroundExprAST> ptr;

    switch (token_value)
    {
    case TokenValue::IF_FOLLOWED_BY:
    case TokenValue::IF_NOT_FOLLOWED_BY:
    {
        Token group_start = lexer_.get_next_token();
        vector<unique_ptr<ExprAST>> sub_query_ptrs_vec = parse();
        // after parsing the sub_query_ptr_vec, current token should be ")"!!!
        Token group_end = lexer_.get_token();
        if (group_start.get_token_value() == TokenValue::GROUP_START && group_end.get_token_value() == TokenValue::GROUP_END)
        {
            string left_symbol =
                (token_value == TokenValue::IF_FOLLOWED_BY ? "(?=" : "(?!");
            ptr = make_unique<LookAroundExprAST>(
                vector<string>{left_symbol, ")"}, std::move(sub_query_ptrs_vec));
            lexer_.get_next_token();
        }
        else
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "invalid \"if [not] followed by (condition)\"";
        }

		break;
    }

    case TokenValue::IF_ALREADY_HAD:
    case TokenValue::IF_NOT_ALREADY_HAD:
    {
        Token str = lexer_.get_next_token();
        if (str.get_token_value() == TokenValue::STRING)
        {
            string symbol =
                (token_value == TokenValue::IF_ALREADY_HAD ? "?<=" : "?<!");
            ptr = make_unique<LookAroundExprAST>(
                vector<string>{"(" + symbol + "(?:" + str.get_value() + "))"});
            lexer_.get_next_token();
        }
        else
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "\"if [not] already had\" should be followed by string literal";
        }

		break;
    }
 
    default:
        ptr = nullptr;
        error_flag_ = true;
        error_msg_ = "unknown lookaround-like statement";
        break;
    }

    return std::move(ptr);
}

inline unique_ptr<FlagExprAST> Parser::parse_flag(const TokenValue &token_value)
{
    unique_ptr<FlagExprAST> ptr;
    switch (token_value)
    {
    case TokenValue::CASE_INSENSITIVE:
        ptr = make_unique<FlagExprAST>("i");
        lexer_.get_next_token();
        break;
    case TokenValue::MULTI_LINE:
        ptr = make_unique<FlagExprAST>("m");
        lexer_.get_next_token();
        break;
    case TokenValue::ALL_LAZY:
        ptr = make_unique<FlagExprAST>("U");
        lexer_.get_next_token();
        break;
    default:
        ptr = nullptr;
        error_flag_ = true;
        error_msg_ = "unknown flag-like statement";
        break;
    }

    return std::move(ptr);
}

inline unique_ptr<AnchorExprAST> Parser::parse_anchor(const TokenValue &token_value)
{
    unique_ptr<AnchorExprAST> ptr;
    switch (token_value)
    {
    case TokenValue::STARTS_WITH:
    case TokenValue::BEGIN_WITH:
        ptr = make_unique<AnchorExprAST>("^");
        lexer_.get_next_token();
        break;
    case TokenValue::MUST_END:
        ptr = make_unique<AnchorExprAST>("$");
        lexer_.get_next_token();
        break;
    default:
        ptr = nullptr;
        error_flag_ = true;
        error_msg_ = "unknown anchor-like statement";
        break;
    }

    return std::move(ptr);
}
}

#endif // !SIMPLEREGEXLANGUAGE_PARSER_H_
