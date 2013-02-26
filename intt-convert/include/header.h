#pragma once

#include <map>
#include <string>

namespace intt { namespace impl {

	struct Header
	{
		bool corrupted;
		std::string rawDef;
		std::map<std::string, std::string> colorDef;
		std::map<std::string, std::string> classDef;
		std::string::size_type position;
	};

	Header getHeader(const std::string& str);

}}
