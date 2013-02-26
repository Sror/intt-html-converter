#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "builder.h"
#include "fonts.h"

namespace intt { namespace impl {

	void Builder::reset() {
		os.str("");
		os.clear();

		while (!st.empty()) {
			st.pop();
		}

		typefaceBold = typefaceItalic = false;
		tab = false;

		tags.clear();
	}

	void Builder::build() {
		size_t i = 0;
		while (document.child(i)) {
			if (document.child(i)->data().type == Tag) {
				build_impl(document.child(i));
			}
			++i;
		}
	}

	void Builder::build_impl(boost::shared_ptr<AstNode> node) {
		size_t i = 0;
		size_t count = 0;

		std::string tmp;
		NodeType type = node->data().type;

		switch (node->data().type) {
		case Text:
			print(node->data().value);
			break;

		case Entity:
			print("<" + build_entity(node->data().value) + '>');
			break;

		case Tag: {
			tmp = node->data().value;
			if (tmp == "table") {
				tmp = build_table(node);
				print(tmp);
				break;
			}
			tmp = build_tag(node);
			break;
		}

		case Attributes:
			break;
		}
	}

	std::string Builder::build_tag(boost::shared_ptr<AstNode> node) {
		std::vector<std::string> local_tags;

		std::string name = node->data().value;
		std::string ret;

		Attr attr = build_style(node->child(0));

		if (name == "p" || name == "div" || (name.size() == 2 && name[0] == 'h')) {
			for (auto i = tags.begin(); i != tags.end(); ++i) {
				if (!i->second.empty()) {
					print("<" + i->first + ":>");
				}
				while (!i->second.empty()) {
					i->second.pop();
				}
			}

			print("\n<ParaStyle:");
			if (attr.class_.empty()) {
				print("NormalParagraphStyle>");
			} else {
				print(attr.class_ + '>');
			}
		} else if (name == "span") {
			if (!attr.class_.empty()) {
				print_style("CharStyle", attr.class_);
				local_tags.push_back("CharStyle");
			}
		} else if (name == "strong" || name == "b" || name == "em" || name == "i") {
			std::string tmp;
			if (name == "b" || name == "strong") {
				typefaceBold = true;
				tmp = "Bold";
			} else {
				typefaceItalic = true;
				tmp = "Italic";
			}
			if (typefaceBold && typefaceItalic) {
				tmp = "Bold Italic";
			}

			print_style("cTypeface", tmp);
			local_tags.push_back("cTypeface");

			if (!attr.class_.empty()) {
				print_style("CharStyle", attr.class_);
				local_tags.push_back("CharStyle");
			}
		} else if (name == "sup" || name == "sub") {
			if (!attr.class_.empty()) {
				print_style("CharStyle", attr.class_);
				local_tags.push_back("CharStyle");
			}

			if (name == "sup") {
				print_style("cPosition", "Superscript");
			} else {
				print_style("cPosition", "Subscript");
			}
		}

		for (auto i = attr.styles.begin(); i != attr.styles.end(); ++i) {
			print_style(i->first, i->second);
			local_tags.push_back(i->first);
		}

		size_t i = 1;
		while (node->child(i)) {
			build_impl(node->child(i++));
		}

		for (auto i = local_tags.begin(); i != local_tags.end(); ++i) {
			if (!tags[*i].empty()) {
				tags[*i].pop();
				print("<" + *i + ":>");
			}
			
			if (!tags[*i].empty()) {
				print("<" + *i + ":" + tags[*i].top() + ">");
			}
		}

		return ret;
	}

