#pragma once

#include "parser.h"
#include "error-handler.h"

namespace intt { namespace impl {

	template<typename Iterator>
	html_grammar<Iterator>::html_grammar() : html_grammar::base_type(document) {
		using qi::lit;
		using qi::eps;
        using qi::lexeme;
        using ascii::char_;
        using ascii::string;
        using namespace qi::labels;

		document = eps[set_type(_val, Document)] 
				> *tag_content[add_node(_val, _1)] >> -sp;

		tag_content = 
				comment 
				| tag[_val = _1] 
				| entity[_val = _1] 
				| text[set_type(_val, Text), set_value(_val, _1)];
		
		tag =
				"<" >> !char_('/') >> eps[set_type(_val, Tag)] >> -sp > tag_name[_a = _1, set_value(_val, _1)] >> -sp 
					>> -attributes[add_node(_val, _1)] >> -sp 
					> (
						"/>"
						| ('>' >> *tag_content[add_node(_val, _1)] > tag_end(_a))
					);
		
		tag_end = "</" >> -sp > string(_r1) >> -sp > '>';
		
		entity = lit('&') >> eps[set_type(_val, Entity)] > entity_value[set_value(_val, _1)] > ';';
		
		text %= lexeme[+(!lit('<') >> !lit('&') >> char_)];
		
		entity_value %= lexeme[+char_("0-9A-Za-z#")];
		
		tag_name %= lexeme[+char_("0-9A-Za-z")];
		
		sp = +char_(" \t\n");

		comment = 
				("<!DOCTYPE" >> +(char_ - '>') > '>')
				| ("<!--" >> *(char_ - "-->") > "-->");

		document.name("document");
		tag_content.name("tag-content");
		tag.name("tag");
		tag_end.name("tag-end");
		entity.name("entity");
		text.name("text");
		entity_value.name("entity-value");
		tag_name.name("tag-name");
		sp.name("space");
		comment.name("comment");
		qi::on_error<qi::fail>(document, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
	}

}}
