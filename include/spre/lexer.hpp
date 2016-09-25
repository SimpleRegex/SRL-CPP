#ifndef SIMPLEREGEXLANGUAGE_LEXER_H_
#define SIMPLEREGEXLANGUAGE_LEXER_H_

#include <string>
#include <cctype>
#include "spre/dictionary.hpp"
#include "spre/token.hpp"

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
		bool has_ended() const;

		enum class State
		{
			NONE,
			END_OF_FILE,
			IDENTIFIER,
			NUMBER,
			STRING
		};

	private:
		const string src_;
		const size_t src_len_; // cache the length
		size_t src_cursor_; // cursor always points to next char
		char curr_char_; // always src_[src_cursor_ - 1] == curr_char_
		string buffer_; // one string object to eat the chars while necessary
		State state_;
		Token token_;
		Dictionary dictionary_;

		void move_to_next_char();
		char peek_prev_char() const;
		char peek_next_char() const;
		void handle_eof_state();
		void handle_identifier_state();
		void handle_number_state();
		void handle_string_state(char string_state_delimiter = '\"');
	};

	Lexer::Lexer(const string &src) :
		src_(src), src_len_(src.length()),
		src_cursor_(0), curr_char_(' '),
		token_(Token()),
		state_(State::NONE)
	{
	}

	Lexer::~Lexer()
	{
	}

	inline Token Lexer::get_token() const
	{
		return token_;
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
		return src_cursor_ >= src_len_;
	}

	inline void Lexer::move_to_next_char()
	{
		curr_char_ = src_cursor_ < src_len_ ? src_[src_cursor_] : '\0';
		src_cursor_ += 1; // the position next to that of curr_char_
	}

	inline char Lexer::peek_prev_char() const
	{
		// we pretend there are spaces before the beginning of the source code
		return src_cursor_ >= 2 ? src_[src_cursor_ - 2] : ' ';
	}

	inline char Lexer::peek_next_char() const
	{
		// maybe special eof?
		// now '\0' is used
		return src_cursor_ < src_len_ ? src_[src_cursor_] : '\0';
	}

	inline Token Lexer::get_next_token()
	{
		
		bool is_matched = false;
		char string_state_delimiter = '\"';

		do
		{
			if (state_ != State::NONE)
			{
				is_matched = true;
			}

			switch (state_)
			{
			case State::NONE:
				move_to_next_char();
				break;
			case State::END_OF_FILE:
				handle_eof_state();
				break;
			case State::IDENTIFIER:
				handle_identifier_state();
				break;
			case State::NUMBER:
				handle_number_state();
				break;
			case State::STRING:
				handle_string_state(string_state_delimiter);
				break;
			default:
				break;
			}

			if (state_ == State::NONE)
			{
				if (curr_char_ == '\0')
				{
					state_ = State::END_OF_FILE;
				}
				else if (std::isalpha(curr_char_) || curr_char_ == '(' || curr_char_ == ')')
				{
					state_ = State::IDENTIFIER;
				}
				else if (std::isdigit(curr_char_))
				{
					state_ = State::NUMBER;
				}
				else if (curr_char_ == '\"' || curr_char_ == '\'')
				{
					state_ = State::STRING;
					string_state_delimiter = curr_char_;
				}
				else if (std::isspace(curr_char_) || curr_char_ == ',')
				{
					//state_ = State::NONE;
				}
			}

		} while (!is_matched);

		return token_;
	}

	inline void Lexer::handle_eof_state()
	{
		token_ = Token("eof", TokenType::END_OF_FILE, TokenValue::END_OF_FILE);
		buffer_.clear();
	}

	inline void Lexer::handle_identifier_state()
	{
		// try to find the keywords inside the dictionary
		bool found = false;
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
			if (
				(
					(std::isalpha(a) && std::isalpha(z)) 
					|| (std::isdigit(a) && std::isdigit(z))
				)
				&& s1 == ' '
				&& to_t == 't'
				&& to_o == 'o'
				&& s2 == ' '
			)
			{
				buffer_.push_back(a);
				buffer_.push_back(z);
				token_ = Token(buffer_, TokenType::CHARACTER, TokenValue::TO);
				found = true;
			}
			/*
			do
			{
				if (!std::isalpha(curr_char_) && !std::isdigit(curr_char_))
				{
					break;
				}
				buffer_.push_back(curr_char_);
				move_to_next_char();

				if (!std::isspace(curr_char_))
				{
					break;
				}
				while (std::isspace(curr_char_))
				{
					move_to_next_char();
				}

				if (curr_char_ != 't')
				{
					break;
				}
				move_to_next_char();
				if (curr_char_ != 'o')
				{
					break;
				}
				move_to_next_char();

				if (!std::isspace(curr_char_))
				{
					break;
				}
				while (std::isspace(curr_char_))
				{
					move_to_next_char();
				}

				if (!std::isalpha(curr_char_) && !std::isdigit(curr_char_))
				{
					break;
				}
				buffer_.push_back(curr_char_);
				token_ = Token(buffer_, TokenType::CHARACTER, TokenValue::TO);
				found = true;
			} while (false);
			*/
		}
		else
		{
			do
			{
				buffer_.push_back(std::tolower(curr_char_)); // because it's case insensitive
				move_to_next_char();

				// TODO: something is the prefix of something
				// exactly, exactly 1 time, exactly 1 times
				// once, once or more
				found = dictionary_.has_token(buffer_);
			} while (
				!found
				&&
				(
					std::isalpha(curr_char_)
					|| (curr_char_ == ' ' && peek_next_char() != ' ') // or std::isspace()
					|| buffer_.length() <= dictionary_.get_key_max_length()
				)
			);

			if (found)
			{
				token_ = Token(buffer_,
					dictionary_.get_token_type(buffer_),
					dictionary_.get_token_value(buffer_));
			}
		}
		// the identifiers / keys in srl 
		// are all alpha or at most one ' ' between words
		// no line breaks allowed here
		// and they cannot exceed the max length of all the keys

		if (!found)
		{
			// then we have trouble, we could not find any available identifier
			// TODO
			token_ = Token();
		}

		buffer_.clear();
		state_ = State::NONE;
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
		while (curr_char_ != '\0' 
			&& (curr_char_ != string_state_delimiter || peek_prev_char() == '\\'))
		{
			buffer_.push_back(curr_char_);
			move_to_next_char();
		}

		if (curr_char_ == '\0')
		{
			// then we have a trouble, the string literal does not end correctly
			// TODO
			token_ = Token();
		}
		else
		{
			move_to_next_char(); // eat the right '\"'
			token_ = Token(buffer_, TokenType::SRC_STRING, TokenValue::STRING);
		}

		buffer_.clear();
		state_ = State::NONE;
	}

}

#endif // !SIMPLEREGEXLANGUAGE_LEXER_H_
