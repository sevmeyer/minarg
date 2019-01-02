#include <catch2/catch.hpp>

#include <string>
#include <vector>

#include <minarg/minarg.hpp>


TEST_CASE("error messages")
{
	bool s{false};
	int i{1};
	unsigned int u{1};
	int a{1};
	int b{1};

	minarg::Parser parser{};
	parser.addOption(s, 's', "ss"    , "");
	parser.addOption(i, 'i', "ii", "", "");
	parser.addOption(u,  0 , "uu", "", "");
	parser.addOperand(a, "aa", "");
	parser.addOperand(b, "bb", "");

	SECTION("invalid unsigned integer")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "--uu", "-2"}), "Cannot parse unsigned integer: -2");
	}
	SECTION("invalid integer")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "-i", "foo"}), "Cannot parse integer: foo");
	}
	SECTION("invalid character after valid integer")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "-i", "12x"}), "Cannot parse integer: 12x");
	}
	SECTION("long option value is missing")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "--uu"}), "Cannot find value for option: --uu");
	}
	SECTION("long option has unexpected merged value")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "--ss=S"}), "Unexpected option value: --ss=S");
	}
	SECTION("short option value is missing")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "-i"}), "Cannot find value for option: -i");
	}
	SECTION("operand looks like short option")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "2", "-3"}), "Unexpected option: -3");
	}
	SECTION("operand looks like long option")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "2", "--33"}), "Unexpected option: --33");
	}
	SECTION("unexpected argument")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "2", "3", "4"}), "Unexpected argument: 4");
	}
	SECTION("unkown short option")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "-x"}), "Unknown option name: x");
	}
	SECTION("unkown long option")
	{
		REQUIRE_THROWS_WITH(parser.parse({"", "--xx"}), "Unknown option name: xx");
	}
	SECTION("missing required boolean option (short name only)")
	{
		bool x{false};
		parser.addOption(x, 'x', "", "", true);
		REQUIRE_THROWS_WITH(parser.parse({""}), "Cannot find required argument: -x");
	}
	SECTION("missing required option (short and long name)")
	{
		int x{1};
		parser.addOption(x, 'x', "xx", "", "", true);
		REQUIRE_THROWS_WITH(parser.parse({""}), "Cannot find required argument: -x");
	}
	SECTION("missing required option (long name only)")
	{
		int x{1};
		parser.addOption(x, 0, "xx", "", "", true);
		REQUIRE_THROWS_WITH(parser.parse({""}), "Cannot find required argument: --xx");
	}
	SECTION("missing required operand")
	{
		int x{1};
		parser.addOperand(x, "xx", "", true);
		REQUIRE_THROWS_WITH(parser.parse({""}), "Cannot find required argument: xx");
	}
	SECTION("missing required sink")
	{
		std::vector<int> x{};
		parser.addOperandSink(x, "xx", "", true);
		REQUIRE_THROWS_WITH(parser.parse({""}), "Cannot find required argument: xx");
	}
}
