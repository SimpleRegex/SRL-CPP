#include <string>
#include <iostream>
#include "spre/spre.hpp"

int main() {
	spre::SRL srl("whitespace, literally \"haha\", digit");
    std::cout << "final result:\n" << srl.get_pattern() << std::endl;

    return 0;
}
