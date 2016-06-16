#include "string_replace.hpp"
#include <array>
#include <iostream>
namespace detail {
	template<typename CharType, typename Container>
	struct test_info {
		std::basic_string<CharType> str;
		Container&& list;
		test_info() = default;
		test_info(const test_info&) = delete;
		test_info(test_info&&) = delete;
		test_info& operator=(const test_info&) = delete;
		test_info& operator=(test_info&&) = delete;
	};
	template<typename CharType, typename Container>
	std::basic_ostream<CharType>& operator<< (std::basic_ostream<CharType>& os, test_info<CharType, Container>&& info) {
		os << info.str << std::endl;
		replace_regex(info.str, info.list);
		os << info.str << std::endl;
		return os;
	}
}
template<typename CharType, typename Container>
detail::test_info<CharType, Container> test(std::basic_string<CharType> str, Container&& c) { return{ std::move(str), std::forward<Container>(c) }; }
int main()
{
	using namespace std::literals;
	std::wcout.imbue(std::locale(""));
	std::cout << test("arikitari na $1 string. $2"s, std::array<std::string, 2>{ { std::string(), "aru"s } }) << std::endl;
	std::wcout << test(L"arikitari na $1 string. $2"s, std::array<std::wstring, 2>{ { std::wstring(), L"aru"s } }) << std::endl;
	std::wcout << test(L"$1機能"s, std::array<std::wstring, 2>{ { std::wstring(), L"岡山の陶芸家を用なしにする"s } }) << std::endl;
	std::wcout << test(L"$1機能"s, std::array<std::wstring, 2>{ { std::wstring(), L"でちまるさんの兄にとどめを刺す"s } }) << std::endl;
}