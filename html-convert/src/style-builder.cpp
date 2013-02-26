#include "style-builder.h"

#include <cmath>
#include <string>
#include <vector>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace html { namespace impl {

	void StyleBuilder::reset() {
		Builder::reset();
	}

	void StyleBuilder::build() {
		size_t i = 0;
		while (document.child(i)) {
			builder_impl(document.child(i++));
		}
		inherit_classes();

		for (auto iter = classes.begin(); iter != classes.end(); ++iter) {
			print(iter->build());
		}
	}

	void StyleBuilder::builder_impl(boost::shared_ptr<AstNode> node) {
		if (node->data().type != Tag) {
			return;
		}
		std::string name = node->child(0)->data().value;

		if (name == "DefineParaStyle" || 
			name == "DefineCharStyle" || 
			name == "DefineTableStyle") {
			Class cl;

			// put class name
			std::string value = node->child(1)->data().value;
			size_t end = value.find_last_of('=');
			value = value.substr(0, end);

			// no class name, ergo: no style
			if (value.empty()) {
				return;
			}
			cl.names.push_back(Builder::sanitizeString(value) + "/*" + value + "*/");
			originalStyles[Builder::sanitizeString(value)] = value;

			// put styles
			size_t i = 2;
			while (node->child(i)) {
				boost::shared_ptr<AstNode> n = node->child(i++);

				std::string type = n->child(0)->data().value;
				std::string val = n->child(1)->data().value;

				// style inheritation
				if (type == "BasedOn" || type == "cColor") {
					cl.inherits.push_back(Builder::sanitizeString(val));
					continue;
				}

				// styles common for paragraphs, characters tables and table cells
				if (type == "cSize") {
					cl.styles.push_back("font-size: " + val + "pt");
				} else if (type == "cFont") {
					cl.styles.push_back("font-family: \'" + val + "\'");
				} else if (type == "cTypeface") {
					if (val == "Bold") {
						cl.styles.push_back("font-weight: bold");
					} else if (val == "Italic") {
						cl.styles.push_back("font-style: italic");
					} else {
						cl.styles.push_back("font-style: normal");
						cl.styles.push_back("font-weight: normal");
					}
				} else if (type == "cCase") {
					if (val == "Small Caps") {
						cl.styles.push_back("font-variant: small-caps");
					} else {
						cl.styles.push_back("font-variant: normal");
					}
				}
				
				if (name == "DefineParaStyle") {
					// styles for paragraphs only
					if (type == "pSpaceBefore") {
						cl.styles.push_back("margin-top: " + val + "pt");
					} else if (type == "pSpaceAfter") {
						cl.styles.push_back("margin-bottom: " + val + "pt");
					} else if (type == "pLeftIndent") {
						cl.styles.push_back("margin-left: " + val + "pt");
					} else if (type == "pRightIndent") {
						cl.styles.push_back("margin-right: " + val + "pt");
					} else if (type == "pFirstLineIndent") {
						cl.styles.push_back("text-indent: " + val + "pt");
					} else if (type == "pTextAlignment") {
						if (val == "Left") {
							cl.styles.push_back("text-align: left");
						} else if (val == "Right") {
							cl.styles.push_back("text-align: right");
						} else if (val == "JustifyLeft" || val == "JustifyCenter" || val == "JustifyRight" || val == "JustifyFull") {
							cl.styles.push_back("text-align: justify");
						} else {
							cl.styles.push_back("text-align: left");
						}
					}
				}
				else if (name == "DefineTableStyle") {
					// styles for tables only
				}
				else if (name == "DefineCellStyle") {
					// styles for table cells only
					if (val == "tCellFillColor") {
						cl.styles.push_back("background-color: " + Builder::sanitizeString(value));
					} else if (val == "tcTopStrokeType" ||
							 val == "tcBottomStrokeType" ||
							 val == "tcLeftStrokeType" ||
							 val == "tcRightStrokeType") {
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
							cl.styles.push_back("border-bottom-style: " + style);
							break;
						case 'T': // tcTopStrokeType
							cl.styles.push_back("border-top-style: " + style);
							break;
						case 'L': // tcLeftStrokeType
							cl.styles.push_back("border-left-style: " + style);
							break;
						case 'R': // tcRightStrokeType
							cl.styles.push_back("border-right-style: " + style);
							break;
						}
					} else if (name == "tCellTopStrokeColor") {
						cl.styles.push_back("border-top-color: " + Builder::sanitizeString(value));
					} else if (name == "tCellBottomStrokeColor") {
						cl.styles.push_back("border-bottom-color: " + Builder::sanitizeString(value));
					} else if (name == "tCellLeftStrokeColor") {
						cl.styles.push_back("border-left-color: " + Builder::sanitizeString(value));
					} else if (name == "tCellRightStrokeColor") {
						cl.styles.push_back("border-right-color: " + Builder::sanitizeString(value));
					} else if (name == "tTextCellVerticalJustification") {
						std::string align;
						if (value == "0") {
							align = "top";
						} else if (value == "1") {
							align = "bottom";
						} else {
							align = "center";
						}
						cl.styles.push_back("vertical-align: " + align);
					}
				}
			}

			classes.push_back(cl);
		} else if (name == "ColorTable") {
			size_t i = 1;
			while (node->child(i)) {
				Class cl;

				boost::shared_ptr<AstNode> n = node->child(i++);
				// put class name
				std::string s = n->child(0)->data().value;
				cl.names.push_back(Builder::sanitizeString(s));
				originalStyles[Builder::sanitizeString(s)] = s;

				// put style
				std::string color = Builder::convertColor(n->child(1)->data().value);
				cl.styles.push_back("color: " + color);

				colors[Builder::sanitizeString(s)] = color;

				classes.push_back(cl);
			}
		}
	}

	void StyleBuilder::inherit_classes() {

		for (auto iter = classes.rbegin(); iter != classes.rend(); ++iter) {
			for (auto strIter = iter->inherits.begin(); strIter != iter->inherits.end(); ++strIter) {
				for (auto parentIter = iter + 1; parentIter != classes.rend(); ++parentIter) {
					size_t end = parentIter->names[0].find_first_of('/');
					if (parentIter->names[0].substr(0, end) == *strIter) {
						parentIter->names.push_back(iter->names[0]);
					}
				}
			}
		}
	}

	std::string StyleBuilder::Class::build() const {
		std::ostringstream out;
		for (auto iter = names.begin(); iter != names.end(); ++iter) {
			out << (iter == names.begin() ? "" : ", ") << "." << *iter;
		}
		out << " {\n";
		for (auto iter = styles.begin(); iter != styles.end(); ++iter) {
			out << "    " << *iter << ";\n";
		}
		out << "}\n";
		return out.str();
	}

}}
