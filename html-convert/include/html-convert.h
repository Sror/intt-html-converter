#pragma once

#include <string>

namespace html {

	struct Options
	{
		bool printStyleAST; // prints style AST to stderr
		bool printDocumentAST; // prints document AST to stderr
	};

	struct Result
	{
		std::string definitions; // contains raw definitions
		std::string originalStyles; // contains original InDesign style names in format sanitized_name:original name
		std::string colorTable; // contains colors from ColorTable in format #000000:color_name
		std::string style; // contains CSS styles parsed from definitions
		std::string document; // contains HTML document
		std::string error; // contains error message, if any
	};

	// Converts InTT format string into HTML.
	// param input: string containing InDesign Tagged Text
	// param result: parsing result
	// param opt: parsing options
	void convert(const std::wstring& input, Result& result, const Options& opt);

}
