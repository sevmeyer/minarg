// minarg 1.0.1
// A minimalist argument parsing library for C++11
// https://github.com/sevmeyer/minarg
//
// Copyright 2018 Severin Meyer
// Licensed under the Boost Software License 1.0
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


#ifndef MINARG_MINARG_HPP_INCLUDED
#define MINARG_MINARG_HPP_INCLUDED


#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>


namespace minarg {
namespace detail {


// ---- Exceptions ----

struct Error : public std::exception
{
	const std::string message;

	Error(std::string message) :
		message{std::move(message)}
	{}

	const char* what() const noexcept override
	{
		return message.c_str();
	}
};


struct Signal : public std::exception
{
	const char shortName;
	const std::string longName;

	Signal(char shortName, std::string longName) :
		shortName{shortName},
		longName{std::move(longName)}
	{}
};


// ---- String to value ----

// Prepare signed integer
template<typename T>
typename std::enable_if<std::is_signed<T>::value, long long>::type
toLongest(const std::string& s, std::size_t* pos, int base)
{
	return std::stoll(s, pos, base);
}


// Prepare unsigned integer
template<typename T>
typename std::enable_if<std::is_unsigned<T>::value, unsigned long long>::type
toLongest(const std::string& s, std::size_t* pos, int base)
{
	// Detect underflow, otherwise stoull wraps around
	// negative values instead of reporting an error
	if (s.find('-') != std::string::npos)
		throw Error{"Cannot parse unsigned integer: " + s};
	return std::stoull(s, pos, base);
}


// Read integer
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
fromString(const std::string& s)
{
	try
	{
		std::size_t pos{0};
		int base{s.find_first_of("Xx") == std::string::npos ? 10 : 16};
		auto value{toLongest<T>(s, &pos, base)};
		if (pos == s.size()
			&& value >= std::numeric_limits<T>::min()
			&& value <= std::numeric_limits<T>::max())
			return static_cast<T>(value);
	}
	catch(const std::invalid_argument&) {}
	catch(const std::out_of_range&) {}
	throw Error{"Cannot parse integer: " + s};
}


// Read non-integer
template<typename T>
typename std::enable_if<!std::is_integral<T>::value, T>::type
fromString(const std::string& s)
{
	T value{};
	std::stringstream stream{s};
	stream >> value >> std::ws;

	if (stream.fail() || !stream.eof())
		throw Error{"Cannot parse value: " + s};

	return value;
}


// Read string
template<>
inline std::string
fromString<std::string>(const std::string& s)
{
	return s;
}


// ---- Value to string ----

// Write integer
template<typename T>
typename std::enable_if<std::is_integral<T>::value, void>::type
toStream(std::ostream& stream, const T& value)
{
	// Upcast all integers, otherwise the
	// char types are interpreted as text
	if (std::is_signed<T>::value)
		stream << static_cast<std::intmax_t>(value);
	else
		stream << static_cast<std::uintmax_t>(value);
}


// Write non-integer
template<typename T>
typename std::enable_if<!std::is_integral<T>::value, void>::type
toStream(std::ostream& stream, const T& value)
{
	if (std::is_same<T, std::string>::value)
		stream << '\"' << value << '\"';
	else
		stream << value;
}


// Create string
template<typename T>
std::string toString(const T& value)
{
	std::stringstream stream{};
	toStream<T>(stream, value);
	return stream.str();
}


// ---- Polymorphic argument types ----

class Arg
{
	public:

		virtual ~Arg() noexcept = default;

		char getShortName() const { return shortName_; }

		const std::string& getLongName()    const { return longName_;    }
		const std::string& getValueName()   const { return valueName_;   }
		const std::string& getDescription() const { return description_; }

		bool isRequired() const { return isRequired_; }
		bool hasValue()   const { return hasValue_;   }
		bool isSink()     const { return isSink_;     }
		bool isDone()     const { return isDone_;     }

		void parse(const std::string& s)
		{
			doParse(s);
		}

		void done()
		{
			doDone();
			isDone_ = true;
		}

		std::string getDefaultValue() const
		{
			return isRequired_ ? std::string{} : doGetDefaultValue();
		}

	protected:

