minarg
======

A minimalist argument parsing library for C++11.
Implemented in a single lightweight header and
shared under the hassle-free [Boost license][boost].

- [Overview](#overview)
- [Example](#example)
- [Interface](#interface)
- [Exceptions](#exceptions)
- [Help](#help)
- [Install](#install)


Overview
--------

The parser expects the [standard sequence of arguments][posix],
starting with the utility name, followed by options,
followed by positional operands.

The following option formats are supported:

```
-a
-bVALUE
-b VALUE
-ab VALUE
--long
--long VALUE
--long=VALUE
```

Option values and operands are templated.
Any type that supports the `>>` stream operator can be parsed,
and the result is assigned to a target variable of that type.

The terminator `--` enforces the end of options.

The prefixes, separator, and terminator can be customized.

The parser can print a formatted help message.


Example
-------

Consider this help message:

```
This is an example

USAGE
  ./example [-h] [-q] [--skip LINES] DIR [FILE]...

OPTIONS
  -h, --help        Show help and exit
  -q, --quiet       Hide progress info
      --skip LINES  Lines to skip (default: 5)

OPERANDS
  DIR   Target directory
  FILE  Input file

```

The following code parses the corresponding arguments
and prints the above help message when requested:

```cpp
#include <cstdlib>
#include <iostream>
#include <minarg/minarg.hpp>

int main(int argc, char* argv[])
{
  // Prepare target variables
  bool q{false};
  int s{5};
  std::string d{};
  std::vector<std::string> f{};

  // Setup parser
  minarg::Parser parser{"This is an example"};

  parser.addSignal(   'h', "help" ,          "Show help and exit");
  parser.addOption(q, 'q', "quiet",          "Hide progress info");
  parser.addOption(s,  0 , "skip" , "LINES", "Lines to skip");

  parser.addOperand(d, "DIR", "Target directory", true);
  parser.addOperandSink(f, "FILE", "Input file");

  // Parse arguments
  try {
    parser.parse(argc, argv);
  }
  catch (const minarg::Signal&) {
    std::cout << parser;
    return EXIT_SUCCESS;
  }
  catch (const minarg::Error& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Parsing was successful
  std::cout << "quiet:" << q << " skip:" << s << " dir:" << d << " files:";
  for (const auto& file : f)
    std::cout << file << ' ';
  std::cout << std::endl;
}
```


Interface
---------

All functionality is provided by the `minarg::Parser` class.
It has one constructor:

```cpp
Parser(std::string helpProlog={}, std::string helpEpilog={})
```

Arguments can be added with the following `add*` functions.
Omit short option names with `0`, long option names with the empty string.

Template type `T` must support the `>>` and `<<` [stream operators][cppStream].\
Template type `Container<T>` must support `push_back(T)`.

Any type `T` that satisfies the `std::is_integral` trait
is parsed as an integer. This includes all `char` types.
Integer notations can be decimal, or hexadecimal with the `0x` prefix.

```cpp
// Add option that throws minarg::Signal
addSignal(
  char shortName,
  std::string longName,
  std::string description)

// Add option without value
addOption(
  bool& target,
  char shortName,
  std::string longName,
  std::string description,
  bool isRequired = false)

// Add option with value
addOption(
  T& target,
  char shortName,
  std::string longName,
  std::string valueName,
  std::string description,
  bool isRequired = false)

// Add positional operand
addOperand(
  T& target,
  std::string valueName,
  std::string description,
  bool isRequired = false)

// Add container for all remaining operands
addOperandSink(
  Container<T>& target,
  std::string valueName,
  std::string description,
  bool isRequired = false)
```

Once all options and operands have been added, the
arguments from the [main function][cppMain] can be parsed with:

```cpp
parse(int argc, const char* const argv[])
parse(const std::vector<std::string>& argv)
```

The syntax can be customized. If the short and long
prefixes are identical, long options take precedence:

```cpp
setShortOptionPrefix(char)       // Default: '-'
setLongOptionPrefix(std::string) // Default: "--"
setLongOptionSeparator(char)     // Default: '='
setOptionTerminator(std::string) // Default: "--"
```

Exceptions
----------

The `parser.parse()` functions throw two custom exception types.
When an argument is invalid or missing, they throw `minarg::Error`:

```cpp
struct Error : public std::exception
{
  const std::string message; // Also returned by what()
  // ...
};
```

When a signal option is found, they throw `minarg::Signal`:

```cpp
struct Signal : public std::exception
{
  const char shortName;
  const std::string longName;
  // ...
};
```

When an exception is thrown, the parsing remains unfinished,
and the target variables should be considered invalid.


Help
----

Use `std::ostream << parser` to print a formatted help message.

The parser implements simple line wrapping. Strings are split
at ASCII spaces (`' '`), or explicit ASCII newlines (`'\n'`).

String length is measured per char, not per Unicode code-point.
UTF-8 encoded strings remain valid, but multi-byte characters
might cause premature line breaks or misaligned columns.

Unreasonably long words will break the formatting.

The help message can be customized with these `Parser` functions:

```cpp
// Change the section titles,
// or disable a section with the empty string
setUsageTitle(std::string)    // Default: "USAGE"
setOptionsTitle(std::string)  // Default: "OPTIONS"
setOperandsTitle(std::string) // Default: "OPERANDS"

// Change the usage line
setUtilityName(std::string)   // Default: "" (use argv[0])
setOptionsUsage(std::string)  // Default: "" (list all options)
setOperandsUsage(std::string) // Default: "" (list all operands)

// Change the text written before default values,
// or disable default values with the empty string
setDefaultValueIntro(std::string) // Default: "default: "

// Change measurements
setHelpWidth(std::string::size_type)  // Default: 80
setHelpIndent(std::string::size_type) // Default: 2
```


Install
-------

This is a header-only library.
Simply add the header from the `include` directory to your project.

The unit tests can be compiled and run with:

```
mkdir build
cd build
cmake ..
make
./test/minarg-test
```


[boost]: https://www.boost.org/users/license.html
[posix]: http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
[cppMain]: https://en.cppreference.com/w/cpp/language/main_function
[cppStream]: https://en.cppreference.com/w/cpp/language/operators#Stream_extraction_and_insertion
