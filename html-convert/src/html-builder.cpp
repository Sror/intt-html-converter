#include "html-builder.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace html { namespace impl {

	void HtmlBuilder::reset() {
		Builder::reset();

		emptyParagraph = false;

		while (!charStyleStack.empty())
			charStyleStack.pop();
		while (!parStyleStack.empty())
			parStyleStack.pop();

		while (!tableStack.empty())
			tableStack.pop();
		while (!tabCellStack.empty())
			tabCellStack.pop();
	}

	void HtmlBuilder::build() {
		size_t i = 0;
		while (document.child(i))
			build_impl(document.child(i++));
		end_char_style();
		end_par_style();
	}

	void HtmlBuilder::build_impl(boost::shared_ptr<AstNode> node) {
		if (!node) {
			return;
		}

		switch (node->data().type) {
		case Tag:
			print_tag(node);
			break;

		case Text: {
				flush_par_style();
				std::string s = node->data().value;
				bool empty = true;
				for (std::string::const_iterator i = s.begin(); i != s.end(); ++i)
					if (std::isgraph(*i))
						empty = false;
				if (empty)
					s = "&nbsp;";

				print(s);
			}
			break;

		case UnicodeChar:
			print("&#" + node->data().value + ";");
			break;
		}
	}

	void HtmlBuilder::print_tag(boost::shared_ptr<AstNode> node) {
		if (node->data().type != Tag) {
			return;
		}

		std::string name = node->child(0)->data().value;
		std::string value;
		if (node->child(1)) {
			value = node->child(1)->data().value;
		}
		if (name == "TableStyle") {
			end_par_style();
			end_char_style();

			Table tab;
			tab.className = Builder::sanitizeString(value);
			tab.origName = value;
			tableStyleDefined.reset(tab);
		} else if (name == "CellStyle") {
			end_par_style();
			end_char_style();

			TabCell cell;
			cell.className = Builder::sanitizeString(value);
			cell.origName = value;
			tabCellStyleDefined.reset(cell);
		} else if (name == "TableStart") {
			end_par_style();
			end_char_style();

			Table tab;
			if (tableStyleDefined) {
				tab = tableStyleDefined.get();
				tableStyleDefined.reset();
			}

			size_t end = value.find_last_of('<');
			value = value.substr(0, end);

			std::vector<std::string> data;
			boost::split(data, value, boost::is_any_of(",:"));

			if (data.size() >= 4) {
				tab.footer = boost::lexical_cast<int>(data[3]);
			} if (data.size() >= 3) {
				tab.header = boost::lexical_cast<int>(data[2]);
			} if (data.size() >= 2) {
				tab.colCount = boost::lexical_cast<int>(data[1]);
				tab.cols.resize(tab.colCount);
			}
			
			if (data.size() >= 1) {
				tab.rowCount = boost::lexical_cast<int>(data[0]);
			}
			
			size_t i = 2;
			while (node->child(i)) {
				boost::shared_ptr<AstNode> n = node->child(i);

				if (n->data().type != Tag) {
					continue;
				}

				// TODO implement table styles
				std::string name = n->child(0)->data().value;
				std::string value = n->child(1)->data().value;

				originalStyles += "<" + name + ":" + value + ">";

				++i;
			}

			tableStack.push(tab);
		} else if (name == "TableEnd") {
			end_par_style();
			end_char_style();

			print_table();
		} else if (name == "CellStart") {
			add_tab_cell(node);
		} else if (name == "CellEnd") {
			end_par_style();
			end_char_style();
		} else if (name == "ColStart") {
			add_tab_col(node);
		} else if (name == "RowStart") {
			add_tab_row(node);
		} else if (name == "ParaStyle") {
			std::ostringstream os2;

			end_par_style();

			while (!parStyleStack.empty()) {
				os2 << parStyleStack.top();
				parStyleStack.pop();
			}

			std::ostringstream tmp;
			if (!value.empty()) {
				tmp << "<p class=\"" << Builder::sanitizeString(value) << "\"";
				parStyleStack.push(tmp.str());
			} else {
				parStyleStack.push("<p");
			}
			print(os2.str());
		} else if (name == "pTextAlignment") {
			if (value.empty()) {
				flush_par_style();
				return;
			}

			if (value == "Left") {
				parStyleStack.push("text-align: left; ");
			} else if (value == "Right") {
				parStyleStack.push("text-align: right; ");
			} else if (value == "Center") {
				parStyleStack.push("text-align: center; ");
			} else if (value == "Justify" || value == "JustifyCenter" || value == "JustifyRight" || value == "JustifyFull") {
				parStyleStack.push("text-align: justify; ");
			}
		} else if (name == "pSpaceBefore") {
			if (value.empty()) {
				flush_par_style();
				return;
			}
			parStyleStack.push("margin-top: " + value + "pt; ");
		} else if (name == "pSpaceAfter") {
			if (value.empty()) {
				flush_par_style();
				return;
			}
			parStyleStack.push("margin-bottom: " + value + "pt; ");
		} else if (name == "pLeftIndent") {
			if (value.empty()) {
				flush_par_style();
				return;
			}
			parStyleStack.push("margin-left: " + value + "pt; ");
		} else if (name == "pRightIndent") {
			if (value.empty()) {
				flush_par_style();
				return;
			}
			parStyleStack.push("margin-right: " + value + "pt; ");
		} else if (name == "pFirstLineIndent") {
			if (value.empty()) {
				flush_par_style();
				return;
			}
			parStyleStack.push("text-indent: " + value + "pt; ");
		} else if (name == "CharStyle") {
			flush_par_style();

			std::ostringstream os2;

			if (value.empty()) {
				end_char_style();
				return;
			}

			charStyleStack.push("</span>");
			os2 << "<span class=\"" << Builder::sanitizeString(value) << "\">";
			
			print(os2.str());
		} else if (name == "cSize") {
			flush_par_style();

			std::ostringstream os2;

			if (value.empty()) {
				end_char_style();
				return;
			}

			float size = boost::lexical_cast<float>(value);
			charStyleStack.push("</span>");
			os2 << "<span style=\"font-size: " << size << "pt; \">";
			
			print(os2.str());
		} else if (name == "cTypeface") {
			flush_par_style();

			std::ostringstream os2;

			if (value.empty()) {
				end_char_style();
				return;
			}

			if (value == "Bold") {
				charStyleStack.push("</strong>");
				os2 << "<strong>";
			} else if (value == "Italic") {
				charStyleStack.push("</em>");
				os2 << "<em>";
			}
			
			print(os2.str());
		} else if (name == "cFont") {
			flush_par_style();

			if (value.empty()) {
				end_char_style();
				return;
			}

			charStyleStack.push("</span>");
			print("<span style=\"font-family: \'" + value + "\';\">");
		} else if (name == "cColor") {
			flush_par_style();

			if (value.empty()) {
				end_char_style();
				return;
			}

			charStyleStack.push("</span>");
			std::string color = styles.getColor(Builder::sanitizeString(value));
			if (color.empty()) {
				color = Builder::convertColor(value);
			}
			print("<span style=\"color: " + color + ";\">");
		} else {
			originalStyles += "<" + name + ":" + value + ">";
		}
	}

	void HtmlBuilder::add_tab_col(boost::shared_ptr<AstNode> node) {
		if (tableStack.empty()) {
			return;
		}

		TabCol col = tableStack.top().cols[tableStack.top().colPtr];

		size_t i = 1;
		while (node->child(i)) {
			boost::shared_ptr<AstNode> tag = node->child(i);
			if (tag->data().type != Tag) {
				continue;
			}

			std::string name = tag->child(0)->data().value;
			std::string value = tag->child(1)->data().value;

			if (name == "tColAttrWidth") {
				col.styles.push_back("width: " + value + "pt");
			}
			++i;
		}

		tableStack.top().cols[tableStack.top().colPtr++] = col;
	}

	void HtmlBuilder::add_tab_row(boost::shared_ptr<AstNode> node) {
		if (tableStack.empty()) {
			return;
		}
		
		TabRow row;

		size_t i = 1;
		while (node->child(i)) {
			boost::shared_ptr<AstNode> tag = node->child(i);
			if (tag->data().type != Tag) {
				continue;
			}
			
			std::string name = tag->child(0)->data().value;
			std::string value = tag->child(1)->data().value;

			if (name == "tRowAttrWidth") {
				row.styles.push_back("height: " + value + "pt");
			}
			
			++i;
		}

		tableStack.top().rows.push_back(row);
	}

	void HtmlBuilder::add_tab_cell(boost::shared_ptr<AstNode> node) {
		if (tableStack.empty()) {
			return;
		}

		++tableStack.top().cellCounter;

		TabCell cell;

		if (tabCellStyleDefined) {
			cell = tabCellStyleDefined.get();
			tabCellStyleDefined.reset();
		}

		std::string str = node->child(1)->data().value;
		size_t end = str.find_last_of('<');
		str = str.substr(0, end);

		std::vector<std::string> pos;
		boost::split(pos, str, boost::is_any_of(",:"));

		if (pos.size() >= 2) {
			cell.col = boost::lexical_cast<int>(pos[1]);
		} if (pos.size() >= 1) {
			cell.row = boost::lexical_cast<int>(pos[0]);
		}

		size_t i = 2;
		while (node->child(i)) {
			boost::shared_ptr<AstNode> tag = node->child(i);
			if (tag->data().type != Tag) {
				continue;
			}

			std::string name = tag->child(0)->data().value;
			std::string value = tag->child(1)->data().value;

			if (name == "tCellFillColor") {
				std::string color = styles.getColor(Builder::sanitizeString(value));
				if (color.empty()) {
					color = Builder::convertColor(value);
				}
				cell.styles.push_back("background-color: " + color);
			} else if (name == "tcTopStrokeType" ||
						name == "tcBottomStrokeType" ||
						name == "tcLeftStrokeType" ||
						name == "tcRightStrokeType") {
				std::string style;
				if (value == "Solid") {
					style = "solid";
				} else if (value == "Dashed") {
					style = "dashed";
				} else if (value == "Dotted") {
					style = "dotted";
				} else if (value == "ThinThick") {
					style = "double";
				} else {
					style = "hidden";
				}

				switch (name[2]) {
				case 'B': // tcBottomStrokeType
					cell.styles.push_back("border-bottom-style: " + style);
					break;
				case 'T': // tcTopStrokeType
					cell.styles.push_back("border-top-style: " + style);
					break;
				case 'L': // tcLeftStrokeType
					cell.styles.push_back("border-left-style: " + style);
					break;
				case 'R': // tcRightStrokeType
					cell.styles.push_back("border-right-style: " + style);
					break;
				}
			} else if (name == "tCellTopStrokeColor") {
				std::string color = styles.getColor(Builder::sanitizeString(value));
				if (color.empty()) {
					color = Builder::convertColor(value);
				}
				cell.styles.push_back("border-top-color: " + color);
			} else if (name == "tCellBottomStrokeColor") {
				std::string color = styles.getColor(Builder::sanitizeString(value));
				if (color.empty()) {
					color = Builder::convertColor(value);
				}
				cell.styles.push_back("border-bottom-color: " + color);
			} else if (name == "tCellLeftStrokeColor") {
				std::string color = styles.getColor(Builder::sanitizeString(value));
				if (color.empty()) {
					color = Builder::convertColor(value);
				}
				cell.styles.push_back("border-left-color: " + color);
			} else if (name == "tCellRightStrokeColor") {
				std::string color = styles.getColor(Builder::sanitizeString(value));
				if (color.empty()) {
					color = Builder::convertColor(value);
				}
				cell.styles.push_back("border-right-color: " + color);
			} else if (name == "tTextCellVerticalJustification") {
				std::string align;
				if (value == "0") {
					align = "top";
				} else if (value == "1") {
					align = "bottom";
				} else {
					align = "center";
				}

				cell.styles.push_back("vertical-align: " + align);
			}

			++i;
		}

		tabCellStack.push(cell);
	}

	void HtmlBuilder::print_table()
	{
		if (tableStack.empty()) {
			return;
		}
		Table tab = tableStack.top();
		tableStack.pop();

		size_t colCount = tab.cols.size();
		size_t rowCount = tab.rows.size();

		// set header rows
		size_t hdrPtr = 0;
		while (tab.header-- > 0) {
			tab.rows[hdrPtr++].header = true;
		}

		// set footer rows
		hdrPtr = rowCount - 1;
		while (tab.footer-- > 0) {
			tab.rows[hdrPtr--].header = true;
		}

		for (std::vector<TabRow>::iterator iter = tab.rows.begin(); iter != tab.rows.end(); ++iter) {
			iter->cells.resize(colCount);
		}

		size_t rowNum = 0, colNum = 0;

		// reverse stack - cells must be printed in order
		std::stack<TabCell> tmp;
		for (size_t i = 0; i < tab.colCount * tab.rowCount && !tabCellStack.empty(); ++i) {
			tmp.push(tabCellStack.top());
			tabCellStack.pop();
		}

		while (!tmp.empty()) {
			TabCell cell = tmp.top();
			tmp.pop();

			if (tab.rows[rowNum].cells[colNum].className != "*") {
				if (cell.col == 1 && !tab.cols[colNum].styles.empty()) {
					cell.styles.push_back(tab.cols[colNum].styles[0]);
				}

				tab.rows[rowNum].cells[colNum] = cell;

				size_t tmp = colNum;
				int span = cell.col;
				while (tmp++ < colCount - 1 && span-- > 1) {
					tab.rows[rowNum].cells[tmp].className = "*";
				}

				tmp = rowNum;
				span = cell.row;
				while (tmp++ < rowCount - 1 && span-- > 1) {
					tab.rows[tmp].cells[colNum].className = "*";
				}
			}

			if (++colNum >= colCount) {
				colNum = 0;
				++rowNum;
			}
		}

		if (!tab.className.empty()) {
			print("\n<table class=\"" + tab.className + "\"");
		} else {
			print("\n<table");
		}

		if (!tab.styles.empty()) {
			print(" style=\"");
			for (std::vector<std::string>::const_iterator i = tab.styles.begin(); i != tab.styles.end(); ++i) {
				print(*i + "; ");
			}
			print("\"");
		}
		print(">\n");

		for (std::vector<TabRow>::const_iterator row = tab.rows.begin(); row != tab.rows.end(); ++row) {
			print("<tr");
			if (!row->styles.empty()) {
				print(" style=\"");
				for (std::vector<std::string>::const_iterator i = row->styles.begin(); i != row->styles.end(); ++i) {
					print(*i + "; ");
				}
				print("\"");
			}
			print(">\n");

			for (std::vector<TabCell>::const_iterator cell = row->cells.begin(); cell != row->cells.end(); ++cell) {
				if (cell->className == "*") {
					continue;
				}

				std::string tag = "<td";
				std::string tagEnd = "</td>";
				if (row->header) {
					tag = "<th";
					tagEnd = "</th>";
				}

				if (!cell->className.empty()) {
					print(tag + " class=\"" + cell->className + "\"");
				} else {
					print(tag);
				}

				if (cell->col > 1) {
					print(" colspan=\"" + boost::lexical_cast<std::string>(cell->col) + "\"");
				}
				if (cell->row > 1) {
					print(" rowspan=\"" + boost::lexical_cast<std::string>(cell->row) + "\"");
				}
				if (!cell->styles.empty()) {
					print(" style=\"");
					for (std::vector<std::string>::const_iterator i = cell->styles.begin(); i != cell->styles.end(); ++i) {
						print(*i + "; ");
					}
					print("\"");
				}
				print(">\n");

				bool empty = true;
				std::string s = cell->content;
				for (std::string::const_iterator i = s.begin(); i != s.end(); ++i) {
					if (std::isgraph(*i)) {
						empty = false;
					}
				}
				if (empty) {
					s = "&nbsp;";
				}
				print(s);

				print(tagEnd + "\n");
			}

			print("</tr>\n");
		}

		print("</table>\n");
	}

	void HtmlBuilder::flush_par_style() {
		if (parStyleStack.empty() || parStyleStack.top()[1] == '/') {
			return;
		}

		std::ostringstream os2;

		std::stack<std::string> st;
		while (!parStyleStack.empty() && parStyleStack.top()[0] != '<') {
			st.push(parStyleStack.top());
			parStyleStack.pop();
		}

		if (parStyleStack.empty()) {
			os2 << "\n\n<!-- Corrupted -->\n\n";
			return;
		}

		os2 << parStyleStack.top();
		parStyleStack.pop();

		if (st.empty()) {
			os2 << '>';
		} else {
			os2 << " style=\"";
			while (!st.empty()) {
				os2 << st.top();
				st.pop();
			}
			os2 << "\">";
		}

		parStyleStack.push("</p>");

		print(os2.str());
	}

	void HtmlBuilder::end_par_style() {
		if (!parStyleStack.empty() && parStyleStack.top() != "</p>") {
			print("&nbsp;");
		}

		flush_par_style();

		if (parStyleStack.empty() || parStyleStack.top() != "</p>") {
			return;
		}

		parStyleStack.pop();

		print("</p>");
	}

	void HtmlBuilder::end_char_style() {
		while (!charStyleStack.empty()) {
			print(charStyleStack.top());
			charStyleStack.pop();
		}
	}

	void HtmlBuilder::print(const std::string& s) {
		if (tabCellStack.empty()) {
			Builder::print(s);
		} else {
			tabCellStack.top().content += s;
		}
	}

}}
