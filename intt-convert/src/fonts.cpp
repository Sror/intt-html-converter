#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <boost/algorithm/string.hpp>

#include "fonts.h"

namespace intt { namespace impl {

	static std::vector<std::wstring> fonts;

	int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam) {
		fonts.push_back(lpelfe->elfFullName);
		//Return non-zero to continue enumeration
		return 1;
	}

	std::vector<std::wstring> get_fonts() {
		LOGFONT lf;
		lf.lfFaceName[0] = '\0';
		lf.lfCharSet = DEFAULT_CHARSET;
		HDC hDC = ::GetDC(NULL);
		EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)&EnumFontFamExProc, NULL, NULL);

		Sleep(10);

		std::vector<std::wstring>::iterator end = std::unique(fonts.begin(), fonts.end());
		fonts.resize(end - fonts.begin());
		
		return fonts;
	}

	std::string get_font(std::string name) {
		static std::vector<std::wstring> system_fonts = get_fonts();

		std::vector<std::string> input;
		boost::split(input, name, boost::is_any_of(","), boost::token_compress_on);

		for (auto iter = input.begin(); iter != input.end(); ++iter) {
			name = *iter;

			size_t start = 0;
			while (!std::isalnum(name[start])) {
				++start;
			}

			size_t end = name.size();
			while (!std::isalnum(name[end - 1])) {
				--end;
			}

			name = name.substr(start, end - start);

			boost::to_lower(name);
			std::vector<std::string> vec;
			boost::split(vec, name, boost::is_any_of(" -"), boost::token_compress_on);

			std::string tmp;
			for (auto i = vec.begin(); i != vec.end(); ++i) {
				if (i->size() > 0) {
					(*i)[0] = std::toupper((*i)[0]);
					tmp += *i;
					tmp += ' ';
				}
			}
			boost::trim(tmp);

			std::wstring tmp2(tmp.begin(), tmp.end());

			for (auto i = system_fonts.begin(); i != system_fonts.end(); ++i) {
				if (i->find(tmp2) != std::wstring::npos) {
					return std::string(i->begin(), i->end());
				}
			}
		}
		return "";
	}

}}
