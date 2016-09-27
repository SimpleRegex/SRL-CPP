#include <string>
#include <iostream>
#include "spre/spre.hpp"

int main() {
    string src = "literally \"haha\", capture(capture(digit from a to z whitespace) as \"inner\") as \"outer\"";
    std::cout << "original string:\n" << src << std::endl;
	spre::SRL srl(src);
    std::cout << "final result:\n" << srl.get_pattern() << std::endl;

    return 0;
}
