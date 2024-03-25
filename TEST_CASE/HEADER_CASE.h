#ifndef HEAD
#define HEAD

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <exception>


#define MM(x,y) x##y
#define TT(x,y) MM(x, y)
#define FOR_NAME_CLASS(x,y) TT(s, __LINE__)
#define FOR_NAME_FUNC(x,y) TT(test_, __LINE__)
#define NAME_FUNC FOR_NAME_FUNC(x, y)
#define NAME_CLASS FOR_NAME_CLASS(x, y)
#define TEST_CASE(x) static void NAME_FUNC (); static NewTest NAME_CLASS(x, NAME_FUNC); void NAME_FUNC () 


class Tests {
public:
	void new_test(std::string str, std::function<void()> fun) {
		_functions[str] = fun;
	}
	std::pair<std::string, std::string> call_function(std::string str) {
		std::pair<std::string, std::string> errors;
		try {
			_functions[str]();
		}
		catch (const std::exception& e) {
			errors.first = str;
			errors.second = e.what();
		}
		return errors;
	}
	std::vector<std::string> all_names() {
		std::vector<std::string> res;
		for (auto it : _functions) {
			res.push_back(it.first);
		}
		return res;
	}
	static Tests& instance() {
		static Tests a;
		return a;
	}
private:
	std::map<std::string, std::function<void()>> _functions;
};



class NewTest {
public:
	NewTest(std::string s, std::function<void()> f) {
		Tests::instance().new_test(s, f);
	}
};

inline void run_test_cases(std::vector<std::string> argv) {
	std::map<std::string, std::string> errors;
	int err_count = 0;
	if (argv.size() == 0) argv = Tests::instance().all_names();
	for (int i = 0; i < argv.size(); i++) {
		auto  error = Tests::instance().call_function(argv[i]);
		if (error.first != "") {
			errors.insert(error);
			err_count++;
		}
	}

	int size = argv.size();
	for (auto i : errors) {
		std::cout << "\n! TEST_CASE" << i.first << " failed with error message : \"" << i.second << "\"\n";
	}
	std::cout << "\n===================\n\n";
	std::cout << size - err_count;
	(size - err_count == 1) ? std::cout << " test passed" : std::cout << " tests passed ";

	if (err_count != 0) {
		std::cout << ", " << err_count;
		(err_count == 1) ? std::cout << " test falied\n" : std::cout << " tests falied\n";
	}
}

#endif 
