#pragma once

#include <map>
#include <stack>
#include <string>
#include <iostream>
#include <algorithm>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "table-builder.h"
#include "ast.h"

namespace intt { namespace impl {

	class Builder
	{
	public:
		Builder(const AstNode& document, 
			const std::map<std::string, std::string>& classes, 
			const std::map<std::string, std::string>& colors) 
			: document(document), classes(classes), colors(colors) {
			init();
			reset();
		}

		void build();
		void reset();
		std::string getResult() const { return os.str(); }

	private:
		struct Attr
		{
			std::string class_;
			std::map<std::string, std::string> styles;
		};

		void print(const std::string& s);
		void print_style(const std::string& style, const std::string& name);
		void build_impl(boost::shared_ptr<AstNode> node);

		std::string build_tag(boost::shared_ptr<AstNode> node);
		
		std::string build_table(boost::shared_ptr<AstNode> node);
		size_t build_row(boost::shared_ptr<AstNode> node);
		size_t build_cell(boost::shared_ptr<AstNode> node);

		Attr build_style(boost::shared_ptr<AstNode> node);

		std::string build_color(const std::string& str);
		std::string build_entity(std::string str);

		AstNode document;
		std::ostringstream os;
		std::ostringstream os_tab;

		std::stack<std::string> st;
		bool typefaceBold, typefaceItalic;
		bool tab;

		const std::map<std::string, std::string>& classes;
		const std::map<std::string, std::string>& colors;
		static std::map<std::string, std::string> colorNames;
		static std::map<std::string, std::string> entities;
		static const char keyMap[128];
		static void init();

		static std::map<std::string, std::stack<std::string> > tags;
	};

}}
