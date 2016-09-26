/*
 * basically to look up the token
 *
 *
 * everytime calling lexer.get_next_token(), we will update the 
 * lexer.get_token(). It should be able to be called multiple times
 * until lexer.has_ended() || lexer.has_error().
 * 
 * Inside the implementation, we have a cursor pointing to the next
 * char to lexer.curr_char_. Everytime the get_next_token() is called, 
 * we will try to scan from (with) curr_char_ to the end char, then 
 * set the curr_char_ as the one **right after** the valid token. This
 * is important to keep this invariant. Furthermore, in the Lexer level,
 * we only focus on the identifier (including keywords, "(", ")", ","),
 * and string literal surrounded, and number, and space, and eof, 
 * and error!
 * 
 */

#ifndef SIMPLEREGEXLANGUAGE_LEXER_H_
#define SIMPLEREGEXLANGUAGE_LEXER_H_

#include "spre/dictionary.hpp"
#include "spre/token.hpp"
#include <cctype>
#include <cstdio>
#include <string>

using std::string;

namespace spre
{
class Lexer
{
  public:
    explicit Lexer(const string &src = "");
    ~Lexer();
    Token get_token() const;
    Token get_next_token();
    bool has_error() const;
    void report_error() const;
    bool has_ended() const;

    enum class State
    {
        IDENTIFIER,
        NUMBER,
        STRING,

        NONE,
        END_OF_FILE,
        ERROR
    };

  private:
    const string src_;
    const size_t src_len_; // cache the length
    size_t src_cursor_;    // cursor always points to next char
    char curr_char_;       // always src_[src_cursor_ - 1] == curr_char_
    string buffer_;        // one string object to eat the chars while necessary
    State state_;
    Token token_;
    Dictionary dictionary_;
    bool error_flag_;
    string error_msg_;

