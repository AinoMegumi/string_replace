#include <iostream>
#include <fstream>
#include <string>
#include <type_traits>
#include <cuchar>
#include <array>
namespace detail {
	template<typename CharType>	constexpr CharType zero();
	template<> constexpr char zero<char>() { return '0'; }
	template<> constexpr wchar_t zero<wchar_t>() { return L'0'; }
	template<> constexpr char16_t zero<char16_t>() { return u'0'; }
	template<> constexpr char32_t zero<char32_t>() { return U'0'; }
	template<typename CharType>	constexpr CharType delim();
	template<> constexpr char delim<char>() { return '$'; }
	template<> constexpr wchar_t delim<wchar_t>() { return L'$'; }
	template<> constexpr char16_t delim<char16_t>() { return u'$'; }
	template<> constexpr char32_t delim<char32_t>() { return U'$'; }
}
namespace type_traits {
	//C++メタ関数のまとめ - Qiita
	//http://qiita.com/_EnumHack/items/ee2141ad47915c55d9cb#%E9%96%A2%E6%95%B0%E3%81%AE%E3%82%AA%E3%83%BC%E3%83%90%E3%83%BC%E3%83%AD%E3%83%BC%E3%83%89%E8%A7%A3%E6%B1%BAsfinae%E3%82%92%E5%88%A9%E7%94%A8%E3%81%99%E3%82%8B
	namespace detail {
		template<typename Container>
		constexpr auto has_operator_subscript_impl(Container&& c) -> decltype(c.operator[](std::declval<std::size_t>()), std::true_type{}) { return{}; }
		template<typename Container>
		constexpr auto has_operator_subscript_impl(...)->std::false_type { return{}; }
	}
	template<typename Container>
	struct has_operator_subscript : public decltype(detail::has_operator_subscript_impl<Container>(std::declval<Container>())){};
	namespace detail {
		template<typename Container>
		constexpr auto has_member_function_size_impl(Container&& c) -> decltype(c.size(), std::true_type{}) { return{}; }
		template<typename Container>
		constexpr auto has_member_function_size_impl(...)->std::false_type { return{}; }
	}
	template<typename Container>
	struct has_member_function_size : public decltype(detail::has_member_function_size_impl<Container>(std::declval<Container>())){};
}

template<typename CharType, typename Container, std::enable_if_t<
	std::is_array<Container>::value || (type_traits::has_operator_subscript<Container>::value && type_traits::has_member_function_size<Container>::value),
	std::nullptr_t
> = nullptr>
void replace_regex(std::basic_string<CharType>& base, const Container& replace_list)
{
	if (0 == std::size(replace_list) || 10 < std::size(replace_list)) return;//処理しない
	for (
		size_t pos = 0, next_diff = 1; 
		std::basic_string<CharType>::npos != (pos = base.find_first_of(detail::delim<CharType>(), pos)) && pos + 1 < base.size();
		pos = pos + next_diff
	) {
		next_diff = 1;
		const int target_id = base[pos + 1] - detail::zero<CharType>();//文字コードで0から9が連続することは保証されている
		if (0 < target_id && target_id < 10 && static_cast<std::size_t>(target_id) < std::size(replace_list) && !replace_list[target_id].empty()) {
			base.replace(pos, 2, replace_list[target_id]);
			next_diff = replace_list[target_id].length();
		}
	}
}
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
	std::cout << test("arikitari na $1 string. $2"s, std::array<std::string, 2>{ { std::string(), "aru"s } }) << std::endl;
	std::wcout << test(L"arikitari na $1 string. $2"s, std::array<std::wstring, 2>{ { std::wstring(), L"aru"s } }) << std::endl;
}