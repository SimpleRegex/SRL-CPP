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
    explicit Parser(Lexer &lexer, bool show_error = true);
    ~Parser();
    bool has_error() const;
    void report_error() const;
    vector<unique_ptr<ExprAST>> parse();

  private:
    Lexer &lexer_;
    bool error_flag_;
    string error_msg_;
    const bool show_error_;

    unique_ptr<ExprAST> parse_token(const Token &token);
    unique_ptr<CharacterExprAST> parse_character(const TokenValue &token_value);
    unique_ptr<QuantifierExprAST> parse_quantifier(const TokenValue &token_value);
    unique_ptr<GroupExprAST> parse_group(const TokenValue &token_value);
    unique_ptr<LookAroundExprAST> parse_lookaround(const TokenValue &token_value);
    unique_ptr<FlagExprAST> parse_flag(const TokenValue &token_value);
    unique_ptr<AnchorExprAST> parse_anchor(const TokenValue &token_value);
    unique_ptr<EOFExprAST> parse_eof(const TokenValue &token_value);
};

Parser::Parser(Lexer &lexer, bool show_error) : lexer_(lexer), error_flag_(false), show_error_(show_error)
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
    fprintf(stderr, "parser error: ");
    fprintf(stderr, "%s", error_msg_.c_str());
    fprintf(stderr, "\n");
}

inline vector<unique_ptr<ExprAST>> Parser::parse()
{
    vector<unique_ptr<ExprAST>> asts;
    bool eof = false;

    while (!lexer_.has_error() && !error_flag_ && !eof)
    {
        Token token = lexer_.get_token();
        //std::cout << token.get_value() << "\n";
        if (token.get_token_type() == TokenType::END_OF_FILE)
        {
            eof = true;
        }
        asts.push_back(std::move(parse_token(token)));
    }
    return std::move(asts);
}


