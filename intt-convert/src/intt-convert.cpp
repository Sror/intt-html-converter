#include <sstream>
#include <string>
#include <boost/format.hpp>

#include "header.h"
#include "parser.h"
#include "builder.h"
#include "intt-convert.h"

namespace intt
{

	enum ConvertType
	{
		ToHTML,
		ToInTT
	};

	std::string convertFromWide(const std::wstring& input, ConvertType type) {
		std::ostringstream output;
		for (auto i = input.begin(); i != input.end(); ++i) {
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
		typedef impl::html_grammar<std::string::const_iterator> html_grammar;

		std::cerr << "Converting Unicode characters\n";
		std::string input2 = convertFromWide(input, ToHTML);

		std::cerr << "Parsing headers\n";
		impl::Header hdr = impl::getHeader(input2);
		if (hdr.corrupted) {
			//result.error = "Error while parsing headers.";
			//return;
		}
		input2 = input2.substr(hdr.position != std::string::npos ? hdr.position : 0);

		html_grammar grammar;
		impl::AstNode doc;

		bool parseSucceeded;
		
		// parse document
		std::cerr << "Parsing document\n";
		std::string::const_iterator docIter = input2.begin();
		std::string::const_iterator docEnd = input2.end();
		parseSucceeded = parse(docIter, docEnd, grammar, doc);

		if (parseSucceeded && docIter == docEnd) {
			// build document
			std::cerr << "Building document\n";
			impl::Builder builder(doc, hdr.classDef, hdr.colorDef);
			builder.build();
			result.document = hdr.rawDef;
			if (result.document.empty()) {
				result.document = "<ASCII-WIN>\n";
			}

			result.document += builder.getResult();
			std::ostringstream os;
			impl::AstNode::print_preorder(&doc, os);
			result.ast = os.str();
		} else {
			std::string::const_iterator some = docIter + 30;
			std::string context(docIter, (some > docEnd) ? docEnd : some);
			std::ostringstream tmp;
			tmp << "Parsing document failed. Stopped at: \'" << context << "...\'.";
			result.error = tmp.str();
			std::cerr << tmp.str() << '\n';
		}
	}

}
