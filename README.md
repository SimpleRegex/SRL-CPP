# SRL-CPP

This is a header-only C++ libary for Simple Regex Language.

**Currently under development.**

## Usage

You only need to include the library directory and use C++ 14 standard to compile. The namespace used is `spre::`.

The library uses C++ 11 features heavily, and uses `make_unique` from C++ 14. You could use latest Visual Studio, or `g++-4.9` or later, or `clang++-3.8` or later. The project uses `cmake` as the builing system. 

```cpp
// test.cpp
#include <string>
#include <iostream>
#include <spre/spre.hpp>
int main()
{
    std::string src = "literally \"something\"";
    spre::SRL srl(src);
    std::cout << srl.get_pattern() << std::endl;
    return 0;
}
```

```bash
$ tree .
.
|-- test.cpp
`-- include
    `-- spre
        |-- ast.hpp
        |-- ...
        ...

$ g++ -I./include -std=c++14 test.cpp
$ ./test
(?:something)
```

The project uses `cmake` as the builing system. It's especially useful for development in Visual Studio in Windows.

## License

MIT.

## Limitations and TODOs

- The `Builder` is yet to be implemented.
- The error reports are implemented as outputing to `stderr`.

## Technical Structures

First of all, it's designed to be a header-only library. Thus everything are written in the header files (`.hpp`). Ideally users only need to `#include <spre/spre.hpp>`.

The library is written as a light-weight compiler-like thing, although SRL is a DSL and does not have control flow (as a subset of Regex) thus could not be considered turing-complete. As a result, this library has lexer and parser and code generator. This library has specific lexer instead of using `yacc`. The code is written following the tutorials from [llvm](http://llvm.org/docs/tutorial/LangImpl02.html) and [@](http://frozengene.github.io/blog/compiler/2014/08/10/compiler_tutorial_03/).

The structure:

```txt

token.hpp
  |
  V
dictionary.hpp         ast.hpp
  |                       |  
  V                       V
lexer.hpp  ---------> parser.hpp --------> generator.hpp
(get tokens)  (get (vector of) asts) (get the compiled regex string)
                                             |
                                             V
                                          spre.hpp
                                        (`SRL` and `Builder`)
```

