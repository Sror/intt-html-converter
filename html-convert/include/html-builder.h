#pragma once

#include <stack>
#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#include "builder.h"
#include "style-builder.h"
#include "ast.h"

namespace html { namespace impl {

	class HtmlBuilder : public Builder
	{
	public:
		HtmlBuilder(const AstNode& document, const StyleBuilder& styles) : Builder(document), styles(styles) {}

		virtual void reset();
		virtual void build();

	private:
		void build_impl(boost::shared_ptr<AstNode> node);
		virtual void print(const std::string& s);

		std::stack<std::string> charStyleStack;
		std::stack<std::string> parStyleStack;
		bool emptyParagraph;
		std::string originalStyles;

		const StyleBuilder& styles;

		void print_tag(boost::shared_ptr<AstNode> node);

		void flush_par_style();
		void end_par_style();
		void end_char_style();

		struct TabCell
		{
			std::string className;
			std::string origName;
			std::vector<std::string> styles;

			std::string content;

			size_t row;
			size_t col;
		};

		struct TabCol
		{
			std::string className;
			std::string origName;
			std::vector<std::string> styles;
		};

		struct TabRow
		{
			TabRow() : header(false) {}
			std::string className;
			std::string origName;
			std::vector<std::string> styles;

			bool header;
			std::vector<TabCell> cells;
		};

		struct Table
		{
			Table() : colPtr(0), rowPtr(0), cellCounter(0) {}

			std::string className;
			std::string origName;
			std::vector<std::string> styles;

			size_t colPtr, rowPtr;
			std::vector<TabCol> cols;
			std::vector<TabRow> rows;

			size_t colCount;
			size_t rowCount;

			size_t header;
			size_t footer;

			size_t cellCounter;
		};

		boost::optional<Table> tableStyleDefined;
		std::stack<Table> tableStack;
		
		boost::optional<TabCell> tabCellStyleDefined;
		std::stack<TabCell> tabCellStack;

		void add_tab_row(boost::shared_ptr<AstNode> node);
		void add_tab_col(boost::shared_ptr<AstNode> node);
		void add_tab_cell(boost::shared_ptr<AstNode> node);
		void print_table();

		struct ListElement
		{
			std::string className;
			std::string origName;
			std::vector<std::string> styles;

			std::string content;
		};

		struct List
		{
			std::string className;
			std::string origName;
			std::vector<std::string> styles;

			bool numbered;

			std::vector<ListElement> elements;
		};

		std::stack<ListElement> lstElemStack;
		std::stack<List> listStack;
	};

}}
