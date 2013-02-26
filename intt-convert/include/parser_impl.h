#pragma once

#include "parser.h"
#include "error-handler.h"

namespace intt { namespace impl {

	template<typename Iterator>
	html_grammar<Iterator>::html_grammar() 
		: html_grammar::base_type(document)
	{
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
		//BOOST_SPIRIT_DEBUG_NODE(document);

		tag_content.name("tag-content");
		//BOOST_SPIRIT_DEBUG_NODE(tag_content);

		tag.name("tag");
		//BOOST_SPIRIT_DEBUG_NODE(tag);

		tag_end.name("tag-end");
		//BOOST_SPIRIT_DEBUG_NODE(tag_end);

		entity.name("entity");
		//BOOST_SPIRIT_DEBUG_NODE(entity);

		text.name("text");
		//BOOST_SPIRIT_DEBUG_NODE(text);

		entity_value.name("entity-value");
		//BOOST_SPIRIT_DEBUG_NODE(entity_value);

		tag_name.name("tag-name");
		//BOOST_SPIRIT_DEBUG_NODE(tag_name);

		sp.name("space");
		//BOOST_SPIRIT_DEBUG_NODE(sp);

		comment.name("comment");
		//BOOST_SPIRIT_DEBUG_NODE(comment);

		qi::on_error<qi::fail>(document, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(tag_content, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(tag, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(tag_end, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(entity, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(text, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(entity_value, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(tag_name, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(sp, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//qi::on_error<qi::fail>(comment, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
	}

}}
