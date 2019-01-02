#include <catch2/catch.hpp>

#include <string>
#include <vector>

#include <minarg/minarg.hpp>


TEST_CASE("argv is char**")
{
	bool a{false};

	minarg::Parser parser{};
	parser.addOption(a, 'a', "", "");

	SECTION("argc = 0")
	{
		parser.parse(0, nullptr);
		REQUIRE(a == false);
	}
	SECTION("argc = 1")
	{
		const char* argv[] = {"-a", nullptr};
		parser.parse(1, argv);
		REQUIRE(a == false);
	}
	SECTION("argc > 1")
	{
		const char* argv[] = {"", "-a", nullptr};
		parser.parse(2, argv);
		REQUIRE(a == true);
	}
}


TEST_CASE("argv is vector")
{
	bool a{false};

	minarg::Parser parser{};
	parser.addOption(a, 'a', "", "");

	SECTION("argc = 0")
	{
		parser.parse({});
		REQUIRE(a == false);
	}
	SECTION("argc = 1")
	{
		parser.parse({"-a"});
		REQUIRE(a == false);
	}
	SECTION("argc > 1")
	{
		parser.parse({"", "-a"});
		REQUIRE(a == true);
	}
}


TEST_CASE("boolean option")
{
	bool a{false};
	bool b{false};
	bool c{false};

	minarg::Parser parser{};
	parser.addOption(a, 'a', ""   , "");
	parser.addOption(b,  0 , "bbb", "");
	parser.addOption(c, 'c', "ccc", "");

	SECTION("none")
	{
		parser.parse({""});
		REQUIRE(a == false);
		REQUIRE(b == false);
		REQUIRE(c == false);
	}
	SECTION("short name")
	{
		parser.parse({"", "-a"});
		REQUIRE(a == true);
		REQUIRE(b == false);
		REQUIRE(c == false);
	}
	SECTION("long name")
	{
		parser.parse({"", "--bbb"});
		REQUIRE(a == false);
		REQUIRE(b == true);
		REQUIRE(c == false);
	}
	SECTION("independent order")
	{
		parser.parse({"", "--bbb", "-c", "-a"});
		REQUIRE(a == true);
		REQUIRE(b == true);
		REQUIRE(c == true);
	}
	SECTION("combined")
	{
		parser.parse({"", "-ac"});
		REQUIRE(a == true);
		REQUIRE(b == false);
		REQUIRE(c == true);
	}
	SECTION("repetition")
	{
		parser.parse({"", "--bbb", "-aa", "--bbb"});
		REQUIRE(a == true);
		REQUIRE(b == true);
		REQUIRE(c == false);
	}
	SECTION("unkown short name")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-b"}), minarg::Error);
	}
	SECTION("unkown long name")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--aaa"}), minarg::Error);
	}
	SECTION("unkown combined name")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-ab"}), minarg::Error);
	}
}


