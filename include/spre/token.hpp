/*
 * the ideas behind this file is heavily borrowed from
 * http://frozengene.github.io/blog/compiler/2014/08/10/compiler_tutorial_03/
 * that is BSD lincensed
 */

#ifndef SIMPLEREGEXLANGUAGE_TOKEN_H_
#define SIMPLEREGEXLANGUAGE_TOKEN_H_

#include <string>

using std::string;

namespace spre
{
	enum class TokenType
	{
		CHARACTER,
		QUANTIFIER,
		GROUP,
		LOOKAROUND,
		FLAG,
		ANCHOR,
		SRC_WHITESPECE,
		SRC_NUMBER,
		SRC_STRING,
		//COMMENT,
		DELIMITER,
		END_OF_FILE,
		UNDEFINED
	};

	enum class TokenValue
	{
		LITERALLY,
		ONE_OF,
		LETTER,
		UPPERCASE_LETTER,
		ANY_CHARACTER,
		NO_CHARACTER,
		DIGIT,
		ANYTHING,
		NEW_LINE,
		WHITESPACE,
		NO_WHITESPACE,
		TAB,
		RAW,
		FROM,
		TO,

		EXCATLY_X_TIMES,
		EXACTLY_ONE_TIME,
		ONCE,
		TWICE,
		BETWEEN_X_AND_Y_TIMES,
		OPTIONAL,
		ONCE_OR_MORE,
		NEVER_OR_MORE,
		AT_LEAST_X_TIMES,
		TIME,
		TIMES,
		AND,

		CAPTURE_AS,
		ANY_OF,
		UNTIL,
		AS,

		IF_FOLLOWED_BY,
		IF_NOT_FOLLOWED_BY,
		IF_ALREADY_HAD,
		IF_NOT_ALREADY_HAD,

		CASE_INSENSITIVE,
		MULTI_LINE,
		ALL_LAZY,
		
		BEGIN_WITH,
		STARTS_WITH,
		MUST_END,

		SPACE,
		NUMBER,
		STRING,
		GROUP_START,
		GROUP_END,

		END_OF_FILE,
		UNDEFINED
	};

	class Token
	{
	public:
		Token(string val = "undefined", 
			TokenType token_type = TokenType::UNDEFINED, 
			TokenValue token_value = TokenValue::UNDEFINED);
		~Token();
		string get_value() const;
		TokenType get_token_type() const;
		TokenValue get_token_value() const;
	private:
		string val_;
		TokenType token_type_;
		TokenValue token_value_;
	};

	Token::Token(string val, TokenType token_type, TokenValue token_value)
		: val_(val), token_type_(token_type), token_value_(token_value)
	{
	}

	Token::~Token()
	{
	}

	inline string Token::get_value() const
	{
		return val_;
	}

	inline TokenType Token::get_token_type() const
	{
		return token_type_;
	}

	inline TokenValue Token::get_token_value() const
	{
		return token_value_;
	}

}

#endif // !SIMPLEREGEXLANGUAGE_TOKEN_H_
