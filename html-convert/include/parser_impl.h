#pragma once

#include "parser.h"
#include "error-handler.h"

namespace html { namespace impl {

	template<typename Iterator>
	indesign_grammar<Iterator>::indesign_grammar() : indesign_grammar::base_type(document)
	{
		using qi::lit;
		using qi::eps;
        using qi::lexeme;
        using ascii::char_;
        using ascii::string;
        using namespace qi::labels;

		text         %= lexeme[+(!lit('<') >> char_)];
		tag_name     %= lexeme[+(!lit(':') >> char_)];
		unicode_char %= lexeme[lit('<') >> '0' >> char_("xX") >> +char_("0-9a-hA-H") > lit('>')];
		tag_content  %= lexeme[+(
								( char_('\\') >> char_) // accept any character preceded by '\' (escape sequence)
								| unicode_char // accept unicode characters
								| (!lit('>') >> !lit('<') >> !lit('=') >> char_))]; // accept any character other than <, >, =

		tag = 
				lit('<') 
				>> eps[set_type(_val, Tag)] 
				>> tag_name[add_value(_val, _1, TagName)] 
				>> ':' 
				>> -tag_content[add_value(_val, _1, TagValue)] >> -lit('=') >> *tag[add_node(_val, _1)]
				> '>';

		document = 
				eps[set_type(_val, Root)] 
				>  *( text[add_value(_val, _1, Text)] 
					| unicode_char[add_value(_val, _1, UnicodeChar)]
					| tag[add_node(_val, _1)] 
					);

		document.name("document");
		tag.name("tag");
		text.name("text");
		tag_name.name("tag-name");
		tag_content.name("tag-content");
		unicode_char.name("unicode-char");

		qi::on_error<qi::fail>(document, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
	}

}}
