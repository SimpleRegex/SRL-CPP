#ifndef SIMPLEREGEXLANGUAGE_GENERATOR_H_
#define SIMPLEREGEXLANGUAGE_GENERATOR_H_

#include "spre/parser.hpp"

namespace spre
{
class Generator
{
  public:
    Generator(Parser &parser);
    ~Generator();

  private:
    Parser parser_;
};

Generator::Generator(Parser &parser) : parser_(parser)
{
}

Generator::~Generator()
{
}
}

#endif // !SIMPLEREGEXLANGUAGE_GENERATOR_H_
