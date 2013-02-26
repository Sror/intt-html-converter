#pragma once

#include <map>
#include <stack>
#include <vector>
#include <string>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "builder.h"
#include "ast.h"

namespace html { namespace impl {

	class StyleBuilder : public Builder
	{
	public:
		StyleBuilder(const AstNode& document) : Builder(document) {}

		virtual void reset();
		virtual void build();

		std::string getColor(const std::string& str) const
		{
			std::map<std::string, std::string>::const_iterator i = colors.find(str);
			if (i != colors.end())
				return i->second;
			return "";
		}

		std::string getOriginalStyles() const
		{
			std::ostringstream os;
			for (std::map<std::string, std::string>::const_iterator i = originalStyles.begin(); i != originalStyles.end(); ++i)
				os << i->first << ":" << i->second << "\n";
			return os.str();
		}

		std::string getColorTable() const
		{
			std::ostringstream os;
			for (std::map<std::string, std::string>::const_iterator i = colors.begin(); i != colors.end(); ++i)
				os << i->second << ":" << i->first << "\n";
			return os.str();
		}

	private:
		void builder_impl(boost::shared_ptr<AstNode> node);
		void inherit_classes();

		struct Class
		{
			std::vector<std::string> names;
			std::vector<std::string> styles;
			std::vector<std::string> inherits;

			std::string build() const;
		};

		std::vector<Class> classes;
		std::map<std::string, std::string> colors;
		std::map<std::string, std::string> originalStyles;
	};

}}