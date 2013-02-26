#include <sstream>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "html-convert.h"
#include "html-builder.h"
#include "style-builder.h"
#include "parser.h"

namespace html
{

	enum ConvertType
	{
		ToHTML,
		ToInTT
	};

	std::string convertFromWide(const std::wstring& input, ConvertType type) {
		std::ostringstream output;
		for (std::wstring::const_iterator i = input.begin(); i != input.end(); ++i) {
			if (i == input.begin() && (*i == 0xFFFE || *i == 0xFEFF)) {
				continue;
			}

			if (*i < 0x80) {
				output << (char)(*i);
			} else {
				switch (type) {
				case ToHTML:
					output << boost::format("&x%|1$04X|;") % (int)(*i);
					break;
				case ToInTT:
					output << boost::format("<0x%|1$04X|>") % (int)(*i);
					break;
				}
			}
		}

		return output.str();
	}

	void convert(const std::wstring& input, Result& result, const Options& opt) {
		typedef impl::indesign_grammar<std::string::const_iterator> indesign_grammar;

		std::string input2 = convertFromWide(input, ToInTT);

		indesign_grammar grammar;
		impl::AstNode doc;
		impl::AstNode style;

		// split into header (style section) and content
		std::string::size_type start = input2.find("<ParaStyle:");
		if (start == std::string::npos) {
			result.error = "Cannot find start of document body.";
			return;
		}

		std::string header = input2.substr(0, start);
		std::string content = input2.substr(start);

		result.definitions = header;

		// remove encoding header
		start = header.find_first_not_of('<');
		start = header.find_first_of('>', start);
		header = header.substr(start+1);

		bool parseSucceeded;

		// parse styles
		std::string::const_iterator styleIter = header.begin();
		std::string::const_iterator styleEnd = header.end();
		parseSucceeded = parse(styleIter, styleEnd, grammar, style);

		impl::StyleBuilder sBuilder(style);

		if (parseSucceeded && styleIter == styleEnd) {
			// build styles
			sBuilder.build();
			result.style = sBuilder.getResult();
			result.originalStyles = sBuilder.getOriginalStyles();
			result.colorTable = sBuilder.getColorTable();
		} else {
			std::string::const_iterator some = styleIter + 30;
			std::string context(styleIter, (some > styleEnd) ? styleEnd : some);
			std::ostringstream tmp;
			tmp << "Parsing style failed. Stopped at: \'" << context << "...\'.";
			result.error = tmp.str();
		}

		if (opt.printStyleAST) {
			std::cerr << "\nStyles AST\n";
			impl::AstNode::print_preorder(&style, std::cerr);
			std::cerr << "\n";
		}

		// parse document
		std::string::const_iterator docIter = content.begin();
		std::string::const_iterator docEnd = content.end();
		parseSucceeded = parse(docIter, docEnd, grammar, doc);

		if (parseSucceeded && docIter == docEnd) {
			// build document
			impl::HtmlBuilder docBuilder(doc, sBuilder);
			docBuilder.build();
			result.document = docBuilder.getResult();
		} else {
			std::string::const_iterator some = docIter + 30;
			std::string context(docIter, (some > docEnd) ? docEnd : some);
			std::ostringstream tmp;
			tmp << "Parsing document failed. Stopped at: \'" << context << "...\'.";
			result.error = tmp.str();
			result.document = tmp.str();
		}

		if (opt.printDocumentAST) {
			std::cerr << "\nDocument AST\n";
			impl::AstNode::print_preorder(&doc, std::cerr);
			std::cerr << "\n";
		}
	}

}
