#ifndef SIMPLEREGEXLANGUAGE_GENERATOR_H_
#define SIMPLEREGEXLANGUAGE_GENERATOR_H_

#include "spre/ast.hpp"
#include "spre/parser.hpp"
#include <memory>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::unique_ptr;

namespace spre
{
class Generator
{
  public:
    explicit Generator(Parser &parser);
    ~Generator();
    string generate();

  private:
    Parser parser_;
};

Generator::Generator(Parser &parser) : parser_(parser)
{
}

Generator::~Generator()
{
}

inline string Generator::generate()
{
    string res;
    vector<unique_ptr<ExprAST>> h = parser_.parse();

    for (const auto &iter : h)
    {
        string k = iter->get_val();
        res.append(k);
    }

    return res;
}
}

#endif // !SIMPLEREGEXLANGUAGE_GENERATOR_H_
