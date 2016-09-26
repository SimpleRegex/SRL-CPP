#ifndef SIMPLEREGEXLANGUAGE_SPRE_H_
#define SIMPLEREGEXLANGUAGE_SPRE_H_

#include <string>
#include "spre/lexer.hpp"
#include "spre/parser.hpp"
#include "spre/generator.hpp"

using std::string;

namespace spre
{
class SRL
{
  public:
    explicit SRL(const string &src = "");
    ~SRL();
    string get_pattern() const;
  private:
    string result_;
};

SRL::SRL(const string &src)
{
    Lexer lexer(src);
    Parser parser(lexer);
    Generator generator(parser);
    result_ = generator.generate();
}

SRL::~SRL()
{
}

inline string SRL::get_pattern() const
{
    return result_;
}

class Builder
{
  public:
    Builder();
    ~Builder();

  private:
};

Builder::Builder()
{
}

Builder::~Builder()
{
}
}

#endif // !SIMPLEREGEXLANGUAGE_SPRE_H_