    void move_to_next_char();
    char peek_prev_char(size_t k = 1) const;
    char peek_next_char(size_t k = 1) const;
    void handle_eof_state();
    void handle_identifier_state();
    void handle_number_state();
    void handle_string_state(char string_state_delimiter = '\"');
};

Lexer::Lexer(const string &src) : src_(src), src_len_(src.length()),
                                  src_cursor_(0), curr_char_(' '),
                                  token_(Token()), state_(State::NONE),
                                  error_flag_(false)
{
}

Lexer::~Lexer()
{
}

inline Token Lexer::get_token() const
{
    return token_;
}

inline bool Lexer::has_error() const
{
    return state_ == State::ERROR;
}

inline void Lexer::report_error() const
{
    if (!has_error())
    {
        return;
    }
    fprintf(stderr, "%s", error_msg_.c_str());
    fprintf(stderr, "\n");
}

inline bool Lexer::has_ended() const
{
    // here is a concept issue, how to define ended?
    // for example,
    // "some string"
    //            ^^
    //  curr_char_  src_cursor    <- is it ended?
    // "some string"
    //             ^^
    //   curr_char_  src_cursor    <- or is it ended?
    return state_ == State::END_OF_FILE;
}

inline void Lexer::move_to_next_char()
{
    curr_char_ = src_cursor_ < src_len_ ? src_[src_cursor_] : '\0';
    src_cursor_ += 1; // the position next to that of curr_char_
}

inline char Lexer::peek_prev_char(size_t k) const
{
    // we pretend there are spaces before the beginning of the source code
    return src_cursor_ + 1 - k >= 2 ? src_[src_cursor_ - 2 + 1 - k] : ' ';
}

inline char Lexer::peek_next_char(size_t k) const
{
    // k = 1 by default
    // maybe special eof?
    // now '\0' is used
    return src_cursor_ - 1 + k < src_len_ ? src_[src_cursor_ - 1 + k] : '\0';
}

inline Token Lexer::get_next_token()
{
    // after running the previous get_next_token()
    // normally state_ == State::NONE
    // specially state_ == State::ERROR
    // specially state_ == State::END_OF_FILE

    //---------------------------------------------------------------
    // some error checks
    //---------------------------------------------------------------

    if (state_ == State::ERROR || state_ == State::END_OF_FILE)
    {
        return token_;
    }

    if (curr_char_ == '\0')
    {
        handle_eof_state();
        return token_;
    }

    if (!std::isspace(curr_char_) && curr_char_ != ',')
    {
        state_ = State::ERROR;
        error_flag_ = true;
        error_msg_ = "you miss some necassary whitespaces";
        token_ = Token();
        return token_;
    }

    //---------------------------------------------------------------
    // aggressively move forward until the first none whitespace
    //---------------------------------------------------------------

    while (std::isspace(curr_char_) || curr_char_ == ',')
    {
        move_to_next_char();
    }

    //---------------------------------------------------------------
    // doing actual things
    //---------------------------------------------------------------

    if (token_.get_token_value() == TokenValue::FROM)
    {
        // special, try reading "to"
        state_ = State::IDENTIFIER;
        handle_identifier_state();
    }
    else if (std::isalpha(curr_char_) || curr_char_ == '(' || curr_char_ == ')')
    {
        state_ = State::IDENTIFIER;
        handle_identifier_state();
    }
    else if (std::isdigit(curr_char_))
    {
        state_ = State::NUMBER;
        handle_number_state();
    }
    else if (curr_char_ == '\"' || curr_char_ == '\'')
    {
        state_ = State::STRING;
        handle_string_state(curr_char_);
    }
    else if (curr_char_ == '\0')
    {
        state_ = State::END_OF_FILE;
        handle_eof_state();
    }
    else
    {
        state_ = State::ERROR;
        error_flag_ = true;
        error_msg_ = "none meaningful input?";
        token_ = Token();
    }

    return token_;
}

inline void Lexer::handle_eof_state()
{
    token_ = Token("eof", TokenType::END_OF_FILE, TokenValue::END_OF_FILE);
    buffer_.clear();
    state_ = State::END_OF_FILE;
}

inline void Lexer::handle_identifier_state()
{
    // try to find the keywords inside the dictionary
    if (token_.get_token_value() == TokenValue::FROM)
    {
        // special case, from a to z
        // we treat "a to z" as a token as TokenValue::TO
        // pattern: a char + spaces + "to" + spaces + a char
        // tricky...
        char a = curr_char_;
        move_to_next_char();
        char s1 = curr_char_;
        while (std::isspace(curr_char_))
        {
            s1 = ' ';
            move_to_next_char();
        }
        char to_t = curr_char_;
        move_to_next_char();
        char to_o = curr_char_;
        move_to_next_char();
        char s2 = curr_char_;
        while (std::isspace(curr_char_))
        {
            s2 = ' ';
            move_to_next_char();
        }
        char z = curr_char_;
        if (((std::isalpha(a) && std::isalpha(z)) || (std::isdigit(a) && std::isdigit(z))) &&
            s1 == ' ' && std::tolower(to_t) == 't' && std::tolower(to_o) == 'o' && s2 == ' ')
        {

            buffer_.push_back(a);
            buffer_.push_back(z);
            token_ = Token(buffer_, TokenType::CHARACTER, TokenValue::TO);
            buffer_.clear();
            state_ = State::NONE;
            return;
        }
        else
        {
            // the "to" part is invalid, so we have a invalid token
            buffer_.clear();
            state_ = State::ERROR;
            error_flag_ = true;
            error_msg_ = "the \"to\" part is invalid";
            token_ = Token();
            return;
        }
    }

    bool found = false;
    while (buffer_.length() < dictionary_.get_key_max_length() && (std::isalpha(curr_char_) || curr_char_ == ' ') && !found)
    {
        buffer_.push_back(std::tolower(curr_char_));
        move_to_next_char();

        found = dictionary_.has_token(buffer_);

        if (found && dictionary_.token_is_prefix(buffer_))
        {
            size_t end = 0;
            string tmp_buffer = buffer_;
            for (size_t i = 0;
                 i < dictionary_.get_key_max_length() - buffer_.length() && peek_next_char(i) != '\0';
                 i++)
            {
                tmp_buffer.push_back(peek_next_char(i));
                if (dictionary_.has_token(tmp_buffer))
                {
                    end = i + 1; // so that end is the position exclusive
                }
            }
            for (size_t i = 0; i < end; i++)
            {
                buffer_.push_back(curr_char_);
                move_to_next_char();
            }
        }
    }

    if (found)
    {
        state_ = State::NONE;
        token_ = Token(buffer_,
                       dictionary_.get_token_type(buffer_),
                       dictionary_.get_token_value(buffer_));
    }
    else
    {
        state_ = State::ERROR;
        error_flag_ = true;
        error_msg_ = "we could not find any available identifier";
        token_ = Token();
    }
    buffer_.clear();
}

inline void Lexer::handle_number_state()
{
    do
    {
        buffer_.push_back(curr_char_); // eat the digits
        move_to_next_char();
    } while (std::isdigit(curr_char_)); // curr_char_ != '\0' &&

    token_ = Token(buffer_, TokenType::SRC_NUMBER, TokenValue::NUMBER);
    buffer_.clear();
    state_ = State::NONE;
}

inline void Lexer::handle_string_state(char string_state_delimiter)
{
    move_to_next_char(); // eat the left '\"'
    while (curr_char_ != '\0' && (curr_char_ != string_state_delimiter || peek_prev_char() == '\\'))
    {
        buffer_.push_back(curr_char_);
        move_to_next_char();
    }

    if (curr_char_ == '\0')
    {
        // then we have a trouble
        state_ = State::ERROR;
        error_flag_ = true;
        error_msg_ = "the string literal does not end correctly";
        token_ = Token();
    }
    else
    {
        move_to_next_char(); // eat the right '\"', curr_char_ is the one on the right of '\"'
        token_ = Token(buffer_, TokenType::SRC_STRING, TokenValue::STRING);
        state_ = State::NONE;
    }
    buffer_.clear();
}
}

#endif // !SIMPLEREGEXLANGUAGE_LEXER_H_