inline unique_ptr<ExprAST> Parser::parse_token(const Token &token)
{
    unique_ptr<ExprAST> ptr;
    switch (token.get_token_type())
    {
    case TokenType::CHARACTER:
        ptr = std::move(parse_character(token.get_token_value()));
        break;
    case TokenType::QUANTIFIER:
        ptr = std::move(parse_quantifier(token.get_token_value()));
        break;
    case TokenType::GROUP:
        ptr = std::move(parse_group(token.get_token_value()));
        break;
    case TokenType::LOOKAROUND:
        ptr = std::move(parse_lookaround(token.get_token_value()));
        break;
    case TokenType::FLAG:
        ptr = std::move(parse_flag(token.get_token_value()));
        break;
    case TokenType::ANCHOR:
        ptr = std::move(parse_anchor(token.get_token_value()));
        break;
    case TokenType::END_OF_FILE:
        ptr = std::move(parse_eof(token.get_token_value()));
        break;
    case TokenType::UNDEFINED:
        error_flag_ = true;
        error_msg_ = "so invalid ast met, some errors happened";
        break;
    default:
        break;
    }

    if (show_error_)
    {
        report_error();
    }

    return std::move(ptr);
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
            return ptr;
        }

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
        return ptr;
    }

    if (token_value == TokenValue::LETTER || token_value == TokenValue::UPPERCASE_LETTER || token_value == TokenValue::DIGIT)
    {
        Token guess_from = lexer_.get_next_token();

        if (guess_from.get_token_value() != TokenValue::FROM)
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
            // now we already at the one after letter/digit/...
            // because we already move to here for guessing from
            return ptr;
        }

        Token guess_to = lexer_.get_next_token();

        if (guess_from.get_token_value() != TokenValue::FROM)
        {
            error_flag_ = true;
            error_msg_ = "\"from\" found, but \"to\" not found";
            return ptr;
        }

        string az = guess_to.get_value();

        if (az.length() != 2)
        {
            error_flag_ = true;
            error_msg_ = "the range \"from\" and \"to\" is not well defined";
            return ptr;
        }

        az.insert(1, "-");
        az.insert(0, "[");
        az.append("]");
        ptr = make_unique<CharacterExprAST>(az);
        lexer_.get_next_token(); // so we eat the leagal token to
        return ptr;
    }

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
        if (group_start.get_token_value() != TokenValue::GROUP_START)
        {
            // early return
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "capture should come with \"(...)\"";
            return std::move(ptr);
        }
        lexer_.get_next_token(); // after parsing "(", now the token become the inside part
        vector<unique_ptr<ExprAST>> cond;
        do 
        {
            cond.push_back(std::move(parse_token(lexer_.get_token())));
            // after parsing, lexer_.get_token() become the one following.
        } while (lexer_.get_token().get_token_value() != TokenValue::GROUP_END
            && lexer_.get_token().get_token_type() != TokenType::END_OF_FILE
            && lexer_.get_token().get_token_type() != TokenType::UNDEFINED);
        // after parsing the sub_query_ptr_vec, current token should be ")"!!!
        
        if (lexer_.get_token().get_token_value() != TokenValue::GROUP_END)
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "capture condition doesn't end correctly";
            return std::move(ptr);
        }
        ptr = make_unique<GroupExprAST>(std::move(cond));

        Token guess_as = lexer_.get_next_token();
        if (guess_as.get_token_value() == TokenValue::AS)
        {
            Token name = lexer_.get_next_token();
            if (name.get_token_value() == TokenValue::STRING)
            {
                // prefect name!
                ptr->set_name(name.get_value());
                lexer_.get_next_token();
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

        return ptr;
        //break;
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

    Token guess = lexer_.get_next_token();
    vector<unique_ptr<ExprAST>> cond;

    switch (guess.get_token_value())
    {
    case TokenValue::STRING:
        cond.push_back(std::move(make_unique<CharacterExprAST>(guess.get_value())));
        lexer_.get_next_token();
        break;
    case TokenValue::GROUP_START:
        lexer_.get_next_token(); // after parsing "(", now the token become the inside part
        do
        {
            cond.push_back(std::move(parse_token(lexer_.get_token())));
            // after parsing, lexer_.get_token() become the one following.
        } while (lexer_.get_token().get_token_value() != TokenValue::GROUP_END
            && lexer_.get_token().get_token_type() != TokenType::END_OF_FILE
            && lexer_.get_token().get_token_type() != TokenType::UNDEFINED);
        // after parsing the sub query, current token should be ")"!!!

        if (lexer_.get_token().get_token_value() != TokenValue::GROUP_END)
        {
            ptr = nullptr;
            error_flag_ = true;
            error_msg_ = "the lookaround condition doesn't end correctly";
            return ptr;
        }
        
        lexer_.get_next_token(); // now the current one is the one after ")"
        break;
    default:
        error_flag_ = true;
        error_msg_ = "the lookaround part doesn't have correct following statements";
        return ptr;
        //break;
    }

    switch (token_value)
    {
    case TokenValue::IF_FOLLOWED_BY:
    case TokenValue::IF_NOT_FOLLOWED_BY:
    {
        string left_symbol =
            (token_value == TokenValue::IF_FOLLOWED_BY ? "(?=" : "(?!");
        ptr = make_unique<LookAroundExprAST>(
            vector<string>{left_symbol, ")"}, std::move(cond));
		break;
    }

    case TokenValue::IF_ALREADY_HAD:
    case TokenValue::IF_NOT_ALREADY_HAD:
    {
        string left_symbol =
            (token_value == TokenValue::IF_ALREADY_HAD ? "?<=" : "?<!");
        ptr = make_unique<LookAroundExprAST>(
            vector<string>{left_symbol, ")"});
        lexer_.get_next_token();
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

inline unique_ptr<EOFExprAST> Parser::parse_eof(const TokenValue &token_value)
{
    // maybe we check the value in the future?
    return make_unique<EOFExprAST>();
}

}

#endif // !SIMPLEREGEXLANGUAGE_PARSER_H_