		Arg(char shortName,
			std::string longName,
			std::string valueName,
			std::string description,
			bool isRequired,
			bool hasValue,
			bool isSink) :
				shortName_{shortName},
				longName_{std::move(longName)},
				valueName_{std::move(valueName)},
				description_{std::move(description)},
				isRequired_{isRequired},
				hasValue_{hasValue},
				isSink_{isSink}
		{}

		virtual void doParse(const std::string&) {}
		virtual void doDone() {}
		virtual std::string doGetDefaultValue() const { return {}; }

	private:

		const char shortName_;
		const std::string longName_;
		const std::string valueName_;
		const std::string description_;

		const bool isRequired_;
		const bool hasValue_;
		const bool isSink_;
		bool isDone_{false};
};


class SignalArg : public Arg
{
	public:

		SignalArg(char shortName,
			std::string longName,
			std::string description) :
				Arg{shortName,
					std::move(longName),
					{},
					std::move(description),
					false, false, false}
		{}

	protected:

		void doDone() override
		{
			throw Signal{getShortName(), getLongName()};
		}
};


class BoolArg : public Arg
{
	public:

		BoolArg(char shortName,
			std::string longName,
			std::string description,
			bool isRequired,
			bool& target) :
				Arg{shortName,
					std::move(longName),
					{},
					std::move(description),
					isRequired, false, false},
				target_{target}
		{}

	protected:

		void doDone() override
		{
			target_ = true;
		}

	private:

		bool& target_;
};


template<typename T>
class ValueArg : public Arg
{
	public:

		ValueArg(char shortName,
			std::string longName,
			std::string valueName,
			std::string description,
			bool isRequired,
			T& target) :
				Arg{shortName,
					std::move(longName),
					std::move(valueName),
					std::move(description),
					isRequired, true, false},
				target_{target},
				default_{target}
		{}

	protected:

		void doParse(const std::string& s) override
		{
			target_ = fromString<T>(s);
		}

		std::string doGetDefaultValue() const override
		{
			return toString<T>(default_);
		}

	private:

		T& target_;
		const T default_;
};


template<template<typename...> class Container, typename T>
class SinkArg : public Arg
{
	public:

		SinkArg(
			std::string valueName,
			std::string description,
			bool isRequired,
			Container<T>& target) :
				Arg{0,
					{},
					std::move(valueName),
					std::move(description),
					isRequired, true, true},
				target_{target}
		{}

	protected:

		void doParse(const std::string& s) override
		{
			target_.push_back(fromString<T>(s));
		}

	private:

		Container<T>& target_;
};


// ---- Public interface ----

class Parser
{
	using ArgPtr = std::unique_ptr<Arg>;
	using ParseIt = std::vector<std::string>::const_iterator;
	using StringSize = std::string::size_type;

	public:

		// ---- Constructor ----

		Parser(std::string helpProlog={}, std::string helpEpilog={}) :
			helpProlog_{std::move(helpProlog)},
			helpEpilog_{std::move(helpEpilog)}
		{}

		// ---- Arguments ----

		void addSignal(
			char shortName,
			std::string longName,
			std::string description)
		{
			options_.push_back(ArgPtr{new SignalArg{
				shortName,
				std::move(longName),
				std::move(description) }});
		}

		void addOption(
			bool& target,
			char shortName,
			std::string longName,
			std::string description,
			bool isRequired = false)
		{
			options_.push_back(ArgPtr{new BoolArg{
				shortName,
				std::move(longName),
				std::move(description),
				isRequired,
				target }});
		}

		template<typename T>
		void addOption(
			T& target,
			char shortName,
			std::string longName,
			std::string valueName,
			std::string description,
			bool isRequired = false)
		{
			options_.push_back(ArgPtr{new ValueArg<T>{
				shortName,
				std::move(longName),
				std::move(valueName),
				std::move(description),
				isRequired,
				target }});
		}

		template<typename T>
		void addOperand(
			T& target,
			std::string valueName,
			std::string description,
			bool isRequired = false)
		{
			operands_.push_back(ArgPtr{new ValueArg<T>{
				0,
				{},
				std::move(valueName),
				std::move(description),
				isRequired,
				target }});
		}