	std::string Builder::build_table(boost::shared_ptr<AstNode> node) {
		tab = true;
		os_tab.str("");
		os_tab.clear();

		int rows, cols, hdr, foot;
		rows = cols = hdr = foot = 0;

		Attr attr = build_style(node->child(0));

		print("<TableStart:#,#,#,#");
		for (auto i = attr.styles.begin(); i != attr.styles.end(); ++i) {
			print("<" + i->first + ':' + i->second + '>');
		}
		print(">");

		boost::shared_ptr<AstNode> header, body, footer;
		
		size_t i = 1;
		while (node->child(i)) {
			std::string tmp = node->child(i)->data().value;
			if (tmp == "tbody") {
				body = node->child(i);
			} else if (tmp == "thead") {
				header = node->child(i);
			} else if (tmp == "tfoot") {
				footer = node->child(i);
			}
			++i;
		}

		if (!body && !header && !footer) {
			body = node;
		}

		i = 1;
		while (header && header->child(i)) {
			if (header->child(i)->data().type == Tag) {
				++hdr;
				cols = build_row(header->child(i));
			}
			++i;
		}

		i = 1;
		while (body && body->child(i)) {
			if (body->child(i)->data().type == Tag) {
				++rows;
				cols = build_row(body->child(i));
			}
			++i;
		}

		i = 1;
		while (footer && footer->child(i)) {
			if (footer->child(i)->data().type == Tag) {
				++foot;
				cols = build_row(footer->child(i));
			}
			++i;
		}

		print("<TableEnd:>");

		tab = false;
		std::string ret = os_tab.str();
		std::string tmp;
		for (size_t i = 0; i < cols; ++i) {
			tmp += "<ColStart:>";
		}
		size_t pos = ret.find("<RowStart:");
		ret.insert(pos, tmp);
		ret.replace(ret.find_first_of("#"), 1, boost::lexical_cast<std::string>(rows));
		ret.replace(ret.find_first_of("#"), 1, boost::lexical_cast<std::string>(cols));
		ret.replace(ret.find_first_of("#"), 1, boost::lexical_cast<std::string>(hdr));
		ret.replace(ret.find_first_of("#"), 1, boost::lexical_cast<std::string>(foot));
		return ret;
	}

	size_t Builder::build_row(boost::shared_ptr<AstNode> node) {
		Attr attr = build_style(node->child(0));

		size_t cell_count = 0;

		print("<RowStart:");
		for (auto i = attr.styles.begin(); i != attr.styles.end(); ++i) {
			print("<" + i->first + ':' + i->second + '>');
		}
		print(">");

		size_t i = 1;
		while (node->child(i)) {
			if (node->child(i)->data().type == Tag) {
				cell_count += build_cell(node->child(i));
			}
			++i;
		}

		print("<RowEnd:>");

		return cell_count;
	}

	size_t Builder::build_cell(boost::shared_ptr<AstNode> node) {
		Attr attr = build_style(node->child(0));

		size_t cell_size = 1;

		size_t i = 0;
		while (node->child(0)->child(i)) {
			boost::shared_ptr<AstNode> tmp = node->child(0)->child(i);
			if (tmp->data().value == "colspan") {
				cell_size = boost::lexical_cast<size_t>(tmp->child(0)->data().value);
			}
			++i;
		}

		if (!attr.class_.empty()) {
			print("<CellStyle:" + attr.class_ + "><StylePriority:0>");
		}
		print("<CellStart:1," + boost::lexical_cast<std::string>(cell_size));
		
		for (auto i = attr.styles.begin(); i != attr.styles.end(); ++i) {
			print("<" + i->first + ':' + i->second + '>');
		}
		print(">");

		if (node->child(1) && (
			node->child(1)->data().type == Text
			|| node->child(1)->data().type == Entity
			|| (node->child(1)->data().type == Tag && !(
				node->child(1)->data().value == "p" 
				|| node->child(1)->data().value == "div" 
				|| node->child(1)->data().value[0] == 'h'))
		)) {
			print("\n<ParaStyle:NormalParagraphStyle>");
		}

		i = 1;
		while (node->child(i)) {
			build_impl(node->child(i++));
		}

		print("<CellEnd:>");

		for (i = 1; i < cell_size; ++i) {
			print("<CellStart:1,1><CellEnd:>");
		}

		return cell_size;
	}

	Builder::Attr Builder::build_style(boost::shared_ptr<AstNode> node) {
		Attr attr;
		
		if (node->data().type != Attributes) {
			return attr;
		}

		size_t i = 0;
		while (node->child(i)) {
			if (node->child(i)->data().type == AttrName && node->child(i)->data().value == "class") {
				attr.class_ = node->child(i)->child(0)->data().value;
				break;
			}
			++i;
		}

		i = 0;
		while (node->child(i)) {
			if (node->child(i)->data().value == "style") {
				boost::shared_ptr<AstNode> n = node->child(i);
				size_t j = 0;
				while (n->child(j)) {
					std::string key = n->child(j)->data().value;
					std::string value = n->child(j)->child(0)->data().value;

					if (key == "font-family") {
						std::string fontname = get_font(value);
						if (!fontname.empty()) {
							attr.styles["cFont"] = fontname;
						}
					} else if (key == "font-size") {
						size_t end = value.find_last_of("0123456789.") + 1;
						double size = boost::lexical_cast<double>(value.substr(0, end));
						std::string unit;
						if (end < value.size()) {
							unit = value.substr(end);
						}
						boost::trim(unit);
						boost::to_lower(unit);
						if (unit.empty()) {
							if (unit == "pc") {
								size *= 12;
							} else if (unit == "in") {
								size *= 72;
							} else if (unit == "mm") {
								size *= 2.8346456692897;
							} else if (unit == "px") {
								size *= (1.0 / 96.0) / 72.0; // assuming resolution 96 DPI
							}
						}

						attr.styles["cSize"] = boost::lexical_cast<std::string>(size);
					} else if (key == "color") {
						attr.styles["cColor"] = build_color(value);
					} else if (key == "font-style") {
						if (value == "italic") {
							attr.styles["cTypeface"] = "Italic";
						}
					} else if (key == "text-decoration") {
						std::vector<std::string> tmp;
						boost::split(tmp, value, boost::is_any_of(", "));
						for (auto i = tmp.begin(); i != tmp.end(); ++i) {
							if (*i == "line-through") {
								attr.styles["cStrikethru"] = "1";
							} else if (*i == "underline") {
								attr.styles["cUnderline"] = "1";
							}
						}
					} else if (key == "font-weight") {
						if (value == "bold") {
							attr.styles["cTypeface"] = "Bold";
						}
					}
					++j;
				}
				break;
			}
			++i;
		}

		return attr;
	}