TEST_CASE("value option")
{
	std::string a{"a"};
	std::string b{"b"};
	int i{1};
	bool s{false};

	minarg::Parser parser{};
	parser.addOption(a, 'a', ""   , "", "");
	parser.addOption(b,  0 , "bbb", "", "");
	parser.addOption(i, 'i', "iii", "", "");
	parser.addOption(s, 's', "sss", "");

	SECTION("empty input")
	{
		parser.parse({""});
		REQUIRE(a == "a");
		REQUIRE(b == "b");
		REQUIRE(i == 1);
	}
	SECTION("short name with separate value")
	{
		parser.parse({"", "-a", "A"});
		REQUIRE(a == "A");
		REQUIRE(b == "b");
		REQUIRE(i == 1);
	}
	SECTION("short name with merged value")
	{
		parser.parse({"", "-aA"});
		REQUIRE(a == "A");
		REQUIRE(b == "b");
		REQUIRE(i == 1);
	}
	SECTION("long name with separate value")
	{
		parser.parse({"", "--bbb", "B"});
		REQUIRE(a == "a");
		REQUIRE(b == "B");
		REQUIRE(i == 1);
	}
	SECTION("long name with merged value")
	{
		parser.parse({"", "--bbb=B"});
		REQUIRE(a == "a");
		REQUIRE(b == "B");
		REQUIRE(i == 1);
	}
	SECTION("long name with merged value (containing separator char)")
	{
		parser.parse({"", "--bbb=="});
		REQUIRE(a == "a");
		REQUIRE(b == "=");
		REQUIRE(i == 1);
	}
	SECTION("long name with merged value (empty)")
	{
		parser.parse({"", "--bbb="});
		REQUIRE(a == "a");
		REQUIRE(b == "");
		REQUIRE(i == 1);
	}
	SECTION("combined short names with separate value")
	{
		parser.parse({"", "-sa", "A"});
		REQUIRE(a == "A");
		REQUIRE(s == true);
	}
	SECTION("combined short names with merged value")
	{
		parser.parse({"", "-saA"});
		REQUIRE(a == "A");
		REQUIRE(s == true);
	}
	SECTION("value looks like option")
	{
		parser.parse({"", "-a", "-i", "--bbb", "--iii", "-i", "-2"});
		REQUIRE(a == "-i");
		REQUIRE(b == "--iii");
		REQUIRE(i == -2);
	}
	SECTION("independent order")
	{
		parser.parse({"", "--bbb", "B", "-i", "2", "-a", "A"});
		REQUIRE(a == "A");
		REQUIRE(b == "B");
		REQUIRE(i == 2);
	}
	SECTION("repeated options")
	{
		parser.parse({"", "-a", "A", "--bbb", "B", "-a", "AA", "--bbb", "BB", "-i", "2", "-i", "22"});
		REQUIRE(a == "AA");
		REQUIRE(b == "BB");
		REQUIRE(i == 22 );
	}
	SECTION("unkown short name")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-b", "B"}), minarg::Error);
	}
	SECTION("unkown long name")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--aaa", "A"}), minarg::Error);
	}
	SECTION("unkown combined name")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-sb", "B"}), minarg::Error);
	}
	SECTION("value option is not last in combined name")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-as", "A"}), minarg::Error);
	}
	SECTION("more than one combined value option")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-ai", "A", "2"}), minarg::Error);
	}
	SECTION("missing value after short option")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-a"}), minarg::Error);
	}
	SECTION("missing value after long separator")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--iii="}), minarg::Error);
	}
	SECTION("missing option name before long separator")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--=2"}), minarg::Error);
	}
	SECTION("long option includes unexpected separator")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--sss="}), minarg::Error);
	}
	SECTION("long option includes unexpected separator and value")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--sss=1"}), minarg::Error);
	}
}


TEST_CASE("operand")
{
	bool s{false};
	std::string a{"a"};
	int i{1};

	minarg::Parser parser{};
	parser.addOption(s, 's', "", "");
	parser.addOperand(a, "", "");
	parser.addOperand(i, "", "");

	SECTION("none provided")
	{
		parser.parse({""});
		REQUIRE(a == "a");
		REQUIRE(i == 1);
	}
	SECTION("one provided, one default")
	{
		parser.parse({"", "A"});
		REQUIRE(a == "A");
		REQUIRE(i == 1);
	}
	SECTION("all provided")
	{
		parser.parse({"", "A", "2"});
		REQUIRE(a == "A");
		REQUIRE(i == 2 );
	}
	SECTION("operand looks like short option prefix")
	{
		parser.parse({"", "-"});
		REQUIRE(a == "-");
		REQUIRE(i == 1);
	}
	SECTION("terminator before operands")
	{
		parser.parse({"", "--", "-s", "-2"});
		REQUIRE(s == false);
		REQUIRE(a == "-s");
		REQUIRE(i == -2);
	}
	SECTION("terminator between operands")
	{
		parser.parse({"", "A", "--", "-2"});
		REQUIRE(a == "A");
		REQUIRE(i == -2);
	}
	SECTION("terminator between sink operands")
	{
		std::vector<int> sink{};
		parser.addOperandSink(sink, "", "");
		parser.parse({"", "A", "2", "10", "--", "20"});
		REQUIRE(a == "A");
		REQUIRE(i == 2);
		REQUIRE(sink.size() == 2);
		REQUIRE(sink[0] == 10);
		REQUIRE(sink[1] == 20);
	}
	SECTION("terminator after operands")
	{
		parser.parse({"", "A", "2", "--"});
		REQUIRE(a == "A");
		REQUIRE(i == 2);
	}
	SECTION("operand is equal to terminator")
	{
		parser.parse({"", "--", "--"});
		REQUIRE(a == "--");
		REQUIRE(i == 1);
	}
	SECTION("option prefix match prevented by whitespace")
	{
		parser.parse({"", " -s", " -2"});
		REQUIRE(a == " -s");
		REQUIRE(i == -2);
	}
	SECTION("missing terminator before potential short option")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-a"}), minarg::Error);
	}
	SECTION("missing terminator before potential long option")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--aaa"}), minarg::Error);
	}
	SECTION("missing terminator before int operand")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "A", "-2"}), minarg::Error);
	}
	SECTION("too many operands")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "A", "2", "3"}), minarg::Error);
	}
}


