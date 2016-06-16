#include <iostream>
#include <fstream>
#include <string>
#include <type_traits>
#include <cuchar>
std::wstring get_string() {
	std::wstring buf;
	std::wifstream ifs("test.txt");
	std::getline(ifs, buf);
	return buf;
}

namespace detail {
	constexpr char delem_impl(char) { return '$'; }
	constexpr wchar_t delem_impl(wchar_t) { return L'$'; }
	constexpr char16_t delem_impl(char16_t) { return u'$'; }
	constexpr char32_t delem_impl(char32_t) { return U'$'; }
	template<typename CharType>
	constexpr CharType delem() { return delem_impl(std::declval<CharType>()); }
	constexpr char zero_impl(char) { return '0'; }
	constexpr wchar_t zero_impl(wchar_t) { return L'0'; }
	constexpr char16_t zero_impl(char16_t) { return u'0'; }
	constexpr char32_t zero_impl(char32_t) { return U'0'; }
	template<typename CharType>
	constexpr CharType zero() { return zero_impl(std::declval<CharType>()); }

}

template<typename CharType, typename Container>
auto replace_regex(std::basic_string<CharType>& base, const Container& replace_list)
-> decltype(
	std::declval<Container>()[std::declval<std::size_t>()], //concept:[]演算子が使える
	std::size(std::declval<Container>()),//concept:sizeメンバー関数があるかC-style配列型
	void//戻り値の型
)
{
	if (0 == std::size(replace_list) || 10 < std::size(replace_list)) return base;//処理しない
	size_t current = 0;
	for (
		size_t found; 
		(found = base.find_first_of(detail::delem<CharType>(), current)) != std::basic_string<CharType>::npos && found + 1 < base.size(); 
		current = found + 1
	) {
		const int target_id = base[found + 1] - zero();
		if (target_id < 10 && target_id < std::size(replace_list)) {
			base.replace(found, 2, replace_list[target_id]);
		}
	}
}
int main()
{
	using namespace std::literals;
	std::string s = "arikitari na $1 string.";
	std::cout << s << std::endl;
	replace_regex(s, { std::string(), "aru"s });
	std::cout << s << std::endl;
}