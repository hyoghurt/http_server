#include <iostream>
#include <unistd.h>

int deleteFile(const std::string &str) {

	/*
	 * incorrect file
	 *
	*/
	std::string path = getenv("PWD");
	std::cout << "PWD: " << path << std::endl;

	if (path.empty()) { return 500; }
	std::string absolutPath = path + std::string("/") + str;
	std::cout << absolutPath << std::endl;

	if (unlink(str.c_str()) == 0) {
		std::cout << "\"" << str << "\" was deleted" << std::endl;
		return 200;
	} else {
		std::cout << "something wrong" << std::endl;
		return 404;
	}
}
//
//int main() {
//	std::string str  = "dir/file.html";
//
//	std::cout << "" << deleteFile(str) << std::endl;
//	return 0;
//}