	void Builder::print_style(const std::string& s, const std::string& name) {
		if (!tags[s].empty()) {
			print("<" + s + ":>");
		}
		print("<" + s + ":" + name + ">");
		tags[s].push(name);
	}

	void Builder::print(const std::string& s) {
		if (!tab) {
			os << s;
		} else {
			os_tab << s;
		}
	}

	std::string Builder::build_color(const std::string& str) {
		std::string tmp(str);
		auto iter = colors.find(str);
		if (iter != colors.end()) {
			tmp = iter->second;
			iter = classes.find(tmp);
			if (iter != classes.end()) {
				tmp = iter->second;
			}
		} else {
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
			auto iter = colorNames.find(tmp);
			if (iter != colorNames.end()) {
				tmp = iter->second;
			} else if (tmp[0] != '#') {
				tmp = "#000000"; // unknown color -- default to black
			}

			tmp = tmp.substr(1);
			if (tmp.size() == 3) {
				tmp.resize(6);
				tmp[4] = tmp[2];
				tmp[2] = tmp[1];

				tmp[1] = tmp[0];
				tmp[3] = tmp[2];
				tmp[5] = tmp[4];
			}
			int r = keyMap[tmp[0]] * 16 + keyMap[tmp[1]];
			int g = keyMap[tmp[2]] * 16 + keyMap[tmp[3]];
			int b = keyMap[tmp[4]] * 16 + keyMap[tmp[5]];

			std::ostringstream os;
			os << "COLOR:RGB:Process:" << (r / 255.0) << ',' << (g / 255.0) << ',' << (b / 255.0);
			tmp = os.str();
		}
		return tmp;
	}

	std::string Builder::build_entity(std::string str) {
		if (str[0] == '#') {
			str[0] = '0';
		} else {
			std::map<std::string, std::string>::const_iterator iter = entities.find(str);
			if (iter != entities.end()) {
				str = iter->second;
			} else {
				str = "0x003F"; // unknown entity -- default to question mark
			}
		}
		return str;
	}

#define DEF_COLOR(name, value) colorNames.insert(std::make_pair(#name, value));
#define DEF_ENTITY(name, value) entities.insert(std::make_pair(name, value));

