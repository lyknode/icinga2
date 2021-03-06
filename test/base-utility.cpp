/* Icinga 2 | (c) 2012 Icinga GmbH | GPLv2+ */

#include "base/utility.hpp"
#include <chrono>
#include <BoostTestTargetConfig.h>

#ifdef _WIN32
# include <windows.h>
# include <shellapi.h>
#endif /* _WIN32 */

using namespace icinga;

BOOST_AUTO_TEST_SUITE(base_utility)

BOOST_AUTO_TEST_CASE(parse_version)
{
	BOOST_CHECK(Utility::ParseVersion("2.11.0-0.rc1.1") == "2.11.0");
	BOOST_CHECK(Utility::ParseVersion("v2.10.5") == "2.10.5");
	BOOST_CHECK(Utility::ParseVersion("r2.11.1") == "2.11.1");
	BOOST_CHECK(Utility::ParseVersion("v2.11.0-rc1-58-g7c1f716da") == "2.11.0");

	BOOST_CHECK(Utility::ParseVersion("v2.11butactually3.0") == "v2.11butactually3.0");
}

BOOST_AUTO_TEST_CASE(compare_version)
{
	BOOST_CHECK(Utility::CompareVersion("2.10.5", Utility::ParseVersion("v2.10.4")) < 0);
	BOOST_CHECK(Utility::CompareVersion("2.11.0", Utility::ParseVersion("2.11.0-0")) == 0);
	BOOST_CHECK(Utility::CompareVersion("2.10.5", Utility::ParseVersion("2.11.0-0.rc1.1")) > 0);
}

BOOST_AUTO_TEST_CASE(comparepasswords_works)
{
	BOOST_CHECK(Utility::ComparePasswords("", ""));

	BOOST_CHECK(!Utility::ComparePasswords("x", ""));
	BOOST_CHECK(!Utility::ComparePasswords("", "x"));

	BOOST_CHECK(Utility::ComparePasswords("x", "x"));
	BOOST_CHECK(!Utility::ComparePasswords("x", "y"));

	BOOST_CHECK(Utility::ComparePasswords("abcd", "abcd"));
	BOOST_CHECK(!Utility::ComparePasswords("abc", "abcd"));
	BOOST_CHECK(!Utility::ComparePasswords("abcde", "abcd"));
}

BOOST_AUTO_TEST_CASE(comparepasswords_issafe)
{
	using std::chrono::duration_cast;
	using std::chrono::microseconds;
	using std::chrono::steady_clock;

	String a, b;

	a.Append(200000001, 'a');
	b.Append(200000002, 'b');

	auto start1 (steady_clock::now());

	Utility::ComparePasswords(a, a);

	auto duration1 (steady_clock::now() - start1);

	auto start2 (steady_clock::now());

	Utility::ComparePasswords(a, b);

	auto duration2 (steady_clock::now() - start2);

	double diff = (double)duration_cast<microseconds>(duration1).count() / (double)duration_cast<microseconds>(duration2).count();
	BOOST_WARN(0.9 <= diff && diff <= 1.1);
}

BOOST_AUTO_TEST_CASE(validateutf8)
{
	BOOST_CHECK(Utility::ValidateUTF8("") == "");
	BOOST_CHECK(Utility::ValidateUTF8("a") == "a");
	BOOST_CHECK(Utility::ValidateUTF8("\xC3") == "\xEF\xBF\xBD");
	BOOST_CHECK(Utility::ValidateUTF8("\xC3\xA4") == "\xC3\xA4");
}

BOOST_AUTO_TEST_CASE(EscapeCreateProcessArg)
{
#ifdef _WIN32
	using convert = std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>, wchar_t>;

	std::vector<std::string> testdata = {
		R"(foobar)",
		R"(foo bar)",
		R"(foo"bar)",
		R"("foo bar")",
		R"(" \" \\" \\\" \\\\")",
		R"( !"#$$%&'()*+,-./09:;<=>?@AZ[\]^_`az{|}~ " \" \\" \\\" \\\\")",
		"'foo\nbar'",
	};

	for (const auto& t : testdata) {
		// Prepend some fake exec name as the first argument is handled differently.
		std::string escaped = "some.exe " + Utility::EscapeCreateProcessArg(t);
		int argc;
		std::shared_ptr<LPWSTR> argv(CommandLineToArgvW(convert{}.from_bytes(escaped.c_str()).data(), &argc), LocalFree);
		BOOST_CHECK_MESSAGE(argv != nullptr, "CommandLineToArgvW() should not return nullptr for " << t);
		BOOST_CHECK_MESSAGE(argc == 2, "CommandLineToArgvW() should find 2 arguments for " << t);
		if (argc >= 2) {
			std::string unescaped = convert{}.to_bytes(argv.get()[1]);
			BOOST_CHECK_MESSAGE(unescaped == t,
				"CommandLineToArgvW() should return original value for " << t << " (got: " << unescaped << ")");
		}
	}
#endif /* _WIN32 */
}

BOOST_AUTO_TEST_SUITE_END()
