#include "string_replace.hpp"
#include <array>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <locale>
#if defined(_WIN32) && !defined(__MINGW32__)
#include <windows.h>
#include <cstring>
#endif
template<std::codecvt_mode mode = static_cast<std::codecvt_mode>(0)>
using codecvt_wchar_t = std::conditional_t<2 == sizeof(wchar_t), std::codecvt_utf8_utf16<wchar_t, 0x10ffff, mode>, std::codecvt_utf8<wchar_t, 0x10ffff, mode>>;
static_assert(2 == sizeof(wchar_t) || 4 == sizeof(wchar_t), "unexpected wchar_t");

namespace detail {
	template<typename ...Args>
	struct first_enabled {};

	template<typename T, typename ...Args>
	struct first_enabled<std::enable_if<true, T>, Args...> { using type = T; };
	template<typename T, typename ...Args>
	struct first_enabled<std::enable_if<false, T>, Args...> : first_enabled<Args...> {};
	template<typename T, typename ...Args>
	struct first_enabled<T, Args...> { using type = T; };

	template<typename ...Args>
	using first_enabled_t = typename first_enabled<Args...>::type;

	template<typename CharType, std::codecvt_mode mode = static_cast<std::codecvt_mode>(0)>
	using codecvt_t = first_enabled_t<
		std::enable_if<2 == sizeof(CharType), std::codecvt_utf8_utf16<CharType, 0x10ffff, mode>>,
		std::enable_if<4 == sizeof(CharType), std::codecvt_utf8<CharType, 0x10ffff, mode>>
	>;
	static_assert(2 == sizeof(wchar_t) || 4 == sizeof(wchar_t), "unexpected wchar_t");
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
		using std::endl;
		os << info.str << endl;
		replace_regex_variable(info.str, info.list);
		os << info.str << endl;
		return os;
	}
#if defined(_WIN32) && !defined(__MINGW32__)
	std::string utf_16_to_shift_jis(const std::wstring& str) {
		static_assert(sizeof(wchar_t) == 2, "this functon is windows only");
		const int len = ::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string re(len * 2, '\0');
		if (!::WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, &re[0], len, nullptr, nullptr)) {
			const auto ec = ::GetLastError();
			switch (ec)
			{
				case ERROR_INSUFFICIENT_BUFFER:
					throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INSUFFICIENT_BUFFER"); break;
				case ERROR_INVALID_FLAGS:
					throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INVALID_FLAGS"); break;
				case ERROR_INVALID_PARAMETER:
					throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: ERROR_INVALID_PARAMETER"); break;
				default:
					throw std::runtime_error("in function utf_16_to_shift_jis, WideCharToMultiByte fail. cause: unknown(" + std::to_string(ec) + ')'); break;
			}
		}
		const std::size_t real_len = std::strlen(re.c_str());
		re.resize(real_len);
		re.shrink_to_fit();
		return re;
	}
	template<typename CharType>
	std::string to_narrow_multibyte_helper(const std::basic_string<CharType>& s) {
		static std::wstring_convert<codecvt_t<CharType>, CharType> cv1;
		static std::wstring_convert<codecvt_wchar_t<>, wchar_t> cv2;
		return utf_16_to_shift_jis(cv2.from_bytes(cv1.to_bytes(s)));
	}
	std::string to_narrow_multibyte_helper(const std::wstring& s) {
		return utf_16_to_shift_jis(s);
	}
#endif
	template<typename CharType>
	std::string to_narrow_multibyte(const std::basic_string<CharType>& s) {
#if defined(_WIN32) && !defined(__MINGW32__)
		return to_narrow_multibyte_helper(s);
#else
		static std::wstring_convert<codecvt_t<CharType>, CharType> cv1;
		return cv1.to_bytes(s);
#endif
	}
	template<typename CharType, typename Container, std::enable_if_t<!std::is_same<char, CharType>::value, std::nullptr_t> = nullptr>
	std::ostream& operator<< (std::ostream& os, test_info<CharType, Container>&& info) {
		using std::endl;
		os << to_narrow_multibyte(info.str) << endl;
		replace_regex_variable(info.str, info.list);
		os << to_narrow_multibyte(info.str) << endl;
		return os;
	}

}
template<typename CharType, typename Container>
detail::test_info<CharType, Container> test(std::basic_string<CharType> str, Container&& c) { return{ std::move(str), std::forward<Container>(c) }; }
int main(int argc, char* argv[])
{
	using namespace std::literals;
	using std::cout;
	using std::endl;
	try {
		cout << test("arikitari na $1 string. $2"s, std::array<std::string, 2>{ { std::string(), "aru"s } }) << endl;
		cout << test(L"arikitari na $1 string. $2"s, std::array<std::wstring, 2>{ { std::wstring(), L"aru"s } }) << endl;
		cout << test(L"$1機能"s, std::array<std::wstring, 2>{ { std::wstring(), L"岡山の陶芸家を用なしにする"s } }) << endl;
		const wchar_t* list[] = { L"", L"でちまるさんの兄にとどめを刺す" };
		cout << test(L"$1機能"s, list) << endl;
		if (2 != argc) return 0;
		std::wstring buf;
		std::wifstream in(argv[1]);
#ifdef __MINGW32__
		in.imbue(std::locale(std::locale("C"), new codecvt_wchar_t<std::consume_header>()));//bom skip:broken!!
#else
		in.imbue(std::locale(std::locale(""), new codecvt_wchar_t<std::consume_header>()));//bom skip
#endif
		std::getline(in, buf);
		cout << test(buf, std::array<std::wstring, 2>{ { std::wstring(), L"明日"s } }) << endl;
	}
	catch (const std::exception& er) {
		std::cerr << "exception:" << er.what() << endl;
	}
}