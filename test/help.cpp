#include <catch2/catch.hpp>

#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <minarg/minarg.hpp>


TEST_CASE("signal")
{
	minarg::Parser parser{};
	parser.addSignal('h', "help", "");

	SECTION("short help signal")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "-h"}), minarg::Signal);
	}
	SECTION("long help signal")
	{
		REQUIRE_THROWS_AS(parser.parse({"", "--help"}), minarg::Signal);
	}
	SECTION("throw signal even if required options are missing")
	{
		bool a{false};
		parser.addOption(a, 'a', "", "", true);
		REQUIRE_THROWS_AS(parser.parse({"", "-h"}), minarg::Signal);
	}
	SECTION("signal merged with other option")
	{
		bool a{false};
		parser.addOption(a, 'a', "", "");
		REQUIRE_THROWS_AS(parser.parse({"", "-ah"}), minarg::Signal);
	}
	SECTION("distinguish between different signals")
	{
		try
		{
			parser.addSignal('v', "version", "");
			parser.parse({"", "--version"});
		}
		catch (const minarg::Signal& h)
		{
			REQUIRE(h.shortName == 'v');
			REQUIRE(h.longName == "version");
		}
	}
}


TEST_CASE("help message sections")
{
	bool a{false};
	int b{1};

	std::ostringstream stream{};
	minarg::Parser parser{"Prolog", "Epilog"};
	parser.setUtilityName("utility");
	parser.addOption(a, 'a', "", "Aa");
	parser.addOperand(b, "BBB", "Bb", true);

	SECTION("default")
	{
		stream << parser;
		REQUIRE(stream.str() ==
			"Prolog\n"
			"\n"
			"USAGE\n"
			"  utility [-a] BBB\n"
			"\n"
			"OPTIONS\n"
			"  -a  Aa\n"
			"\n"
			"OPERANDS\n"
			"  BBB  Bb\n"
			"\n"
			"Epilog\n"
			"\n");
	}
	SECTION("custom titles")
	{
		parser.setUsageTitle("Hello");
		parser.setOptionsTitle("World");
		parser.setOperandsTitle("Goodbye");
		stream << parser;
		REQUIRE(stream.str() ==
			"Prolog\n"
			"\n"
			"Hello\n"
			"  utility [-a] BBB\n"
			"\n"
			"World\n"
			"  -a  Aa\n"
			"\n"
			"Goodbye\n"
			"  BBB  Bb\n"
			"\n"
			"Epilog\n"
			"\n");
	}
	SECTION("preserve stream formatting")
	{
		auto flags{stream.flags()};
		auto locale{stream.getloc()};
		stream << parser;
		REQUIRE(stream.flags() == flags);
		REQUIRE(stream.getloc() == locale);
	}
}


TEST_CASE("usage section")
{
	bool a{false};
	int b{1};

	std::ostringstream stream{};
	minarg::Parser parser{};
	parser.setOptionsTitle("");
	parser.setOperandsTitle("");
	parser.addOption(a, 'a', "", "Aa");
	parser.addOperand(b, "BBB", "Bb");

	SECTION("read utility name from argv")
	{
		parser.parse({"hello"});
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  hello [-a] [BBB]\n"
			"\n");
	}
	SECTION("preserve custom utility name")
	{
		parser.setUtilityName("custom");
		parser.parse({"hello"});
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  custom [-a] [BBB]\n"
			"\n");
	}
	SECTION("custom options and operands usage strings")
	{
		parser.setUtilityName("utility");
		parser.setOptionsUsage("options...");
		parser.setOperandsUsage("operands...");
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  utility options... operands...\n"
			"\n");
	}
}


