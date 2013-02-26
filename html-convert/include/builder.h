#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "ast.h"

namespace html { namespace impl {

	class Builder
	{
	public:
		Builder(const AstNode& document) : document(document) {}
		virtual ~Builder() {}

		virtual void build() = 0;

		virtual void reset()
		{
			os.str("");
			os.clear();
		}

		virtual std::string getResult() const { return os.str(); }

	protected:
		AstNode document;

		virtual void print(const std::string& s) { os << s; }

		/// <summary>
		/// Method for sanitizing string -- replaces all special characters and spaces to _ char.
		/// </summary>
		/// <param name="str">String to sanitize</param>
		/// <returns>Sanitized string</returns>
		static std::string sanitizeString(const std::string& str)
		{
			std::string res;
			Sanitizer s;
			std::transform(str.begin(), str.end(), std::back_inserter(res), s);
			boost::replace_all(res, " ", "_");
			return res;
		}

		static std::string convertColor(const std::string& str)
		{
			std::string tmp(str);
			boost::erase_all(tmp, "\\");
			boost::erase_all(tmp, " ");

			std::vector<std::string> colorInfo;
			boost::split(colorInfo, tmp, boost::is_any_of(":"));
			if (colorInfo[0] != "COLOR")
			{
				if (colorInfo.size() > 1)
					return "#000000";
				else
					return colorInfo[0];
			}

			if (colorInfo.size() < 4)
				return "#000000";

			std::vector<std::string> components;
			boost::split(components, colorInfo[3], boost::is_any_of(","));

			unsigned char r, g, b;

			if (colorInfo[1] == "CMYK")
			{
				// convert formula taken from http://easyrgb.com/index.php?X=MATH

				float c = boost::lexical_cast<float>(components[0]);
				float m = boost::lexical_cast<float>(components[1]);
				float y = boost::lexical_cast<float>(components[2]);
				float k = boost::lexical_cast<float>(components[3]);

				// first step: convert CMYK to CMY
				float c2 = (c * (1.0f - k) + k);
				float m2 = (m * (1.0f - k) + k);
				float y2 = (y * (1.0f - k) + k);

				// second step: convert CMY to RGB
				r = (unsigned char)((1.0f - c2) * 255);
				g = (unsigned char)((1.0f - m2) * 255);
				b = (unsigned char)((1.0f - y2) * 255);
			}
			else if (colorInfo[1] == "LAB")
			{
				// TODO implement
				r = g = b = 0;
			}
			else // RGB color
			{
				r = (unsigned char)(boost::lexical_cast<float>(components[0]) * 255);
				g = (unsigned char)(boost::lexical_cast<float>(components[1]) * 255);
				b = (unsigned char)(boost::lexical_cast<float>(components[2]) * 255);
			}

			return (boost::format("#%|1$02X|%|2$02X|%|3$02X|") % (int)r % (int)g % (int)b).str();
		}

	private:
		std::ostringstream os;

		struct Sanitizer
		{
			template<typename Char>
			Char operator()(Char c) const
			{
				return (std::isalnum(c) || std::isspace(c)) ? c : '_';
			}
		};
	};

}}
