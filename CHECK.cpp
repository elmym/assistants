#include <iostream>
#include <vector>
#define GET_LINE c.get_line(__LINE__, __FILE__)
#define CHECK 	GET_LINE; c.check 
#define REQUIRE GET_LINE; c.require
#define CHECK_THROWS(x) {GET_LINE; bool BIG_BOOL = false; try {x;} catch (std::runtime_error) {BIG_BOOL=true;} c.check(BIG_BOOL);}
#define REQUIRE_THROWS(x) {GET_LINE; bool BIG_BOOL = false; try {x;} catch (std::runtime_error) {BIG_BOOL=true;} c.require(BIG_BOOL);}




class CHECKS {
public:
	~CHECKS() {
		for (int i = 0; i < _fal_lines.size(); ++i) {
			std::cout << "- Assertion at " << _fal_files[i] << ":" << _fal_lines[i] << "  failed" << std::endl;
		}
		std::cout << std::endl << "=======================" << std::endl;
		std::cout << "Total assertions :" << _tests.size() << std::endl << "Failed assertions :" << _fal_lines.size() << std::endl;
	}

	void check(bool a);
	void require(bool a);
	void get_line(int a, std::string b);
	//friend bool operator== (double a, );
private:
	int _line;
	std::string _file;
	std::vector<int> _fal_lines;
	std::vector<std::string> _fal_files;
	std::vector<int> _tests;
};

CHECKS c;
void CHECKS::check(bool a) {
	_tests.push_back(1);
	if (a == false) _fal_lines.push_back(_line); _fal_files.push_back(_file);
}

void CHECKS::require(bool a) {
	_tests.push_back(1);
	if (a == false) {
		_fal_lines.push_back(_line);
		_fal_files.push_back(_file);
		exit(0);
	}
}


void CHECKS::get_line(int a, std::string b) {
	_line = a;
	_file = b;
}

void func(int t) {
	if (t == 1) throw std::runtime_error("err");
}

int main() {
	REQUIRE(1 == 1);
	//REQUIRE(1 == 2);
	CHECK_THROWS(func(1));
	CHECK_THROWS(func(0));
	REQUIRE_THROWS(func(0));
	CHECK(1 == 3);
	CHECK(3 == 3);
	CHECK(1.0000000000000000001 == 1);
	CHECK(2 == 1);
}
