/*
* the ideas behind this file is heavily borrowed from
* http://llvm.org/docs/tutorial/LangImpl02.html
* that is UIUC (BSD-like) lincensed
*/

#ifndef SIMPLEREGEXLANGUAGE_AST_H_
#define SIMPLEREGEXLANGUAGE_AST_H_

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::unique_ptr;

namespace spre
{
class ExprAST
{
  public:
    virtual string get_val() const = 0;
    virtual ~ExprAST() = default;
};

class CharacterExprAST : public ExprAST
{
  public:
    CharacterExprAST(const string &val = "");
    string get_val() const override;

  private:
    const string val_;
};

CharacterExprAST::CharacterExprAST(const string &val) : val_(val)
{
}

inline string CharacterExprAST::get_val() const
{
    return val_;
}

class QuantifierExprAST : public ExprAST
{
  public:
    QuantifierExprAST(const string &val);
    string get_val() const override;

  private:
    const string val_;
};

QuantifierExprAST::QuantifierExprAST(const string &val) : val_(val)
{
}

inline string QuantifierExprAST::get_val() const
{
    return val_;
}

class GroupExprAST : public ExprAST
{
  public:
    GroupExprAST(vector<unique_ptr<ExprAST>> cond);
    GroupExprAST(vector<unique_ptr<ExprAST>> cond,
                 const string &name, vector<unique_ptr<ExprAST>> until_cond);
    void set_name(const string &name);
    void set_until_cond(vector<unique_ptr<ExprAST>> until_cond);
    string get_val() const override;

  private:
    vector<unique_ptr<ExprAST>> cond_;
    string name_;
    vector<unique_ptr<ExprAST>> until_cond_; // maybe useless
};

GroupExprAST::GroupExprAST(vector<unique_ptr<ExprAST>> cond)
    : cond_(std::move(cond))
{
}

GroupExprAST::GroupExprAST(vector<unique_ptr<ExprAST>> cond,
                           const string &name, vector<unique_ptr<ExprAST>> until_cond)
    : cond_(std::move(cond)), name_(name), until_cond_(std::move(until_cond))
{
}

inline void GroupExprAST::set_name(const string &name)
{
    name_ = name;
}

inline void GroupExprAST::set_until_cond(vector<unique_ptr<ExprAST>> until_cond)
{
    until_cond_ = std::move(until_cond);
}

inline string GroupExprAST::get_val() const
{
    if (cond_.size() != 0)
    {
        string res = "(";

        if (name_.size() != 0)
        {
            res.append("?<");
            res.append(name_);
            res.append(">");
        }

        for (auto const &iter : cond_)
        {
            res.append(iter->get_val());
        }
        res.append(")");
        return res;
    }

    // error!
    return "";
}

class LookAroundExprAST : public ExprAST
{
  public:
    LookAroundExprAST(const vector<string> vals,
                      vector<unique_ptr<ExprAST>> cond = vector<unique_ptr<ExprAST>>());
    string get_val() const override;

  private:
    const vector<string> vals_;
    vector<unique_ptr<ExprAST>> cond_;
};

LookAroundExprAST::LookAroundExprAST(const vector<string> vals,
                                     vector<unique_ptr<ExprAST>> cond)
    : vals_(std::move(vals)), cond_(std::move(cond))
{
}

inline string LookAroundExprAST::get_val() const
{
    if (vals_.size() == 2 && cond_.size() != 0)
    {
        string res = vals_[0];
        for (auto const &iter : cond_)
        {
            res.append(iter->get_val());
        }
        res.append(vals_[1]);
        return res;
    }

    // error!
    return "";
}

class FlagExprAST : public ExprAST
{
  public:
    FlagExprAST(const string &val);
    string get_val() const override;

  private:
    const string val_;
};

FlagExprAST::FlagExprAST(const string &val) : val_(val)
{
}

inline string FlagExprAST::get_val() const
{
    return val_;
}

class AnchorExprAST : public ExprAST
{
  public:
    AnchorExprAST(const string &val);
    string get_val() const override;

  private:
    const string val_;
};

AnchorExprAST::AnchorExprAST(const string &val) : val_(val)
{
}

inline string AnchorExprAST::get_val() const
{
    return val_;
}


class EOFExprAST : public ExprAST
{
public:
    EOFExprAST();
    string get_val() const override;

private:
};

EOFExprAST::EOFExprAST()
{
}

inline string EOFExprAST::get_val() const
{
    return "";
}

}


#endif // !SIMPLEREGEXLANGUAGE_AST_H_
