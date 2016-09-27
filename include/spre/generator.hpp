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
    explicit Generator(Parser &parser, bool show_error = true);
    ~Generator();
    bool has_error() const;
    void report_error() const;
    string generate();

  private:
    Parser parser_;
    bool error_flag_;
    string error_msg_;
    const bool show_error_;
};

Generator::Generator(Parser &parser, bool show_error) : parser_(parser), show_error_(show_error)
{
}

Generator::~Generator()
{
}

inline bool Generator::has_error() const
{
    return error_flag_;
}

inline void Generator::report_error() const
{
    if (!has_error())
    {
        return;
    }
    fprintf(stderr, "generator error: ");
    fprintf(stderr, "%s", error_msg_.c_str());
    fprintf(stderr, "\n");
}

inline string Generator::generate()
{
    string res;
    vector<unique_ptr<ExprAST>> h = parser_.parse();
    std::cout << "asts length: " << h.size() << "\n";
    for (const auto &iter : h)
    {
        string k = iter == nullptr ? "nullptr" : iter->get_val();
        res.append(k);
    }

    return res;
}
}

#endif // !SIMPLEREGEXLANGUAGE_GENERATOR_H_