TEST_CASE("operand sink")
{
	std::vector<std::string> sink{};

	minarg::Parser parser{};
	parser.addOperandSink(sink, "", "");

	SECTION("empty")
	{
		parser.parse({""});
		REQUIRE(sink.size() == 0);
	}
	SECTION("one element")
	{
		parser.parse({"", "A"});
		REQUIRE(sink.size() == 1);
		REQUIRE(sink[0] == "A");
	}
	SECTION("multiple elements")
	{
		parser.parse({"", "A", "B", "-"});
		REQUIRE(sink.size() == 3);
		REQUIRE(sink[0] == "A");
		REQUIRE(sink[1] == "B");
		REQUIRE(sink[2] == "-");
	}
	SECTION("terminator before values")
	{
		parser.parse({"", "--", "-A", "--B", "C"});
		REQUIRE(sink.size() == 3);
		REQUIRE(sink[0] == "-A");
		REQUIRE(sink[1] == "--B");
		REQUIRE(sink[2] == "C");
	}
	SECTION("terminator between values")
	{
		parser.parse({"", "A", "--", "--B", "C"});
		REQUIRE(sink.size() == 3);
		REQUIRE(sink[0] == "A");
		REQUIRE(sink[1] == "--B");
		REQUIRE(sink[2] == "C");
	}
	SECTION("terminator after values")
	{
		parser.parse({"", "A", "B", "C", "--"});
		REQUIRE(sink.size() == 3);
		REQUIRE(sink[0] == "A");
		REQUIRE(sink[1] == "B");
		REQUIRE(sink[2] == "C");
	}
	SECTION("preserved pre-existing elements in sink")
	{
		sink.push_back("pre");
		parser.parse({"", "A", "B"});
		REQUIRE(sink.size() == 3);
		REQUIRE(sink[0] == "pre");
		REQUIRE(sink[1] == "A");
		REQUIRE(sink[2] == "B");
	}
	SECTION("missing terminator")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-a"}), minarg::Error);
	}
}


TEST_CASE("required options and operands")
{
	bool b{false};
	std::string v{"v"};
	std::string w{"w"};
	std::string o{"o"};
	std::vector<std::string> s{};

	minarg::Parser parser{};
	parser.addOption(b, 'b', ""   ,     "", true);
	parser.addOption(v, 'v', ""   , "", "", true);
	parser.addOption(w,  0 , "www", "", "", true);
	parser.addOperand(o, "", "", true);
	parser.addOperandSink(s, "", "", true);

	SECTION("all are present, option values are separate")
	{
		parser.parse({"", "-b", "-v", "V", "--www", "W", "O", "S"});
		REQUIRE(b == true);
		REQUIRE(v == "V");
		REQUIRE(w == "W");
		REQUIRE(o == "O");
		REQUIRE(s.size() == 1);
		REQUIRE(s[0] == "S");
	}
	SECTION("all are present, option values are merged")
	{
		parser.parse({"", "-b", "-vV", "--www=W", "O", "S"});
		REQUIRE(b == true);
		REQUIRE(v == "V");
		REQUIRE(w == "W");
		REQUIRE(o == "O");
		REQUIRE(s.size() == 1);
		REQUIRE(s[0] == "S");
	}
	SECTION("required boolean option is missing")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-vV", "--www=W", "O", "S"}), minarg::Error);
	}
	SECTION("required value option is missing")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-b", "--www=W", "O", "S"}), minarg::Error);
	}
	SECTION("required operand is missing")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-b", "-vV", "--www=W"}), minarg::Error);
	}
	SECTION("required operand for sink is missing")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-b", "-vV", "--www=W", "O"}), minarg::Error);
	}
}


