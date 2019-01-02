#include <catch2/catch.hpp>

#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include <minarg/minarg.hpp>


// ---- String ----

TEST_CASE("string value")
{
	std::string s{"s"};
	std::string o{"o"};

	minarg::Parser parser{};
	parser.addOption(s, 's', "", "", "");
	parser.addOperand(o, "", "");

	SECTION("empty")
	{
		parser.parse({"", "-s", ""});
		REQUIRE(s.empty());
	}
	SECTION("leading whitespace")
	{
		parser.parse({"", "-s", " \ts"});
		REQUIRE(s == " \ts");
	}
	SECTION("trailing whitespace")
	{
		parser.parse({"", "-s", "s\t "});
		REQUIRE(s == "s\t ");
	}
	SECTION("whitespace in-between")
	{
		parser.parse({"", "-s", "s \t s"});
		REQUIRE(s == "s \t s");
		REQUIRE(o == "o");
	}
	SECTION("stand-alone option prefix")
	{
		parser.parse({"", "-s", "-", "-"});
		REQUIRE(s == "-");
		REQUIRE(o == "-");
	}
}


// ---- Integer ----

TEST_CASE("general integer syntax")
{
	int i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("leading whitespace")
	{
		parser.parse({"", "-i", " -2"});
		REQUIRE(i == -2);
	}
//	SECTION("trailing whitespace") // Not feasible with std::st(u)ll
//	{
//		parser.parse({"", "-i", "2 "});
//		REQUIRE(i == 2);
//	}
	SECTION("empty string")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", ""}), minarg::Error);
	}
	SECTION("invalid character")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "1.0"}), minarg::Error);
	}
}


TEST_CASE("char is parsed as integer")
{
	char c{1};
	signed char s{1};
	unsigned char u{1};

	minarg::Parser parser{};
	parser.addOption(c, 'c', "", "", "");
	parser.addOption(s, 's', "", "", "");
	parser.addOption(u, 'u', "", "", "");

	SECTION("char")
	{
		parser.parse({"", "-c", "2"});
		REQUIRE(c == 2);
	}
	SECTION("signed char")
	{
		parser.parse({"", "-s", "-33"});
		REQUIRE(s == -33);
	}
	SECTION("unsigned char")
	{
		parser.parse({"", "-u", "44"});
		REQUIRE(u == 44);
	}
	SECTION("hexadecimal")
	{
		parser.parse({"", "-c", "0x1f"});
		REQUIRE(c == 31);
	}
	SECTION("invalid character")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-c", "a"}), minarg::Error);
	}
}


TEST_CASE("int8 value")
{
	std::int8_t i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("min")
	{
		parser.parse({"", "-i", "-128"});
		REQUIRE(i == std::numeric_limits<std::int8_t>::min());
	}
	SECTION("max")
	{
		parser.parse({"", "-i", "127"});
		REQUIRE(i == std::numeric_limits<std::int8_t>::max());
	}
	SECTION("underflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "-129"}), minarg::Error);
	}
	SECTION("overflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "128"}), minarg::Error);
	}
}


TEST_CASE("uint8 value")
{
	std::uint8_t i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("min")
	{
		parser.parse({"", "-i", "0"});
		REQUIRE(i == std::numeric_limits<std::uint8_t>::min());
	}
	SECTION("max")
	{
		parser.parse({"", "-i", "255"});
		REQUIRE(i == std::numeric_limits<std::uint8_t>::max());
	}
	SECTION("negative")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "-1"}), minarg::Error);
	}
	SECTION("overflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "256"}), minarg::Error);
	}
}


TEST_CASE("int32 value")
{
	std::int32_t i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("min")
	{
		parser.parse({"", "-i", "-2147483648"});
		REQUIRE(i == std::numeric_limits<std::int32_t>::min());
	}
	SECTION("max")
	{
		parser.parse({"", "-i", "2147483647"});
		REQUIRE(i == std::numeric_limits<std::int32_t>::max());
	}
	SECTION("underflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "-2147483649"}), minarg::Error);
	}
	SECTION("overflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "2147483648"}), minarg::Error);
	}
}


