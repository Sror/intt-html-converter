#include <iostream>
#include <fstream>
#include <string>
#include <iterator>

#include "utf8-reader.h"
#include "html-convert.h"
#include "intt-convert.h"

int main(int argc, char* argv[]) {
	std::string filename;
    if (argc > 2) {
        filename = argv[2];
	} else {
        std::cerr << "Error: No input file provided or no convert type specified.\n";
		std::cerr << "Excepted parameters: " << argv[0] << " <format> <input-file>\n";
		std::cerr << "format:\n\thtml\tConvert InTT file to HTML format.\n\tintt\tConvert HTML file to InTT format.\n";
        return 1;
    }

	std::wstring storage = utf8::read_utf8(filename.c_str());
	if (storage.empty()) {
		return 1;
	}

	std::string type(argv[1]);

	if (type == "html") {
		html::Options opt;
		opt.printStyleAST = false;
		opt.printDocumentAST = false;

		html::Result result;
		html::convert(storage, result, opt);

		std::cout << "<!--RAW-HEADER\n" << result.definitions << "-->\n";
		std::cout << "<!--CLASS-MAP\n" << result.originalStyles << "-->\n";
		std::cout << "<!--COLOR-MAP\n" << result.colorTable << "-->\n";
		//std::cout << "<style type=\"text/css\"><!--\n" << result.style << "--></style>\n";
		std::cout << result.document << std::endl;
	} else if (type == "intt") {
		intt::Options opt;
		opt.printAST = false;

		intt::Result result;
		intt::convert(storage, result, opt);

		std::cout << result.document;
	} else {
		std::cerr << "Error: Unknown convert parameter: " << type << std::endl;
        return 1;
	}

	return 0;
}

