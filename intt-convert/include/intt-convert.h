#pragma once

#include <string>

namespace intt {

	struct Options
	{
		bool printAST; // prints document AST to stderr
	};

	struct Result
	{
		std::string document; // contains HTML document
		std::string error; // contains error message, if any
		std::string ast; // contains AST tree
	};

	// Converts HTML format string into InTT.
	// param input: string containing HTML document
	// param result: parsing result
	// param opt: parsing options
	void convert(const std::wstring& input, Result& result, const Options& opt);

}