TEST_CASE("help message formatting")
{
	bool a{false};
	int i{1};
	std::vector<int> sink{};

	std::ostringstream stream{};
	minarg::Parser parser{};
	parser.setUtilityName("hello");

	SECTION("required options and operands")
	{
		parser.addOption(a, 'a', "", "Aa", true);
		parser.addOption(i, 'b', "", "BB", "Bb", true);
		parser.addOperand(i, "CC", "Cc", true);
		parser.addOperandSink(sink, "DDD", "Dd", true);
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  hello -a -b BB CC DDD...\n"
			"\n"
			"OPTIONS\n"
			"  -a     Aa\n"
			"  -b BB  Bb\n"
			"\n"
			"OPERANDS\n"
			"  CC   Cc\n"
			"  DDD  Dd\n"
			"\n");
	}
	SECTION("optional options and operands")
	{
		parser.addOption(a, 'a', "", "Aa");
		parser.addOption(i, 'b', "", "BB", "Bb");
		parser.addOperand(i, "CC", "Cc");
		parser.addOperandSink(sink, "DDD", "Dd");
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  hello [-a] [-b BB] [CC] [DDD]...\n"
			"\n"
			"OPTIONS\n"
			"  -a     Aa\n"
			"  -b BB  Bb (default: 1)\n"
			"\n"
			"OPERANDS\n"
			"  CC   Cc (default: 1)\n"
			"  DDD  Dd\n"
			"\n");
	}
	SECTION("only long options")
	{
		parser.addOption(a, 0, "aaaa", "Aa", true);
		parser.addOption(i, 0, "bb", "BBB", "Bb", true);
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  hello --aaaa --bb BBB\n"
			"\n"
			"OPTIONS\n"
			"  --aaaa    Aa\n"
			"  --bb BBB  Bb\n"
			"\n");
	}
	SECTION("mix of short and long options")
	{
		parser.setHelpWidth(21);
		parser.addOption(a, 'a', "aa", "Aa", true);
		parser.addOption(i, 'b', "bbb", "BB", "Bb", true);
		parser.addOption(i, 'c', "", "CCC", "Cc", true);
		parser.addOption(i, 0, "dddd", "DDDD", "Dd", true);
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  hello -a -b BB\n"
			"    -c CCC\n"
			"    --dddd DDDD\n"
			"\n"
			"OPTIONS\n"
			"  -a, --aa         Aa\n"
			"  -b, --bbb BB     Bb\n"
			"  -c CCC           Cc\n"
			"      --dddd DDDD  Dd\n"
			"\n");
	}
	SECTION("custom prefixes")
	{
		parser.setShortOptionPrefix('+');
		parser.setLongOptionPrefix("/");
		parser.addOption(a, 'a', "", "Aa", true);
		parser.addOption(i, 0, "bbb", "BB", "Bb", true);
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  hello +a /bbb BB\n"
			"\n"
			"OPTIONS\n"
			"  +a           Aa\n"
			"      /bbb BB  Bb\n"
			"\n");
	}
	SECTION("custom indent")
	{
		parser.setHelpWidth(16);
		parser.setHelpIndent(4);
		parser.addOption(i, 'b', "", "BB", "Bb", true);
		parser.addOperand(i, "CCCC", "Cc", true);
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"    hello -b BB\n"
			"        CCCC\n"
			"\n"
			"OPTIONS\n"
			"    -b BB    Bb\n"
			"\n"
			"OPERANDS\n"
			"    CCCC    Cc\n"
			"\n");
	}
}


TEST_CASE("help message with default values")
{
	std::ostringstream stream{};
	minarg::Parser parser{};
	parser.setUsageTitle("");
	parser.setOptionsTitle("");

	SECTION("print string value")
	{
		std::string empty{""};
		std::string hello{"hello"};

		parser.addOperand(empty, "empty", "");
		parser.addOperand(hello, "hello", "");

		stream << parser;
		REQUIRE(stream.str() ==
			"OPERANDS\n"
			"  empty  (default: \"\")\n"
			"  hello  (default: \"hello\")\n"
			"\n");
	}

	SECTION("print char value as integer")
	{
		char ch{65};
		signed char sCh{-65};
		unsigned char uCh{65};

		parser.addOperand(ch , "char ", "");
		parser.addOperand(sCh, "sChar", "");
		parser.addOperand(uCh, "uChar", "");

		stream << parser;
		REQUIRE(stream.str() ==
			"OPERANDS\n"
			"  char   (default: 65)\n"
			"  sChar  (default: -65)\n"
			"  uChar  (default: 65)\n"
			"\n");
	}

	SECTION("print integer value")
	{
		auto int8Min   {std::numeric_limits<std::int8_t>::min()};
		auto int8Max   {std::numeric_limits<std::int8_t>::max()};
		auto uint8Max  {std::numeric_limits<std::uint8_t>::max()};
		auto int32Min  {std::numeric_limits<std::int32_t>::min()};
		auto int32Max  {std::numeric_limits<std::int32_t>::max()};
		auto uint32Max {std::numeric_limits<std::uint32_t>::max()};
		auto int64Min  {std::numeric_limits<std::int64_t>::min()};
		auto int64Max  {std::numeric_limits<std::int64_t>::max()};
		auto uint64Max {std::numeric_limits<std::uint64_t>::max()};

		parser.addOperand(int8Min   , "int8Min  ", "");
		parser.addOperand(int8Max   , "int8Max  ", "");
		parser.addOperand(uint8Max  , "uint8Max ", "");
		parser.addOperand(int32Min  , "int32Min ", "");
		parser.addOperand(int32Max  , "int32Max ", "");
		parser.addOperand(uint32Max , "uint32Max", "");
		parser.addOperand(int64Min  , "int64Min ", "");
		parser.addOperand(int64Max  , "int64Max ", "");
		parser.addOperand(uint64Max , "uint64Max", "");

		stream << parser;
		REQUIRE(stream.str() ==
			"OPERANDS\n"
			"  int8Min    (default: -128)\n"
			"  int8Max    (default: 127)\n"
			"  uint8Max   (default: 255)\n"
			"  int32Min   (default: -2147483648)\n"
			"  int32Max   (default: 2147483647)\n"
			"  uint32Max  (default: 4294967295)\n"
			"  int64Min   (default: -9223372036854775808)\n"
			"  int64Max   (default: 9223372036854775807)\n"
			"  uint64Max  (default: 18446744073709551615)\n"
			"\n");
	}

	SECTION("print floating point value")
	{
		float zero{0.0f};
		float half{0.5f};

		parser.addOperand(zero, "zero", "");
		parser.addOperand(half, "half", "");

		stream << parser;
		REQUIRE(stream.str() ==
			"OPERANDS\n"
			"  zero  (default: 0)\n" // std::ostream output, 0.0 would be better
			"  half  (default: 0.5)\n"
			"\n");
	}

	SECTION("custom default intro")
	{
		int i{2};
		parser.addOperand(i, "II", "Ii");
		parser.setDefaultValueIntro("Hello:");
		stream << parser;
		REQUIRE(stream.str() ==
			"OPERANDS\n"
			"  II  Ii (Hello:2)\n"
			"\n");
	}
	SECTION("disabled default")
	{
		int i{2};
		parser.addOperand(i, "II", "Ii");
		parser.setDefaultValueIntro("");
		stream << parser;
		REQUIRE(stream.str() ==
			"OPERANDS\n"
			"  II  Ii\n"
			"\n");
	}
}