TEST_CASE("custom syntax")
{
	bool a{false};
	bool b{false};
	int i{1};
	std::string o{"o"};

	minarg::Parser parser{};
	parser.addOption(a, 'a', ""   , "");
	parser.addOption(b,  0 , "bbb", "");
	parser.addOption(i,  0 , "iii", "", "");
	parser.addOperand(o, "", "");

	SECTION("custom short prefix")
	{
		parser.setShortOptionPrefix('+');
		parser.parse({"", "+a"});
		REQUIRE(a == true);
		REQUIRE(b == false);
	}
	SECTION("custom long prefix")
	{
		parser.setLongOptionPrefix("+");
		parser.parse({"", "+bbb"});
		REQUIRE(a == false);
		REQUIRE(b == true);
	}
	SECTION("disabled long prefix")
	{
		parser.setShortOptionPrefix('+');
		parser.setLongOptionPrefix("");
		parser.parse({"", "--bbb"});
		REQUIRE(o == "--bbb");
	}
	SECTION("custom long separator")
	{
		parser.setLongOptionSeparator(':');
		parser.parse({"", "--iii:2"});
		REQUIRE(i == 2);
	}
	SECTION("disable long separator")
	{
		parser.setLongOptionSeparator(0);
		REQUIRE_THROWS_AS(parser.parse({"", "--iii=2"}), minarg::Error);
	}
	SECTION("custom terminator looks like option")
	{
		parser.setOptionTerminator("-a");
		parser.parse({"", "-a", "-a"});
		REQUIRE(a == false);
		REQUIRE(o == "-a");
	}
	SECTION("disabled terminator")
	{
		parser.setOptionTerminator("");
		parser.parse({"", "-a", ""});
		REQUIRE(a == true);
		REQUIRE(o == "");
	}
}


TEST_CASE("prefix precedence")
{
	bool a{false};
	bool al{false};
	bool b{false};
	bool ab{false};

	minarg::Parser parser{};
	parser.addOption(a, 'a', ""  , "");
	parser.addOption(al, 0 , "a" , "");
	parser.addOption(b, 'b', ""  , "");
	parser.addOption(ab, 0 , "ab", "");
	parser.setShortOptionPrefix('/');
	parser.setLongOptionPrefix("/");

	SECTION("long option looks like short option")
	{
		parser.parse({"", "/a"});
		REQUIRE(a  == false);
		REQUIRE(al == true);
	}
	SECTION("long option looks like combined short options")
	{
		parser.parse({"", "/ab"});
		REQUIRE(a  == false);
		REQUIRE(al == false);
		REQUIRE(b  == false);
		REQUIRE(ab == true);
	}
}


TEST_CASE("value precedence")
{
	bool s{false};
	std::string a{"a"};
	std::string o{"o"};

	minarg::Parser parser{};
	parser.addOption(s, 's', "", "");
	parser.addOption(a, 'a', "", "", "");
	parser.addOperand(o, "", "");

	SECTION("option value looks like option")
	{
		parser.parse({"", "-a", "-s"});
		REQUIRE(a == "-s");
		REQUIRE(s == false);
	}
	SECTION("option value looks like terminator")
	{
		parser.parse({"", "-a", "--", "-s"});
		REQUIRE(a == "--");
		REQUIRE(s == true);
		REQUIRE(o == "o");
	}
	SECTION("operand looks like option after terminator")
	{
		parser.parse({"", "--", "-s"});
		REQUIRE(o == "-s");
		REQUIRE(s == false);
	}
}