		template<template<typename...> class Container, typename T>
		void addOperandSink(
			Container<T>& target,
			std::string valueName,
			std::string description,
			bool isRequired = false)
		{
			operands_.push_back(ArgPtr{new SinkArg<Container, T>{
				std::move(valueName),
				std::move(description),
				isRequired,
				target }});
		}

		// ---- Parsing ----
		// Expects standard argc and argv parameters
		// https://en.cppreference.com/w/cpp/language/main_function

		void parse(int argc, const char* const argv[])
		{
			parse({argv, argv + argc});
		}

		void parse(const std::vector<std::string>& argv)
		{
			ParseIt it{argv.begin()};
			parseAll(it, argv.end());
		}

		// ---- Settings ----

		void setShortOptionPrefix(char c)        { shortPrefix_   = c; }
		void setLongOptionPrefix(std::string s)  { longPrefix_    = std::move(s); }
		void setLongOptionSeparator(char c)      { longSeparator_ = c; }
		void setOptionTerminator(std::string s)  { terminator_    = std::move(s); }
		void setUsageTitle(std::string s)        { usageTitle_    = std::move(s); }
		void setOptionsTitle(std::string s)      { optionsTitle_  = std::move(s); }
		void setOperandsTitle(std::string s)     { operandsTitle_ = std::move(s); }
		void setUtilityName(std::string s)       { utilityName_   = std::move(s); }
		void setOptionsUsage(std::string s)      { optionsUsage_  = std::move(s); }
		void setOperandsUsage(std::string s)     { operandsUsage_ = std::move(s); }
		void setDefaultValueIntro(std::string s) { defaultIntro_  = std::move(s); }
		void setHelpWidth(StringSize s)          { helpWidth_     = s > 0 ? s : 0; }
		void setHelpIndent(StringSize s)         { helpIndent_    = s > 0 ? s : 0; }

	private:

		char shortPrefix_{'-'};
		std::string longPrefix_{"--"};
		char longSeparator_{'='};
		std::string terminator_{"--"};
		bool isTerminated_{false};

		std::string helpProlog_{};
		std::string helpEpilog_{};
		std::string usageTitle_{"USAGE"};
		std::string optionsTitle_{"OPTIONS"};
		std::string operandsTitle_{"OPERANDS"};
		std::string utilityName_{};
		std::string optionsUsage_{};
		std::string operandsUsage_{};
		std::string defaultIntro_{"default: "};

		StringSize helpWidth_{80};
		StringSize helpIndent_{2};

		// INVARIANT: Arg* != nullptr
		std::vector<ArgPtr> options_{};
		std::vector<ArgPtr> operands_{};

		// ---- Parse ----

		void parseAll(ParseIt& it, ParseIt end)
		{
			parseUtility(it, end);
			parseOptions(it, end);
			parseOperands(it, end);
			parseTerminator(it, end);

			checkEnd(it, end);
			checkRequired(options_);
			checkRequired(operands_);
		}

		void parseUtility(ParseIt& it, ParseIt end)
		{
			if (it == end)
				return;

			if (utilityName_.empty())
				utilityName_ = *it;
			++it;
		}

		void parseOptions(ParseIt& it, ParseIt end)
		{
			while (true)
			{
				auto old{it};

				parseTerminator(it, end);
				if (it != old)
					break;

				parseLongOption(it, end);
				if (it != old)
					continue;

				parseShortOptions(it, end);
				if (it != old)
					continue;

				break;
			}
		}

		void parseTerminator(ParseIt& it, ParseIt end)
		{
			if (it == end || isTerminated_ || terminator_.empty() || *it != terminator_)
				return;

			isTerminated_ = true;
			++it;
		}

		bool predictLongOption(const ParseIt& it, ParseIt end) const
		{
			return it != end
				&& !isTerminated_
				&& !longPrefix_.empty()
				&& it->size() > longPrefix_.size()
				&& it->compare(0, longPrefix_.size(), longPrefix_) == 0;
		}

