#include <vector>
#include <sstream>
#include <boost/lexical_cast.hpp>

#include "header.h"

namespace intt { namespace impl {

	Header getHeader(const std::string& str) {
		Header hdr;
		hdr.position = 0;
		hdr.corrupted = false;

		std::string::size_type pos = 0, end = 0;

		pos = str.find("<!--RAW-HEADER\n");
		if (pos != std::string::npos) {
			pos = str.find_first_of("<", pos + 1);
			end = str.find("-->", pos);
			if (end != std::string::npos) {
				hdr.rawDef = str.substr(pos, end - pos);
			} else {
				hdr.corrupted = true;
			}
		} else {
			hdr.corrupted = true;
			return hdr;
		}

		pos = str.find("<!--CLASS-MAP\n", end);
		if (pos != std::string::npos) {
			end = str.find("-->", pos);
			if (end != std::string::npos) {
				std::istringstream is(str.substr(pos, end - pos));
				std::string line;
				getline(is, line);
				while (getline(is, line)) {
					std::string first, second;
					pos = line.find_first_of(':');
					first = line.substr(0, pos);
					second = line.substr(pos + 1);

					hdr.classDef.insert(std::make_pair(first, second));
				}
			} else {
				hdr.corrupted = true;
			}
		} else {
			return hdr;
		}

		pos = str.find("<!--COLOR-MAP\n", end);
		if (pos != std::string::npos) {
			end = str.find("-->", pos);
			if (end != std::string::npos) {
				std::istringstream is(str.substr(pos, end - pos));
				std::string line;
				getline(is, line);
				while (getline(is, line)) {
					std::string first, second;
					pos = line.find_first_of(':');
					if (pos == std::string::npos){
						continue;
					}

					first = line.substr(0, pos);
					second = line.substr(pos + 1);

					hdr.colorDef.insert(std::make_pair(first, second));
				}
			} else{
				hdr.corrupted = true;
			}
		} else {
			return hdr;
		}

 		hdr.position = str.find_first_of('<', end);

		return hdr;
	}

}}