TEST_CASE("uint32 value")
{
	std::uint32_t i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("min")
	{
		parser.parse({"", "-i", "0"});
		REQUIRE(i == std::numeric_limits<std::uint32_t>::min());
	}
	SECTION("max")
	{
		parser.parse({"", "-i", "4294967295"});
		REQUIRE(i == std::numeric_limits<std::uint32_t>::max());
	}
	SECTION("negative")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "-1"}), minarg::Error);
	}
	SECTION("overflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "4294967296"}), minarg::Error);
	}
}


TEST_CASE("int64 value")
{
	std::int64_t i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("min")
	{
		parser.parse({"", "-i", "-9223372036854775808"});
		REQUIRE(i == std::numeric_limits<std::int64_t>::min());
	}
	SECTION("max")
	{
		parser.parse({"", "-i", "9223372036854775807"});
		REQUIRE(i == std::numeric_limits<std::int64_t>::max());
	}
	SECTION("underflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "-9223372036854775809"}), minarg::Error);
	}
	SECTION("overflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "9223372036854775808"}), minarg::Error);
	}
}


TEST_CASE("uint64 value")
{
	std::uint64_t i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("min")
	{
		parser.parse({"", "-i", "0"});
		REQUIRE(i == std::numeric_limits<std::uint64_t>::min());
	}
	SECTION("max")
	{
		parser.parse({"", "-i", "18446744073709551615"});
		REQUIRE(i == std::numeric_limits<std::uint64_t>::max());
	}
	SECTION("negative")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "-1"}), minarg::Error);
	}
	SECTION("overflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "18446744073709551616"}), minarg::Error);
	}
}


TEST_CASE("hexadecimal integer format")
{
	std::int32_t i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("uppercase")
	{
		parser.parse({"", "-i", "0XABCDEF"});
		REQUIRE(i == 11259375);
	}
	SECTION("lowercase")
	{
		parser.parse({"", "-i", "0xabcdef"});
		REQUIRE(i == 11259375);
	}
	SECTION("zero")
	{
		parser.parse({"", "-i", "0x00000000"});
		REQUIRE(i == 0);
	}
	SECTION("min")
	{
		parser.parse({"", "-i", "-0x80000000"});
		REQUIRE(i == std::numeric_limits<std::int32_t>::min());
	}
	SECTION("max")
	{
		parser.parse({"", "-i", "0x7fffffff"});
		REQUIRE(i == std::numeric_limits<std::int32_t>::max());
	}
	SECTION("underflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "-0x80000001"}), minarg::Error);
	}
	SECTION("overflow")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "0x80000000"}), minarg::Error);
	}
	SECTION("missing prefix")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "ff"}), minarg::Error);
	}
	SECTION("missing value")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "0x"}), minarg::Error);
	}
	SECTION("invalid character")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-i", "0xG"}), minarg::Error);
	}
}


TEST_CASE("ignore ambiguous octal integer format")
{
	int i{1};

	minarg::Parser parser{};
	parser.addOption(i, 'i', "", "", "");

	SECTION("non-octal digit")
	{
		parser.parse({"", "-i", "09"});
		REQUIRE(i == 9);
	}
	SECTION("ambiguous value")
	{
		parser.parse({"", "-i", "010"});
		REQUIRE(i == 10); // Not 8
	}
}


// ---- Floating point ----

// NOTE: Floating point implementation is platform dependent.
// The test values are chosen to represent reasonable use.
// Extreme values are not tested, e.g. underflow, overflow, INF, NAN


