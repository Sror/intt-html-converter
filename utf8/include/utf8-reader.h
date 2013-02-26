#pragma once

#include <string>

namespace utf8 {

	std::wstring read_utf8(const char* filename);
	void write(const char* filename, const std::string& content);
	void write(const char* filename, const std::wstring& content);

}
