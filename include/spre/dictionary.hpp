/*
* the ideas behind this file is heavily borrowed from
* http://frozengene.github.io/blog/compiler/2014/08/10/compiler_tutorial_03/
* that is BSD lincensed
*/

#ifndef SIMPLEREGEXLANGUAGE_DICTIONARY_H_
#define SIMPLEREGEXLANGUAGE_DICTIONARY_H_

#include "spre/token.hpp"
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

using std::tuple;
using std::make_tuple;
using std::unordered_set;
using std::unordered_map;
using std::string;

namespace spre
{
using MetaType = tuple<TokenType, TokenValue>;

class Dictionary
{
  public:
    Dictionary();
    ~Dictionary();
    bool has_token(const string &name) const;
    bool token_is_prefix(const string &name) const;
    size_t get_key_max_length() const;
    MetaType get(const string &name) const;
    TokenType get_token_type(const string &name) const;
    TokenValue get_token_value(const string &name) const;

  private:
    unordered_map<string, MetaType> dictionary_;
    unordered_set<string> prefix_;
    size_t key_max_len_; // cache the max length of all the keys!
};

Dictionary::Dictionary() : dictionary_{
                               {"literally",
                                make_tuple(TokenType::CHARACTER, TokenValue::LITERALLY)},
                               {"one of",
                                make_tuple(TokenType::CHARACTER, TokenValue::ONE_OF)},
                               {"letter",
                                make_tuple(TokenType::CHARACTER, TokenValue::LETTER)},
                               {"uppercase letter",
                                make_tuple(TokenType::CHARACTER, TokenValue::UPPERCASE_LETTER)},
                               {"any character",
                                make_tuple(TokenType::CHARACTER, TokenValue::ANY_CHARACTER)},
                               {"no character",
                                make_tuple(TokenType::CHARACTER, TokenValue::NO_CHARACTER)},
                               {"digit",
                                make_tuple(TokenType::CHARACTER, TokenValue::DIGIT)},
                               {"anything",
                                make_tuple(TokenType::CHARACTER, TokenValue::ANYTHING)},
                               {"new line",
                                make_tuple(TokenType::CHARACTER, TokenValue::NEW_LINE)},
                               {"whitespace",
                                make_tuple(TokenType::CHARACTER, TokenValue::WHITESPACE)},
                               {"no whitespace",
                                make_tuple(TokenType::CHARACTER, TokenValue::NO_WHITESPACE)},
                               {"tab",
                                make_tuple(TokenType::CHARACTER, TokenValue::TAB)},
                               {"raw",
                                make_tuple(TokenType::CHARACTER, TokenValue::RAW)},
                               {"from",
                                make_tuple(TokenType::CHARACTER, TokenValue::FROM)},
                               {"to",
                                make_tuple(TokenType::CHARACTER, TokenValue::TO)},

                               {"exactly",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::EXCATLY_X_TIMES)},
                               {"exactly 1 time",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::EXACTLY_ONE_TIME)},
                               {"once",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::ONCE)},
                               {"twice",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::TWICE)},
                               {"between",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::BETWEEN_X_AND_Y_TIMES)},
                               {"optional",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::OPTIONAL)},
                               {"once or more",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::ONCE_OR_MORE)},
                               {"never or more",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::NEVER_OR_MORE)},
                               {"at least",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::AT_LEAST_X_TIMES)},
                               {"time",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::TIME)},
                               {"times",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::TIMES)},
                               {"and",
                                make_tuple(TokenType::QUANTIFIER, TokenValue::AND)},

                               {"capture",
                                make_tuple(TokenType::GROUP, TokenValue::CAPTURE_AS)},
                               {"any of",
                                make_tuple(TokenType::GROUP, TokenValue::ANY_OF)},
                               {"until",
                                make_tuple(TokenType::GROUP, TokenValue::UNTIL)},
                               {"as",
                                make_tuple(TokenType::GROUP, TokenValue::AS)},

                               {"if followed by",
                                make_tuple(TokenType::LOOKAROUND, TokenValue::IF_FOLLOWED_BY)},
                               {"if not followed by",
                                make_tuple(TokenType::LOOKAROUND, TokenValue::IF_NOT_FOLLOWED_BY)},
                               {"if already had",
                                make_tuple(TokenType::LOOKAROUND, TokenValue::IF_ALREADY_HAD)},
                               {"if not already had",
                                make_tuple(TokenType::LOOKAROUND, TokenValue::IF_NOT_ALREADY_HAD)},

                               {"case insensitive",
                                make_tuple(TokenType::FLAG, TokenValue::CASE_INSENSITIVE)},
                               {"multi line",
                                make_tuple(TokenType::FLAG, TokenValue::MULTI_LINE)},
                               {"all lazy",
                                make_tuple(TokenType::FLAG, TokenValue::ALL_LAZY)},

                               {"begin with",
                                make_tuple(TokenType::ANCHOR, TokenValue::BEGIN_WITH)},
                               {"starts with",
                                make_tuple(TokenType::ANCHOR, TokenValue::STARTS_WITH)},
                               {"must end",
                                make_tuple(TokenType::ANCHOR, TokenValue::MUST_END)},

                               {",",
                                make_tuple(TokenType::SRC_WHITESPECE, TokenValue::SPACE)},
                               {" ",
                                make_tuple(TokenType::SRC_WHITESPECE, TokenValue::SPACE)},
                               {"\n",
                                make_tuple(TokenType::SRC_WHITESPECE, TokenValue::SPACE)},

                               {"\"",
                                make_tuple(TokenType::DELIMITER, TokenValue::STRING)},
                               {"\'",
                                make_tuple(TokenType::DELIMITER, TokenValue::STRING)},
                               {"(",
                                make_tuple(TokenType::DELIMITER, TokenValue::GROUP_START)},
                               {")",
                                make_tuple(TokenType::DELIMITER, TokenValue::GROUP_END)}},
                           prefix_{"exactly", "once", "time", "times"}, key_max_len_(0)
{
    for (auto const &iter : dictionary_)
    {
        if (key_max_len_ < (iter.first).length())
        {
            key_max_len_ = (iter.first).length();
        }
    }
}

Dictionary::~Dictionary()
{
}

inline size_t Dictionary::get_key_max_length() const
{
    return key_max_len_;
}

inline bool Dictionary::has_token(const string &name) const
{
    auto iter = dictionary_.find(name);
    return iter != dictionary_.end();
}

inline bool Dictionary::token_is_prefix(const string &name) const
{
    auto iter = prefix_.find(name);
    return iter != prefix_.end();
}

inline MetaType Dictionary::get(const string &name) const
{
    if (!has_token(name))
    {
        return make_tuple(TokenType::UNDEFINED, TokenValue::UNDEFINED);
    }
    return dictionary_.at(name);
}

inline TokenType Dictionary::get_token_type(const string &name) const
{
    return std::get<0>(get(name));
}

inline TokenValue Dictionary::get_token_value(const string &name) const
{
    return std::get<1>(get(name));
}
}

#endif // !SIMPLEREGEXLANGUAGE_DICTIONARY_H_
