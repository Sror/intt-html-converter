#pragma once

#include <iostream>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "ast.h"

namespace intt { namespace impl {

	namespace phx = boost::phoenix;
	namespace spirit = boost::spirit;
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	template<typename Iterator>
	struct attr_grammar : qi::grammar<Iterator, AstNode()>
	{
		attr_grammar();

		qi::rule<Iterator, AstNode()> result;
		qi::rule<Iterator, AstNode()> attr, attr_val, style_elem, style_elem_2;

		qi::rule<Iterator, std::string()> content, content_2, style_content, style_content_2;
		qi::rule<Iterator, std::string()> identifier;

		qi::rule<Iterator> sp;

		phx::function<node_adder> add_node;
		phx::function<node_type_setter> set_type;
		phx::function<node_value_setter> set_value;
		phx::function<value_adder> add_value;
	};

}}
