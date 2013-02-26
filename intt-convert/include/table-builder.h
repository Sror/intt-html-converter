#pragma once

#include <string>
#include <vector>

namespace intt { namespace impl {

#if 0
	struct TabCell
	{
		std::vector<std::string> classes;
		std::vector<std::string> styles;
		std::string content;
		bool isHeader;
	};

	struct TabCol
	{
		double width;
	};

	struct TabRow
	{
		double height;
		std::vector<TabCell> cells;
	};

	struct Table
	{
		std::vector<TabCol> columns;
		std::vector<TabRow> rows;

		Table(size_t cols, size_t rows);
		void addCell(const TabCell& cell);
	};
#endif

}}
