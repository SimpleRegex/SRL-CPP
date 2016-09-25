#include <string>
#include <iostream>
#include <tuple>
#include <vector>
#include "spre/spre.hpp"
#include "spre/token.hpp"
#include "spre/dictionary.hpp"

int main() {
	spre::Lexer lexer("letter \"haha\"");
	
	while (!lexer.has_ended())
	{
		std::cout << lexer.get_next_token().get_value() << std::endl;
	}
	std::cout << lexer.get_next_token().get_value() << std::endl;

    return 0;
}