		void parseLongOption(ParseIt& it, ParseIt end)
		{
			if (!predictLongOption(it, end))
				return;

			const std::string& token{*it++};
			auto nameIt{std::next(token.begin(), longPrefix_.size())};
			auto sepIt{std::find(nameIt, token.end(), longSeparator_)};

			Arg* option{getOption({nameIt, sepIt})};
			if (option->hasValue())
			{
				if (sepIt != token.end())
					option->parse({std::next(sepIt), token.end()});
				else
				{
					if (it == end)
						throw Error{"Cannot find value for option: " + token};
					option->parse(*it++);
				}
			}
			else
				if (sepIt != token.end())
					throw Error{"Unexpected option value: " + token};
			option->done();
		}

		bool predictShortOption(const ParseIt& it, ParseIt end) const
		{
			return it != end
				&& !isTerminated_
				&& it->size() > 1
				&& it->front() == shortPrefix_;
		}

		void parseShortOptions(ParseIt& it, ParseIt end)
		{
			if (!predictShortOption(it, end))
				return;

			const std::string& token{*it++};
			auto nameIt{std::next(token.begin())};

			while (nameIt != token.end())
			{
				Arg* option{getOption(*nameIt++)};
				if (option->hasValue())
				{
					if (nameIt != token.end())
					{
						option->parse(std::string{nameIt, token.end()});
						nameIt = token.end();
					}
					else
					{
						if (it == end)
							throw Error{"Cannot find value for option: " + token};
						option->parse(*it++);
					}
				}
				option->done();
			}
		}

		void parseOperands(ParseIt& it, ParseIt end)
		{
			for (auto& operand : operands_)
				parseOperandContent(it, end, operand.get());
		}

		void parseOperandContent(ParseIt& it, ParseIt end, Arg* operand)
		{
			while (true)
			{
				parseTerminator(it, end);
				if (it == end)
					break;

				if (predictLongOption(it, end) || predictShortOption(it, end))
					throw Error{"Unexpected option: " + *it};

				operand->parse(*it++);
				operand->done();
				if (!operand->isSink())
					break;
			}
		}

		void checkEnd(ParseIt& it, ParseIt end) const
		{
			if (it != end)
				throw Error{"Unexpected argument: " + *it};
		}

		void checkRequired(const std::vector<ArgPtr>& args) const
		{
			for (const auto& arg : args)
				if (arg->isRequired() && !arg->isDone())
					throw Error{"Cannot find required argument: " + expandName(arg.get())};
		}

		std::string expandName(const Arg* arg) const
		{
			if (arg->getShortName() != 0)
				return std::string{shortPrefix_} + arg->getShortName();
			if (!arg->getLongName().empty())
				return longPrefix_ + arg->getLongName();
			return arg->getValueName();
		}

		Arg* getOption(const std::string& name)
		{
			if (!name.empty())
				for (auto& option : options_)
					if (option->getLongName() == name)
						return option.get();
			throw Error{"Unknown option name: " + name};
		}

		Arg* getOption(char name)
		{
			if (name != 0)
				for (auto& option : options_)
					if (option->getShortName() == name)
						return option.get();
			throw Error{"Unknown option name: " + std::string{name}};
		}

		// ---- Write ----

		friend std::ostream& operator<<(std::ostream&, const Parser&);

		void writeHelp(std::ostream& out) const
		{
			writeParagraph(out, helpProlog_);
			writeUsage(out);
			writeGlossary(out, optionsTitle_, options_);
			writeGlossary(out, operandsTitle_, operands_);
			writeParagraph(out, helpEpilog_);
		}

		void writeParagraph(std::ostream& out, const std::string& paragraph) const
		{
			if (paragraph.empty())
				return;

			writeWrapped(out, tokenize(paragraph), 0, 0);
			out << "\n\n";
		}

		void writeUsage(std::ostream& out) const
		{
			if (usageTitle_.empty())
				return;

			std::vector<std::string> tokens{};

			if (!utilityName_.empty())
				tokens.push_back(utilityName_);

			if (!optionsUsage_.empty())
				tokens.push_back(optionsUsage_);
			else
				pushUsageTokens(tokens, options_);

			if (!operandsUsage_.empty())
				tokens.push_back(operandsUsage_);
			else
				pushUsageTokens(tokens, operands_);

			out << usageTitle_ << '\n' << std::string(helpIndent_, ' ');
			writeWrapped(out, tokens, helpIndent_, helpIndent_*2);
			out << "\n\n";
		}

