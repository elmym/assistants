#include "HEADER_CASE.h"

TEST_CASE("[test1]") {
	std::cout << "testik 1" << std::endl;
	throw std::runtime_error("ERR");
}

TEST_CASE("[test2]") {
	std::cout << "testik 2" << std::endl;
	throw std::runtime_error("ERR2");
}

TEST_CASE("[test3]") {
	std::cout << "testik 3" << std::endl;
	throw std::runtime_error("ERR2");
}

int main(int argc, char* argv[]) {
	std::vector<std::string> argv_s;
	for (int i = 1; i < argc; ++i) {
		argv_s.push_back(std::string(argv[i]));
	}
	//run_test_cases({ "[test1]", "[test2]", "[test3]" });
	run_test_cases(argv_s);
}