	void Builder::init() {
		DEF_COLOR(aliceblue,            "#F0F8FF")
		DEF_COLOR(antiquewhite,         "#FAEBD7")
		DEF_COLOR(aqua,                 "#00FFFF")
		DEF_COLOR(aquamarine,           "#7FFFD4")
		DEF_COLOR(azure,                "#F0FFFF")
		DEF_COLOR(beige,                "#F5F5DC")
		DEF_COLOR(bisque,               "#FFE4C4")
		DEF_COLOR(black,                "#000000")
		DEF_COLOR(blanchedalmond,       "#FFEBCD")
		DEF_COLOR(blue,                 "#0000FF")
		DEF_COLOR(blueviolet,           "#8A2BE2")
		DEF_COLOR(brown,                "#A52A2A")
		DEF_COLOR(burlywood,            "#DEB887")
		DEF_COLOR(cadetblue,            "#5F9EA0")
		DEF_COLOR(chartreuse,           "#7FFF00")
		DEF_COLOR(chocolate,            "#D2691E")
		DEF_COLOR(coral,                "#FF7F50")
		DEF_COLOR(cornflowerblue,       "#6495ED")
		DEF_COLOR(cornsilk,             "#FFF8DC")
		DEF_COLOR(crimson,              "#DC143C")
		DEF_COLOR(cyan,                 "#00FFFF")
		DEF_COLOR(darkblue,             "#00008B")
		DEF_COLOR(darkcyan,             "#008B8B")
		DEF_COLOR(darkgoldenrod,        "#B8860B")
		DEF_COLOR(darkgray,             "#A9A9A9")
		DEF_COLOR(darkgrey,             "#A9A9A9")
		DEF_COLOR(darkgreen,            "#006400")
		DEF_COLOR(darkkhaki,            "#BDB76B")
		DEF_COLOR(darkmagenta,          "#8B008B")
		DEF_COLOR(darkolivegreen,       "#556B2F")
		DEF_COLOR(darkorange,           "#FF8C00")
		DEF_COLOR(darkorchid,           "#9932CC")
		DEF_COLOR(darkred,              "#8B0000")
		DEF_COLOR(darksalmon,           "#E9967A")
		DEF_COLOR(darkseagreen,         "#8FBC8F")
		DEF_COLOR(darkslateblue,        "#483D8B")
		DEF_COLOR(darkslategray,        "#2F4F4F")
		DEF_COLOR(darkslategrey,        "#2F4F4F")
		DEF_COLOR(darkturquoise,        "#00CED1")
		DEF_COLOR(darkviolet,           "#9400D3")
		DEF_COLOR(deeppink,             "#FF1493")
		DEF_COLOR(deepskyblue,          "#00BFFF")
		DEF_COLOR(dimgray,              "#696969")
		DEF_COLOR(dimgrey,              "#696969")
		DEF_COLOR(dodgerblue,           "#1E90FF")
		DEF_COLOR(firebrick,            "#B22222")
		DEF_COLOR(floralwhite,          "#FFFAF0")
		DEF_COLOR(forestgreen,          "#228B22")
		DEF_COLOR(fuchsia,              "#FF00FF")
		DEF_COLOR(gainsboro,            "#DCDCDC")
		DEF_COLOR(ghostwhite,           "#F8F8FF")
		DEF_COLOR(gold,                 "#FFD700")
		DEF_COLOR(goldenrod,            "#DAA520")
		DEF_COLOR(gray,                 "#808080")
		DEF_COLOR(grey,                 "#808080")
		DEF_COLOR(green,                "#008000")
		DEF_COLOR(greenyellow,          "#ADFF2F")
		DEF_COLOR(honeydew,             "#F0FFF0")
		DEF_COLOR(hotpink,              "#FF69B4")
		DEF_COLOR(indianred,            "#CD5C5C")
		DEF_COLOR(indigo,               "#4B0082")
		DEF_COLOR(ivory,                "#FFFFF0")
		DEF_COLOR(khaki,                "#F0E68C")
		DEF_COLOR(lavender,             "#E6E6FA")
		DEF_COLOR(lavenderblush,        "#FFF0F5")
		DEF_COLOR(lawngreen,            "#7CFC00")
		DEF_COLOR(lemonchiffon,         "#FFFACD")
		DEF_COLOR(lightblue,            "#ADD8E6")
		DEF_COLOR(lightcoral,           "#F08080")
		DEF_COLOR(lightcyan,            "#E0FFFF")
		DEF_COLOR(lightgoldenrodyellow, "#FAFAD2")
		DEF_COLOR(lightgray,            "#D3D3D3")
		DEF_COLOR(lightgrey,            "#D3D3D3")
		DEF_COLOR(lightgreen,           "#90EE90")
		DEF_COLOR(lightpink,            "#FFB6C1")
		DEF_COLOR(lightsalmon,          "#FFA07A")
		DEF_COLOR(lightseagreen,        "#20B2AA")
		DEF_COLOR(lightskyblue,         "#87CEFA")
		DEF_COLOR(lightslategray,       "#778899")
		DEF_COLOR(lightslategrey,       "#778899")
		DEF_COLOR(lightsteelblue,       "#B0C4DE")
		DEF_COLOR(lightyellow,          "#FFFFE0")
		DEF_COLOR(lime,                 "#00FF00")
		DEF_COLOR(limegreen,            "#32CD32")
		DEF_COLOR(linen,                "#FAF0E6")
		DEF_COLOR(magenta,              "#FF00FF")
		DEF_COLOR(maroon,               "#800000")
		DEF_COLOR(mediumaquamarine,     "#66CDAA")
		DEF_COLOR(mediumblue,           "#0000CD")
		DEF_COLOR(mediumorchid,         "#BA55D3")
		DEF_COLOR(mediumpurple,         "#9370D8")
		DEF_COLOR(mediumseagreen,       "#3CB371")
		DEF_COLOR(mediumslateblue,      "#7B68EE")
		DEF_COLOR(mediumspringgreen,    "#00FA9A")
		DEF_COLOR(mediumturquoise,      "#48D1CC")
		DEF_COLOR(mediumvioletred,      "#C71585")
		DEF_COLOR(midnightblue,         "#191970")
		DEF_COLOR(mintcream,            "#F5FFFA")
		DEF_COLOR(mistyrose,            "#FFE4E1")
		DEF_COLOR(moccasin,             "#FFE4B5")
		DEF_COLOR(navajowhite,          "#FFDEAD")
		DEF_COLOR(navy,                 "#000080")
		DEF_COLOR(oldlace,              "#FDF5E6")
		DEF_COLOR(olive,                "#808000")
		DEF_COLOR(olivedrab,            "#6B8E23")
		DEF_COLOR(orange,               "#FFA500")
		DEF_COLOR(orangered,            "#FF4500")
		DEF_COLOR(orchid,               "#DA70D6")
		DEF_COLOR(palegoldenrod,        "#EEE8AA")
		DEF_COLOR(palegreen,            "#98FB98")
		DEF_COLOR(paleturquoise,        "#AFEEEE")
		DEF_COLOR(palevioletred,        "#D87093")
		DEF_COLOR(papayawhip,           "#FFEFD5")
		DEF_COLOR(peachpuff,            "#FFDAB9")
		DEF_COLOR(peru,                 "#CD853F")
		DEF_COLOR(pink,                 "#FFC0CB")
		DEF_COLOR(plum,                 "#DDA0DD")
		DEF_COLOR(powderblue,           "#B0E0E6")
		DEF_COLOR(purple,               "#800080")
		DEF_COLOR(red,                  "#FF0000")
		DEF_COLOR(rosybrown,            "#BC8F8F")
		DEF_COLOR(royalblue,            "#4169E1")
		DEF_COLOR(saddlebrown,          "#8B4513")
		DEF_COLOR(salmon,               "#FA8072")
		DEF_COLOR(sandybrown,           "#F4A460")
		DEF_COLOR(seagreen,             "#2E8B57")
		DEF_COLOR(seashell,             "#FFF5EE")
		DEF_COLOR(sienna,               "#A0522D")
		DEF_COLOR(silver,               "#C0C0C0")
		DEF_COLOR(skyblue,              "#87CEEB")
		DEF_COLOR(slateblue,            "#6A5ACD")
		DEF_COLOR(slategray,            "#708090")
		DEF_COLOR(slategrey,            "#708090")
		DEF_COLOR(snow,                 "#FFFAFA")
		DEF_COLOR(springgreen,          "#00FF7F")
		DEF_COLOR(steelblue,            "#4682B4")
		DEF_COLOR(tan,                  "#D2B48C")
		DEF_COLOR(teal,                 "#008080")
		DEF_COLOR(thistle,              "#D8BFD8")
		DEF_COLOR(tomato,               "#FF6347")
		DEF_COLOR(turquoise,            "#40E0D0")
		DEF_COLOR(violet,               "#EE82EE")
		DEF_COLOR(wheat,                "#F5DEB3")
		DEF_COLOR(white,                "#FFFFFF")
		DEF_COLOR(whitesmoke,           "#F5F5F5")
		DEF_COLOR(yellow,               "#FFFF00")
		DEF_COLOR(yellowgreen,          "#9ACD32")

		DEF_ENTITY("quot",     "0x0022")
		DEF_ENTITY("amp",      "0x0026")
		DEF_ENTITY("apos",     "0x0027")
		DEF_ENTITY("lt",       "0x003C")
		DEF_ENTITY("gt",       "0x003E")
		DEF_ENTITY("nbsp",     "0x00A0")
		DEF_ENTITY("iexcl",    "0x00A1")
		DEF_ENTITY("cent", 	   "0x00A2")
		DEF_ENTITY("pound",    "0x00A3")
		DEF_ENTITY("curren",   "0x00A4")
		DEF_ENTITY("yen",      "0x00A5")
		DEF_ENTITY("brvbar",   "0x00A6")
		DEF_ENTITY("sect",     "0x00A7")
		DEF_ENTITY("uml",      "0x00A8")
		DEF_ENTITY("copy",     "0x00A9")
		DEF_ENTITY("ordf",     "0x00AA")
		DEF_ENTITY("laquo",    "0x00AB")
		DEF_ENTITY("not",      "0x00AC")
		DEF_ENTITY("shy",      "0x00AD")
		DEF_ENTITY("reg",      "0x00AE")
		DEF_ENTITY("macr",     "0x00AF")
		DEF_ENTITY("deg",      "0x00B0")
		DEF_ENTITY("plusmn",   "0x00B1")
		DEF_ENTITY("sup2",     "0x00B2")
		DEF_ENTITY("sup3",     "0x00B3")
		DEF_ENTITY("acute",    "0x00B4")
		DEF_ENTITY("micro",    "0x00B5")
		DEF_ENTITY("para",     "0x00B6")
		DEF_ENTITY("middot",   "0x00B7")
		DEF_ENTITY("cedil",    "0x00B8")
		DEF_ENTITY("sup1",     "0x00B9")
		DEF_ENTITY("ordm",     "0x00BA")
		DEF_ENTITY("raquo",    "0x00BB")
		DEF_ENTITY("frac14",   "0x00BC")
		DEF_ENTITY("frac12",   "0x00BD")
		DEF_ENTITY("frac34",   "0x00BE")
		DEF_ENTITY("iquest",   "0x00BF")
		DEF_ENTITY("Agrave",   "0x00C0")
		DEF_ENTITY("Aacute",   "0x00C1")
		DEF_ENTITY("Acirc",    "0x00C2")
		DEF_ENTITY("Atilde",   "0x00C3")
		DEF_ENTITY("Auml",     "0x00C4")
		DEF_ENTITY("Aring",    "0x00C5")
		DEF_ENTITY("AElig",    "0x00C6")
		DEF_ENTITY("Ccedil",   "0x00C7")
		DEF_ENTITY("Egrave",   "0x00C8")
		DEF_ENTITY("Eacute",   "0x00C9")
		DEF_ENTITY("Ecirc",    "0x00CA")
		DEF_ENTITY("Euml",     "0x00CB")
		DEF_ENTITY("Igrave",   "0x00CC")
		DEF_ENTITY("Iacute",   "0x00CD")
		DEF_ENTITY("Icirc",    "0x00CE")
		DEF_ENTITY("Iuml",     "0x00CF")
		DEF_ENTITY("ETH",      "0x00D0")
		DEF_ENTITY("Ntilde",   "0x00D1")
		DEF_ENTITY("Ograve",   "0x00D2")
		DEF_ENTITY("Oacute",   "0x00D3")
		DEF_ENTITY("Ocirc",    "0x00D4")
		DEF_ENTITY("Otilde",   "0x00D5")
		DEF_ENTITY("Ouml",     "0x00D6")
		DEF_ENTITY("times",    "0x00D7")
		DEF_ENTITY("Oslash",   "0x00D8")
		DEF_ENTITY("Ugrave",   "0x00D9")
		DEF_ENTITY("Uacute",   "0x00DA")
		DEF_ENTITY("Ucirc",    "0x00DB")
		DEF_ENTITY("Uuml",     "0x00DC")
		DEF_ENTITY("Yacute",   "0x00DD")
		DEF_ENTITY("THORN",    "0x00DE")
		DEF_ENTITY("szlig",    "0x00DF")
		DEF_ENTITY("agrave",   "0x00E0")
		DEF_ENTITY("aacute",   "0x00E1")
		DEF_ENTITY("acirc",    "0x00E2")
		DEF_ENTITY("atilde",   "0x00E3")
		DEF_ENTITY("auml",     "0x00E4")
		DEF_ENTITY("aring",    "0x00E5")
		DEF_ENTITY("aelig",    "0x00E6")
		DEF_ENTITY("ccedil",   "0x00E7")
		DEF_ENTITY("egrave",   "0x00E8")
		DEF_ENTITY("eacute",   "0x00E9")
		DEF_ENTITY("ecirc",    "0x00EA")
		DEF_ENTITY("euml",     "0x00EB")
		DEF_ENTITY("igrave",   "0x00EC")
		DEF_ENTITY("iacute",   "0x00ED")
		DEF_ENTITY("icirc",    "0x00EE")
		DEF_ENTITY("iuml",     "0x00EF")
		DEF_ENTITY("eth",      "0x00F0")
		DEF_ENTITY("ntilde",   "0x00F1")
		DEF_ENTITY("ograve",   "0x00F2")
		DEF_ENTITY("oacute",   "0x00F3")
		DEF_ENTITY("ocirc",    "0x00F4")
		DEF_ENTITY("otilde",   "0x00F5")
		DEF_ENTITY("ouml",     "0x00F6")
		DEF_ENTITY("divide",   "0x00F7")
		DEF_ENTITY("oslash",   "0x00F8")
		DEF_ENTITY("ugrave",   "0x00F9")
		DEF_ENTITY("uacute",   "0x00FA")
		DEF_ENTITY("ucirc",    "0x00FB")
		DEF_ENTITY("uuml",     "0x00FC")
		DEF_ENTITY("yacute",   "0x00FD")
		DEF_ENTITY("thorn",    "0x00FE")
		DEF_ENTITY("yuml",     "0x00FF")
		DEF_ENTITY("OElig",    "0x0152")
		DEF_ENTITY("oelig",    "0x0153")
		DEF_ENTITY("Scaron",   "0x0160")
		DEF_ENTITY("scaron",   "0x0161")
		DEF_ENTITY("Yuml",     "0x0178")
		DEF_ENTITY("fnof",     "0x0192")
		DEF_ENTITY("circ",     "0x02C6")
		DEF_ENTITY("tilde",    "0x02DC")
		DEF_ENTITY("Alpha",    "0x0391")
		DEF_ENTITY("Beta",     "0x0392")
		DEF_ENTITY("Gamma",    "0x0393")
		DEF_ENTITY("Delta",    "0x0394")
		DEF_ENTITY("Epsilon",  "0x0395")
		DEF_ENTITY("Zeta",     "0x0396")
		DEF_ENTITY("Eta",      "0x0397")
		DEF_ENTITY("Theta",    "0x0398")
		DEF_ENTITY("Iota",     "0x0399")
		DEF_ENTITY("Kappa",    "0x039A")
		DEF_ENTITY("Lambda",   "0x039B")
		DEF_ENTITY("Mu",       "0x039C")
		DEF_ENTITY("Nu",       "0x039D")
		DEF_ENTITY("Xi",       "0x039E")
		DEF_ENTITY("Omicron",  "0x039F")
		DEF_ENTITY("Pi",       "0x03A0")
		DEF_ENTITY("Rho",      "0x03A1")
		DEF_ENTITY("Sigma",    "0x03A3")
		DEF_ENTITY("Tau",      "0x03A4")
		DEF_ENTITY("Upsilon",  "0x03A5")
		DEF_ENTITY("Phi",      "0x03A6")
		DEF_ENTITY("Chi",      "0x03A7")
		DEF_ENTITY("Psi",      "0x03A8")
		DEF_ENTITY("Omega",    "0x03A9")
		DEF_ENTITY("alpha",    "0x03B1")
		DEF_ENTITY("beta",     "0x03B2")
		DEF_ENTITY("gamma",    "0x03B3")
		DEF_ENTITY("delta",    "0x03B4")
		DEF_ENTITY("epsilon",  "0x03B5")
		DEF_ENTITY("zeta",     "0x03B6")
		DEF_ENTITY("eta",      "0x03B7")
		DEF_ENTITY("theta",    "0x03B8")
		DEF_ENTITY("iota",     "0x03B9")
		DEF_ENTITY("kappa",    "0x03BA")
		DEF_ENTITY("lambda",   "0x03BB")
		DEF_ENTITY("mu",       "0x03BC")
		DEF_ENTITY("nu",       "0x03BD")
		DEF_ENTITY("xi",       "0x03BE")
		DEF_ENTITY("omicron",  "0x03BF")
		DEF_ENTITY("pi",       "0x03C0")
		DEF_ENTITY("rho",      "0x03C1")
		DEF_ENTITY("sigmaf",   "0x03C2")
		DEF_ENTITY("sigma",    "0x03C3")
		DEF_ENTITY("tau",      "0x03C4")
		DEF_ENTITY("upsilon",  "0x03C5")
		DEF_ENTITY("phi",      "0x03C6")
		DEF_ENTITY("chi",      "0x03C7")
		DEF_ENTITY("psi",      "0x03C8")
		DEF_ENTITY("omega",    "0x03C9")
		DEF_ENTITY("thetasym", "0x03D1")
		DEF_ENTITY("upsih",    "0x03D2")
		DEF_ENTITY("piv",      "0x03D6")
		DEF_ENTITY("ensp",     "0x2002")
		DEF_ENTITY("emsp",     "0x2003")
		DEF_ENTITY("thinsp",   "0x2009")
		DEF_ENTITY("zwnj",     "0x200C")
		DEF_ENTITY("zwj",      "0x200D")
		DEF_ENTITY("lrm",      "0x200E")
		DEF_ENTITY("rlm",      "0x200F")
		DEF_ENTITY("ndash",    "0x2013")
		DEF_ENTITY("mdash",    "0x2014")
		DEF_ENTITY("lsquo",    "0x2018")
		DEF_ENTITY("rsquo",    "0x2019")
		DEF_ENTITY("sbquo",    "0x201A")
		DEF_ENTITY("ldquo",    "0x201C")
		DEF_ENTITY("rdquo",    "0x201D")
		DEF_ENTITY("bdquo",    "0x201E")
		DEF_ENTITY("dagger",   "0x2020")
		DEF_ENTITY("Dagger",   "0x2021")
		DEF_ENTITY("bull",     "0x2022")
		DEF_ENTITY("hellip",   "0x2026")
		DEF_ENTITY("permil",   "0x2030")
		DEF_ENTITY("prime",    "0x2032")
		DEF_ENTITY("Prime",    "0x2033")
		DEF_ENTITY("lsaquo",   "0x2039")
		DEF_ENTITY("rsaquo",   "0x203A")
		DEF_ENTITY("oline",    "0x203E")
		DEF_ENTITY("frasl",    "0x2044")
		DEF_ENTITY("euro",     "0x20AC")
		DEF_ENTITY("image",    "0x2111")
		DEF_ENTITY("weierp",   "0x2118")
		DEF_ENTITY("real",     "0x211C")
		DEF_ENTITY("trade",    "0x2122")
		DEF_ENTITY("alefsym",  "0x2135")
		DEF_ENTITY("larr",     "0x2190")
		DEF_ENTITY("uarr",     "0x2191")
		DEF_ENTITY("rarr",     "0x2192")
		DEF_ENTITY("darr",     "0x2193")
		DEF_ENTITY("harr",     "0x2194")
		DEF_ENTITY("crarr",    "0x21B5")
		DEF_ENTITY("lArr",     "0x21D0")
		DEF_ENTITY("uArr",     "0x21D1")
		DEF_ENTITY("rArr",     "0x21D2")
		DEF_ENTITY("dArr",     "0x21D3")
		DEF_ENTITY("hArr",     "0x21D4")
		DEF_ENTITY("forall",   "0x2200")
		DEF_ENTITY("part",     "0x2202")
		DEF_ENTITY("exist",    "0x2203")
		DEF_ENTITY("empty",    "0x2205")
		DEF_ENTITY("nabla",    "0x2207")
		DEF_ENTITY("isin",     "0x2208")
		DEF_ENTITY("notin",    "0x2209")
		DEF_ENTITY("ni",       "0x220B")
		DEF_ENTITY("prod",     "0x220F")
		DEF_ENTITY("sum",      "0x2211")
		DEF_ENTITY("minus",    "0x2212")
		DEF_ENTITY("lowast",   "0x2217")
		DEF_ENTITY("radic",    "0x221A")
		DEF_ENTITY("prop",     "0x221D")
		DEF_ENTITY("infin",    "0x221E")
		DEF_ENTITY("ang",      "0x2220")
		DEF_ENTITY("and",      "0x2227")
		DEF_ENTITY("or",       "0x2228")
		DEF_ENTITY("cap",      "0x2229")
		DEF_ENTITY("cup",      "0x222A")
		DEF_ENTITY("int",      "0x222B")
		DEF_ENTITY("there4",   "0x2234")
		DEF_ENTITY("sim",      "0x223C")
		DEF_ENTITY("cong",     "0x2245")
		DEF_ENTITY("asymp",    "0x2248")
		DEF_ENTITY("ne",       "0x2260")
		DEF_ENTITY("equiv",    "0x2261")
		DEF_ENTITY("le",       "0x2264")
		DEF_ENTITY("ge",       "0x2265")
		DEF_ENTITY("sub",      "0x2282")
		DEF_ENTITY("sup",      "0x2283")
		DEF_ENTITY("nsub",     "0x2284")
		DEF_ENTITY("sube",     "0x2286")
		DEF_ENTITY("supe",     "0x2287")
		DEF_ENTITY("oplus",    "0x2295")
		DEF_ENTITY("otimes",   "0x2297")
		DEF_ENTITY("perp",     "0x22A5")
		DEF_ENTITY("sdot",     "0x22C5")
		DEF_ENTITY("lceil",    "0x2308")
		DEF_ENTITY("rceil",    "0x2309")
		DEF_ENTITY("lfloor",   "0x230A")
		DEF_ENTITY("rfloor",   "0x230B")
		DEF_ENTITY("lang",     "0x2329")
		DEF_ENTITY("rang",     "0x232A")
		DEF_ENTITY("loz",      "0x25CA")
		DEF_ENTITY("spades",   "0x2660")
		DEF_ENTITY("clubs",    "0x2663")
		DEF_ENTITY("hearts",   "0x2665")
		DEF_ENTITY("diams",    "0x2666")
	}

	const char Builder::keyMap[] = {
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  15
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  31
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  47
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,  0,  0,  0,  0, //  63
		0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  79
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //  95
		0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 111
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 127
	};

	std::map<std::string, std::string> Builder::colorNames;
	std::map<std::string, std::string> Builder::entities;
	std::map<std::string, std::stack<std::string> > Builder::tags;

}}