TEST_CASE("floating point value")
{
	float d{1.0f};

	minarg::Parser parser{};
	parser.addOption(d, 'd', "", "", "");

	SECTION("zero")
	{
		parser.parse({"", "-d", "0.0"});
		REQUIRE(d == 0.0f);
	}
	SECTION("large negative")
	{
		parser.parse({"", "-d", "-1000000.0"});
		REQUIRE(d == -1000000.0f);
	}
	SECTION("large positive")
	{
		parser.parse({"", "-d", "1000000.0"});
		REQUIRE(d == 1000000.0f);
	}
	SECTION("tiny negative")
	{
		parser.parse({"", "-d", "-0.000001"});
		REQUIRE(d == Approx(-0.000001f));
	}
	SECTION("tiny positive")
	{
		parser.parse({"", "-d", "0.000001"});
		REQUIRE(d == Approx(0.000001f));
	}
	SECTION("empty")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-f", ""}), minarg::Error);
	}
	SECTION("invalid char")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-f", "1.-"}), minarg::Error);
	}
	SECTION("missing significand")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-f", "e1"}), minarg::Error);
	}
	SECTION("missing exponent")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-f", "1e"}), minarg::Error);
	}
}


TEST_CASE("alternative floating point format")
{
	float d{1.0f};

	minarg::Parser parser{};
	parser.addOption(d, 'd', "", "", "");

	SECTION("pure integer")
	{
		parser.parse({"", "-d", "1"});
		REQUIRE(d == 1.0f);
	}
	SECTION("no fraction part")
	{
		parser.parse({"", "-d", "2."});
		REQUIRE(d == 2.0f);
	}
	SECTION("no integer part")
	{
		parser.parse({"", "-d", ".5"});
		REQUIRE(d == 0.5f);
	}
	SECTION("scientific verbose")
	{
		parser.parse({"", "-d", "1.0e-6"});
		REQUIRE(d == Approx(0.000001f));
	}
	SECTION("scientific minimal")
	{
		parser.parse({"", "-d", "1e6"});
		REQUIRE(d == 1000000.0f);
	}
	SECTION("scientific small negative")
	{
		parser.parse({"", "-d", "-1e-6"});
		REQUIRE(d == Approx(-0.000001f));
	}
	SECTION("scientific small positive")
	{
		parser.parse({"", "-d", "1e-6"});
		REQUIRE(d == Approx(0.000001f));
	}
	SECTION("scientific big negative")
	{
		parser.parse({"", "-d", "-1e+6"});
		REQUIRE(d == -1000000.0f);
	}
	SECTION("scientific big positive")
	{
		parser.parse({"", "-d", "1e+6"});
		REQUIRE(d == 1000000.0f);
	}
	SECTION("scientific uppercase")
	{
		parser.parse({"", "-d", "1E6"});
		REQUIRE(d == 1000000.0f);
	}
}


// ---- Custom type ----

struct YesNo
{
	bool value{false};
};


std::istream& operator>>(std::istream& stream, YesNo& y)
{
	std::string str{};
	if (stream >> str)
	{
		if (str == "yes")
			y.value = true;
		else if (str == "no")
			y.value = false;
		else
			stream.setstate(std::ios_base::failbit);
	}
	return stream;
}


std::ostream& operator<<(std::ostream& stream, const YesNo& y)
{
	stream << (y.value ? "yes" : "no");
	return stream;
}


TEST_CASE("custom value type")
{
	YesNo y{};

	minarg::Parser parser{};
	parser.addOption(y, 'y', "", "YY", "Yy");

	SECTION("default value")
	{
		parser.parse({"", "-y", "no"});
		REQUIRE(y.value == false);
	}
	SECTION("new value")
	{
		parser.parse({"", "-y", "yes"});
		REQUIRE(y.value == true);
	}
	SECTION("print default")
	{
		std::ostringstream stream{};
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  [-y YY]\n"
			"\n"
			"OPTIONS\n"
			"  -y YY  Yy (default: no)\n"
			"\n");
	}
	SECTION("invalid value")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-y", "ja"}), minarg::Error);
	}
}
