#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "utf8.h"
#include "utf8-reader.h"

namespace utf8 {

	std::wstring read_utf8(const char* filename) {
		// Open the test file (contains UTF-8 encoded text)
		std::ifstream fs8(filename);
		if (!fs8.is_open()) {
			std::cerr << "Could not open " << filename << std::endl;
			return L"";
		}

		fs8.unsetf(std::ios::skipws);

		std::wstring wide;

		// Convert it to utf-16
		if (sizeof(std::wstring::value_type) > 2) {
			utf8to32(std::istream_iterator<char>(fs8), std::istream_iterator<char>(), std::back_inserter(wide));
		} else {
			utf8to16(std::istream_iterator<char>(fs8), std::istream_iterator<char>(), std::back_inserter(wide));
		}
		fs8.close();

		return wide;
	}

	void write(const char* filename, const std::wstring& content) {
		std::ofstream fs8(filename);
		if (!fs8.is_open()) {
			std::cerr << "Could not open " << filename << std::endl;
			return;
		}

		if (sizeof(std::wstring::value_type) > 2) {
			utf32to8(content.begin(), content.end(), std::ostream_iterator<char>(fs8));
		} else {
			utf16to8(content.begin(), content.end(), std::ostream_iterator<char>(fs8));
		}
		fs8.close();
	}

	void write(const char* filename, const std::string& content) {
		std::ofstream fs8(filename);
		if (!fs8.is_open()) {
			std::cerr << "Could not open " << filename << std::endl;
			return;
		}

		std::copy(content.begin(), content.end(), std::ostream_iterator<char>(fs8));
		fs8.close();
	}

}
