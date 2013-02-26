#pragma once

#include <iostream>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include "ast.h"

namespace html { namespace impl {

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	namespace phx = boost::phoenix;

	struct node_adder
	{
		template<typename, typename>
		struct result { typedef void type; };

		void operator()(AstNode& node, const AstNode& child) const {
			node.add_child(boost::shared_ptr<AstNode>(new AstNode(child)));
		}
	};

	struct value_adder
	{
		template<typename, typename, typename>
		struct result { typedef void type; };

		void operator()(AstNode& node, const std::string& text, NodeType type) const {
			boost::shared_ptr<AstNode> n(new AstNode(text));
			n->data().type = type;
			node.add_child(n);
		}
	};

	struct node_type_setter
	{
		template<typename, typename>
		struct result { typedef void type; };

		void operator()(AstNode& node, NodeType type) const {
			node.data().type = type;
		}
	};

	template<typename Iterator>
	struct indesign_grammar : qi::grammar<Iterator, AstNode()>
	{
		indesign_grammar();

		qi::rule<Iterator, AstNode()> document;
		qi::rule<Iterator, AstNode()> tag;
		qi::rule<Iterator, std::string()> text;
		qi::rule<Iterator, std::string()> tag_name;
		qi::rule<Iterator, std::string()> tag_content;
		qi::rule<Iterator, std::string()> unicode_char;

		phx::function<node_adder> add_node;
		phx::function<node_type_setter> set_type;
		phx::function<value_adder> add_value;
	};

}}
