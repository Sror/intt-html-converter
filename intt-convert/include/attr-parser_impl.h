#pragma once

#include "attr-parser.h"
#include "error-handler.h"

namespace intt { namespace impl {

	template<typename Iterator>
	attr_grammar<Iterator>::attr_grammar() 
		: attr_grammar::base_type(result)
	{
		using qi::lit;
		using qi::eps;
        using qi::lexeme;
        using ascii::char_;
        using ascii::string;
        using namespace qi::labels;

		result = eps[set_type(_val, Attributes)] >> *(attr[add_node(_val, _1)]);

		attr = 
				// double quotes
				(eps[set_type(_val, AttrName)] >> string("style")[set_value(_val, _1)] >> -sp >> '=' 
					>> '\"' >> (style_elem[add_node(_val, _1)] % ';') >> -lit(';') >> -sp > '\"' >> -sp)
				// single quotes
				| (eps[set_type(_val, AttrName)] >> string("style")[set_value(_val, _1)] >> -sp >> '=' 
					> '\'' >> (style_elem_2[add_node(_val, _1)] % ';') >> -lit(';') >> -sp > '\'' >> -sp)

				| (eps[set_type(_val, AttrName)] >> identifier[set_value(_val, _1)] >> -sp > '=' > attr_val[add_node(_val, _1)] >> -sp);
		
		attr_val = 
				eps[set_type(_val, AttrValue)] >> -sp >> '\"' > content[set_value(_val, _1)] > '\"'
				| eps[set_type(_val, AttrValue)] >> -sp >> '\'' > content_2[set_value(_val, _1)] > '\'';
		
		style_elem = 
				// double quotes
				eps[set_type(_val, Style)] >> -sp 
					>> identifier[set_value(_val, _1)] >> -sp 
					> ':' >> -sp 
					> style_content[add_value(_val, _1, Text)] >> -sp;
		 
		style_elem_2 = 
				// single quotes
				eps[set_type(_val, Style)] >> -sp 
					>> identifier[set_value(_val, _1)] >> -sp 
					> ':' >> -sp 
					> style_content_2[add_value(_val, _1, Text)] >> -sp;
		
		identifier %= lexeme[+char_("0-9A-Za-z_@-")];
		
		style_content %= lexeme[+(!lit(';') >> !lit('\"') >> char_)];
		style_content_2 %= lexeme[+(!lit(';') >> !lit('\'') >> char_)];
		
		content %= lexeme[+(char_ - '\"')];
		content_2 %= lexeme[+(char_ - '\'')];
		
		sp = +char_(" \t\n");
		
		result.name("result");
		qi::on_error<qi::fail>(result, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(result);

		attr.name("attribute");
		//qi::on_error<qi::fail>(attr, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(attr);

		attr_val.name("attribute-name");
		//qi::on_error<qi::fail>(attr_val, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(attr_val);

		style_elem.name("double-quoted-style-element");
		//qi::on_error<qi::fail>(style_elem, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(style_elem);

		style_elem_2.name("single-quoted-style-element");
		//qi::on_error<qi::fail>(style_elem_2, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(style_elem_2);

		identifier.name("identifier");
		//qi::on_error<qi::fail>(identifier, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(identifier);

		style_content.name("double-quoted-style-content");
		//qi::on_error<qi::fail>(style_content, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(style_content);

		style_content_2.name("single-quoted-style-content");
		//qi::on_error<qi::fail>(style_content_2, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(style_content_2);

		content.name("content");
		//qi::on_error<qi::fail>(content, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(content);

		content_2.name("content-2");
		//qi::on_error<qi::fail>(content, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(content_2);

		sp.name("space");
		//qi::on_error<qi::fail>(sp, err_handler(qi::_1, qi::_2, qi::_3, qi::_4));
		//BOOST_SPIRIT_DEBUG_NODE(space);
	}

}}