		void pushUsageTokens(
			std::vector<std::string>& tokens,
			const std::vector<ArgPtr>& args) const
		{
			for (const auto& arg : args)
			{
				std::string token{};

				if (arg->getShortName() != 0)
				{
					token += shortPrefix_;
					token += arg->getShortName();
				}
				else if (!arg->getLongName().empty())
				{
					token += longPrefix_;
					token += arg->getLongName();
				}

				if (arg->hasValue())
				{
					if (!token.empty())
						token += ' ';
					token += arg->getValueName();
				}

				if (!arg->isRequired())
					token = '[' + token + ']';

				if (arg->isSink())
					token += "...";

				tokens.push_back(std::move(token));
			}
		}

		void writeGlossary(
			std::ostream& out,
			const std::string& title,
			const std::vector<ArgPtr>& args) const
		{
			if (title.empty() || args.empty())
				return;

			struct Entry
			{
				std::string term;
				std::vector<std::string> description;
			};

			const bool hasAnyShort{hasAnyShortName(args)};
			std::vector<Entry> entries{};
			StringSize maxTermSize{0};

			for (const auto& arg : args)
			{
				Entry entry{{}, tokenize(arg->getDescription())};

				if (hasAnyShort)
				{
					if (arg->getShortName() == 0)
						entry.term += "  ";
					else
					{
						entry.term += shortPrefix_;
						entry.term += arg->getShortName();
					}
				}

				if (!arg->getLongName().empty())
				{
					if (hasAnyShort)
						entry.term += (arg->getShortName() != 0) ? ", " : "  ";
					entry.term += longPrefix_;
					entry.term += arg->getLongName();
				}

				if (arg->hasValue())
				{
					if (!entry.term.empty())
						entry.term += ' ';
					entry.term += arg->getValueName();
				}

				if (!defaultIntro_.empty())
				{
					std::string value{arg->getDefaultValue()};
					if (!value.empty())
						entry.description.push_back('(' + defaultIntro_ + value + ')');
				}

				if (entry.term.size() > maxTermSize)
					maxTermSize = entry.term.size();

				entries.push_back(std::move(entry));
			}

			out << title << '\n';

			const StringSize tab{helpIndent_ + maxTermSize + helpIndent_};
			for (const Entry& entry : entries)
			{
				out << std::string(helpIndent_, ' ') << entry.term <<
					std::string(tab - helpIndent_ - entry.term.size(), ' ');

				writeWrapped(out, entry.description, tab, tab);
				out << '\n';
			}

			out << '\n';
		}

		bool hasAnyShortName(const std::vector<ArgPtr>& args) const
		{
			for (const auto& arg : args)
				if (arg->getShortName() != 0)
					return true;
			return false;
		}

		std::vector<std::string> tokenize(const std::string& text) const
		{
			std::vector<std::string> tokens{};
			auto it{text.begin()};

			while (it != text.end())
			{
				if (*it == ' ')
				{
					++it;
					continue;
				}

				if (*it == '\n')
				{
					tokens.emplace_back("\n");
					++it;
					continue;
				}

				auto start{it};
				while (it != text.end() && *it != ' ' && *it != '\n')
					++it;
				tokens.emplace_back(start, it);
			}

			return tokens;
		}

		void writeWrapped(
			std::ostream& out,
			const std::vector<std::string>& tokens,
			StringSize initialPos,
			StringSize hangingIndent) const
		{
			StringSize pos{initialPos};
			StringSize spaces{0};

			for (const std::string& token : tokens)
			{
				const bool isNewline{token == "\n"};
				const bool isOverflow{pos + spaces + token.size() > helpWidth_};

				if (isNewline || (isOverflow && pos > hangingIndent))
				{
					out << '\n';
					pos = 0;
					spaces = hangingIndent;

					if (isNewline)
						continue;
				}

				out << std::string(spaces, ' ') << token;
				pos += spaces + token.size();
				spaces = 1;
			}
		}
};


inline std::ostream& operator<<(std::ostream& out, const Parser& parser)
{
	parser.writeHelp(out);
	return out;
}


} // namespace detail


// Public types
using detail::Parser;
using detail::Error;
using detail::Signal;


} // namespace minarg

#endif // MINARG_MINARG_HPP_INCLUDED
