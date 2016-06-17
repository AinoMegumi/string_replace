# string_replace
[![Build Status](https://travis-ci.org/AinoMegumi/string_replace.svg?branch=master)](https://travis-ci.org/AinoMegumi/string_replace)

A simple regex-variable(``$0``～``$9``) replace library.

## compiler require

C++14 support is required(C++14 constexpr support is not required).

- Visual Studio 2015 Update 2
- Visual Studio 2015 Clang with Microsoft CodeGen(2016 May)(clang 3.8)
- msys2 mingw32 g++ 5.4.0
- msys2 mingw32 clang++ 3.8.0
- msys2 mingw64 g++ 5.4.0
- msys2 mingw64 clang++ 3.8.0
- linux g++ 5.4.0-3

``C++11 constexpr`` support and ``noexcept specifier`` support is required so that ``Visual Studio 2013`` is not supported.
## Usage

```cpp
using namespace std::literals;
std::string s1 = "arikitari na $1 string. $2";
replace_regex_variable(s1, std::array<std::string, 2>{ { std::string(), "aru"s } });//"arikitari na aru string. $2"
std::wstring s2 = L"arikitari na $1 string. $2";
replace_regex_variable(s2, { std::wstring(), L"aru"s });//L"arikitari na aru string. $2"
std::wstring s3 = L"arikitari na $1 string. $2";
replace_regex_variable(s3, { L"", L"aru", L"inserted string" });//L"arikitari na aru string. inserted string"
const char* list[] = { "", "aru" };
std::string s4 = "arikitari na $1 string. $2";
replace_regex_variable(s4, list);//"arikitari na aru string. $2"
```