TEST_CASE("help message line wrapping")
{
	bool a{false};

	minarg::Parser parser{};
	parser.setUsageTitle("");
	parser.setOperandsTitle("");
	parser.setHelpWidth(21);

	std::ostringstream stream{};

	SECTION("boundary checks")
	{
		parser.addOption(a, 'a', "", "Exactly to here Can't fit next t Fullwidthtoken.");
		stream << parser;
		REQUIRE(stream.str() ==
			"OPTIONS\n"
			"  -a  Exactly to here\n"
			"      Can't fit next\n"
			"      t\n"
			"      Fullwidthtoken.\n"
			"\n");
	}
	SECTION("overshoot")
	{
		parser.addOption(a, 'a', "", "Thisisaverylongtoken Next line ok Anotherverylongtoken");
		stream << parser;
		REQUIRE(stream.str() ==
			"OPTIONS\n"
			"  -a  Thisisaverylongtoken\n"
			"      Next line ok\n"
			"      Anotherverylongtoken\n"
			"\n");
	}
	SECTION("explicit newline")
	{
		parser.addOption(a, 'a', "", "First\nSecond line\n\nFourth \n Fifth");
		stream << parser;
		REQUIRE(stream.str() ==
			"OPTIONS\n"
			"  -a  First\n"
			"      Second line\n"
			"\n"
			"      Fourth\n"
			"      Fifth\n"
			"\n");
	}
	SECTION("space collapsing")
	{
		parser.addOption(a, 'a', "", "  Hello,   world!  ");
		stream << parser;
		REQUIRE(stream.str() ==
			"OPTIONS\n"
			"  -a  Hello, world!\n"
			"\n");
	}
	SECTION("text only consists of whitespace")
	{
		parser.addOption(a, 'a', "", "    ");
		stream << parser;
		REQUIRE(stream.str() ==
			"OPTIONS\n"
			"  -a  \n"
			"\n");
	}
	SECTION("zero width (stupid)")
	{
		int i{1};
		parser.addOption(i, 'a', "aaa", "AA", "A stupid width.");
		parser.addOperand(i, "BBB", "Still stupid...");
		parser.setUsageTitle("USAGE");
		parser.setOperandsTitle("OPERANDS");
		parser.setUtilityName("hello");
		parser.setHelpWidth(0);
		stream << parser;
		REQUIRE(stream.str() ==
			"USAGE\n"
			"  hello\n"
			"    [-a AA]\n"
			"    [BBB]\n"
			"\n"
			"OPTIONS\n"
			"  -a, --aaa AA  A\n"
			"                stupid\n"
			"                width.\n"
			"                (default: 1)\n"
			"\n"
			"OPERANDS\n"
			"  BBB  Still\n"
			"       stupid...\n"
			"       (default: 1)\n"
			"\n");
	}
}
