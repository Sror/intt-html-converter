#pragma once

#include <iostream>

#include "ast.h"
#include "attr-parser.h"

namespace intt { namespace impl {

	namespace phx = boost::phoenix;
	namespace spirit = boost::spirit;
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	template<typename Iterator>
	struct html_grammar : qi::grammar<Iterator, AstNode()>
	{
		html_grammar();

		attr_grammar<Iterator> attributes;

		qi::rule<Iterator, AstNode()> document, tag_content;
		qi::rule<Iterator, AstNode(), qi::locals<std::string> > tag;
		
		qi::rule<Iterator, AstNode()> entity;
		qi::rule<Iterator, std::string()> text;
		qi::rule<Iterator, std::string()> entity_value;
		qi::rule<Iterator, std::string()> tag_name;
		qi::rule<Iterator, void(std::string)> tag_end;
		qi::rule<Iterator> comment;

		qi::rule<Iterator> sp;

		phx::function<node_adder> add_node;
		phx::function<node_type_setter> set_type;
		phx::function<node_value_setter> set_value;
		phx::function<value_adder> add_value;
	};

}}
