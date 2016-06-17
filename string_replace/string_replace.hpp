#pragma once
#include <string>
#include <type_traits>
#include <iterator>
#ifndef __MINGW32__
#include <cuchar>
#endif
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
	template <class C>
	constexpr auto size(const C& c) -> decltype(c.size())
	{
		return c.size();
	}
	template <class T, std::size_t N>
	constexpr std::size_t size(const T(&)[N]) noexcept
	{
		return N;
	}
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
	//!\~english	@brief meta tmeplate to check the type has member function 'operator[]'
	//!\~japanese	@brief 'operatpr[]'メンバー関数をもつか調べるメタtemplate
	template<typename Container>
	struct has_operator_subscript : public decltype(detail::has_operator_subscript_impl<Container>(std::declval<Container>())){};
	namespace detail {
		template<typename Container>
		constexpr auto has_member_function_size_impl(Container&& c) -> decltype(c.size(), std::true_type{}) { return{}; }
		template<typename Container>
		constexpr auto has_member_function_size_impl(...)->std::false_type { return{}; }
	}
	//!\~english	@brief meta tmeplate to check the type has member function 'size()'
	//!\~japanese	@brief 'size()'メンバー関数をもつか調べるメタtemplate
	template<typename Container>
	struct has_member_function_size : public decltype(detail::has_member_function_size_impl<Container>(std::declval<Container>())){};
}

//!\~english	@brief replace $0-$9 in 1st argument to 2nd argument's element.
//!\~japanese	@brief 第一引数中の$0～$9を第二引数の配列の要素で置換する
//!\~english	@param base[in, out] target string
//!\~japanese	@param base[in, out] 対象文字列
//!\~english	@param replace_list[in] replace string list(element type must be std::basic_string)
//!\~japanese	@param replace_list[in] 置換文字列群(要素型はstd::basic_stringである必要があります)
template<typename CharType, typename Container, std::enable_if_t<
	//Require concept for 'Container':
	//  1. operator[]がつかえる(operator[]をメンバーに持つか(フリー関数としては定義できない)ポインタに暗黙変換可能(=C形式の配列))
	//  2. std::size()(C++17 or later)が呼べる(=C形式の配列であるかsize()をメンバー関数に持つ)
	std::is_array<Container>::value || (type_traits::has_operator_subscript<Container>::value && type_traits::has_member_function_size<Container>::value),
	std::nullptr_t
> = nullptr>
void replace_regex(std::basic_string<CharType>& base, const Container& replace_list)
{
	if (0 == detail::size(replace_list) || 10 < detail::size(replace_list)) return;//処理しない
	for (
		size_t pos = 0, next_diff = 1;
		std::basic_string<CharType>::npos != (pos = base.find_first_of(detail::delim<CharType>(), pos)) && pos + 1 < base.size();
		pos = pos + next_diff
		) {
		next_diff = 1;
		const int target_id = base[pos + 1] - detail::zero<CharType>();//文字コードで0から9が連続することは保証されている
		if (0 < target_id && target_id < 10 && static_cast<std::size_t>(target_id) < detail::size(replace_list) && !replace_list[target_id].empty()) {
			base.replace(pos, 2, replace_list[target_id]);
			next_diff = replace_list[target_id].length();//文字列の長さ分skipする
		}
	}
}
