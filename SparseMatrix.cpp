#include<iostream>
#include<vector>
#include<map>
#include<algorithm>
#include <iomanip>
#include<fstream>
#include<sstream>

constexpr size_t INVALID_INDEX = size_t(-1);

class ISparseMatrix {
public:
	virtual ~ISparseMatrix() = default;
	virtual size_t n_rows() const = 0;
	virtual double value(size_t i, size_t j) const = 0;
	virtual bool is_in_stencil(size_t i, size_t j) const = 0;
protected:
	void validate_ij(size_t i, size_t j) const;

};

void ISparseMatrix::validate_ij(size_t i, size_t j) const {
	if (i > n_rows() - 1 || j > n_rows() - 1) {
		throw std::runtime_error("values out of matrix");
	}
	if(n_rows()==0) {
		throw std::runtime_error("zero rows");
	}
}


////////////
// CSRMatrix
////////////


class CsrMatrix : public ISparseMatrix {
public:
	CsrMatrix(
		const std::vector<size_t>& addr,
		const std::vector<size_t>& cols,
		const std::vector<double>& vals);

	size_t n_rows() const override;
	double value(size_t i, size_t j) const override;
	bool is_in_stencil(size_t i, size_t j) const override;
private:
	std::vector<size_t> _addr;
	std::vector<size_t> _cols;
	std::vector<double> _vals;
	size_t find_index(size_t i, size_t j) const;
};

size_t CsrMatrix::find_index(size_t i, size_t j) const {
	size_t addr0 = _addr[i];
	size_t addr1 = _addr[i + 1];

	for (size_t a = addr0; a < addr1; a++) {
		if (_cols[a] == j) {
			return a;
		}
	}
	return INVALID_INDEX;
}

CsrMatrix::CsrMatrix(const std::vector<size_t>& addr, const std::vector<size_t>& cols, const std::vector<double>& vals) {
	_addr = addr;
	_cols = cols;
	_vals = vals;
}

size_t CsrMatrix::n_rows() const {
	return _addr.size() - 1;
}

double CsrMatrix::value(size_t i, size_t j) const {
	validate_ij(i, j);
	size_t a = find_index(i, j);
	if (a != INVALID_INDEX) return _vals[a];
	else return 0.0;
}

bool CsrMatrix::is_in_stencil(size_t i, size_t j) const {
	validate_ij(i, j);
	size_t a = find_index(i, j);
	if (a != INVALID_INDEX) return true;
	else return false;
}


///////////
//LODMatrix
///////////


class LodMatrix : public ISparseMatrix {
public:
	LodMatrix(size_t nrows);

	void set_value(size_t i, size_t j, double val);
	void clear_row(size_t i);
	CsrMatrix to_csr() const;

	size_t n_rows() const override;
	double value(size_t i, size_t j) const override;
	bool is_in_stencil(size_t i, size_t j) const override;
private:
	std::vector<std::map<size_t, double>> _data;
};

LodMatrix::LodMatrix(size_t nrows) {
	std::map<size_t, double> v;
	for (size_t i = 0; i < nrows; i++) {
		_data.push_back(v);
	}
}

void LodMatrix::set_value(size_t i, size_t j, double val) {
	validate_ij(i, j);
	//auto fnd = _data[i].find(j);
	//if (fnd != _data[i].end()) {
	//	fnd->second = val;
	//}
	//else
	//	_data[i].insert({ j, val });
	_data[i][j] = val;
}

void LodMatrix::clear_row(size_t i) {
	validate_ij(i, 0);
	_data[i].clear();
}

CsrMatrix LodMatrix::to_csr() const {
	std::vector<size_t> addr;
	std::vector<size_t> cols;
	std::vector<double> vals;
	addr.push_back(0.0);
	int cout = 0;
	for (size_t i = 0; i < _data.size(); ++i) {
		for (auto it = _data[i].cbegin(); it != _data[i].cend(); ++it)
		{
			vals.push_back(it->second);
			cols.push_back(it->first);
			cout++;
		}
		addr.push_back(cout);
	}
	CsrMatrix s = CsrMatrix(addr, cols, vals);
	return s;
}

size_t LodMatrix::n_rows()const {
	return _data.size();
}

double LodMatrix::value(size_t i, size_t j)const {
	validate_ij(i, j);
	auto fnd = _data[i].find(j);
	if (fnd != _data[i].end()) {
		return fnd->second;
	}
	else return 0.0;
}

bool LodMatrix::is_in_stencil(size_t i, size_t j)  const {
	validate_ij(i, j);
	auto fnd = _data[i].find(j);
	if (fnd != _data[i].end()) return true;
	else return false;
}


///////
//Print
///////


void print_matrix_full(const ISparseMatrix& mat, std::ostream& s = std::cout) {
	for (size_t i = 0; i < mat.n_rows(); i++) {
		for (size_t j = 0; j < mat.n_rows(); j++) {
			if (mat.is_in_stencil(i, j) == false) {
				s << std::setw(4) << "*";
			}
			else {
				s << std::setw(4) << mat.value(i, j);
			}
		}
		s << std::endl;
	}
}

void print_matrix_stencil(size_t irow, const ISparseMatrix& mat, std::ostream& s = std::cout) {
	s << "ROW = " << irow << std::endl;
	for (size_t j = 0; j < mat.n_rows() - 1; j++) {
		if (mat.is_in_stencil(irow, j) == true) {
			s << j << ": " << mat.value(irow, j) << std::endl;
		}
	}
}

void print_matrix_stencil(const ISparseMatrix& mat, std::ostream& s = std::cout) {
	s << "NROWS = " << mat.n_rows() << std::endl;
	for (size_t i = 0; i < mat.n_rows(); i++) {
		print_matrix_stencil(i,mat,s);
	}
}

///////
//TESTS
///////

bool CHECK(bool cond) {
	if (!cond) throw std::runtime_error("test failed");
}

void test_sparse_matrix() {
	bool exception_thrown;

	// =========== constructor
	std::vector<size_t> addr{ 0, 1, 3, 4 };
	std::vector<size_t> cols{ 2, 0, 2, 1 };
	std::vector<double> vals{ 10, 4, 2, 3 };
	CsrMatrix cmat(addr, cols, vals);
	// ============ n-rows
	CHECK(cmat.n_rows() == 3);
	// ============ value
	CHECK(cmat.value(0, 0) == 0);
	CHECK(cmat.value(0, 1) == 0);
	CHECK(cmat.value(0, 2) == 10.0);
	CHECK(cmat.value(1, 0) == 4);
	CHECK(cmat.value(1, 1) == 0);
	CHECK(cmat.value(1, 2) == 2);
	CHECK(cmat.value(2, 0) == 0);
	CHECK(cmat.value(2, 1) == 3);
	CHECK(cmat.value(2, 2) == 0);
	try {
		exception_thrown = false;
		cmat.value(2, 3);
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	// ============ is_in_stencil
	CHECK(cmat.is_in_stencil(0, 0) == false);
	CHECK(cmat.is_in_stencil(0, 2) == true);
	CHECK(cmat.is_in_stencil(2, 2) == false);
	try {
		exception_thrown = false;
		cmat.value(3, 0);
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);

	//LODMatrix
	LodMatrix lmat(3);
	// nrows
	CHECK(lmat.n_rows() == 3);
	CHECK(lmat.value(0, 2) == 0);
	// set_value
	lmat.set_value(0, 2, 10.0);
	lmat.set_value(1, 0, 4.0);
	lmat.set_value(1, 2, 2.0);
	lmat.set_value(2, 1, 3.0);
	try {
		exception_thrown = false;
		lmat.set_value(10, 10, 1.0);
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	// value
	CHECK(lmat.value(0, 2) == 10.0);
	CHECK(lmat.value(2, 1) == 3.0);
	CHECK(lmat.value(2, 2) == 0.0);
	try {
		exception_thrown = false;
		lmat.value(1, 3);
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	// convert
	CsrMatrix cmat2 = lmat.to_csr();
	//check conversion through print
	std::ostringstream ss_lmat, ss_cmat, ss_cmat2;
	print_matrix_full(lmat, ss_lmat);
	print_matrix_full(cmat, ss_cmat);
	print_matrix_full(cmat2, ss_cmat2);
	CHECK(ss_lmat.str() == ss_cmat2.str());
	CHECK(ss_cmat.str() == ss_cmat2.str());

}

void test_sparse_matrix2() {
	bool exception_thrown;

	LodMatrix lmat(3);
	lmat.set_value(0, 0, 1);
	lmat.set_value(1, 0, 2);
	lmat.set_value(2, 0, 3);
	try {
		lmat.set_value(3, 0, 4);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	try {
		lmat.set_value(0, 3, 4);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	lmat.clear_row(2);
	try {
		lmat.clear_row(3);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	CHECK(lmat.value(0, 0) == 1);
	CHECK(lmat.value(1, 0) == 2);
	CHECK(lmat.is_in_stencil(2, 0) == false);
	CHECK(lmat.value(2, 0) == 0);

	lmat.set_value(0, 0, 0);
	CHECK(lmat.value(0, 0) == 0);

	CsrMatrix cmat = lmat.to_csr();
	//CsrMatrix cmat({0, 1, 1, 2}, {0, 0}, {1, 2});

	CHECK(cmat.value(0, 0) == 0);
	CHECK(cmat.value(1, 0) == 2);
	CHECK(cmat.is_in_stencil(2, 0) == false);
	CHECK(cmat.value(2, 0) == 0);
	try {
		cmat.value(3, 0);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	try {
		cmat.is_in_stencil(0, 3);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);

	CHECK(cmat.is_in_stencil(1, 2) == false);
	CHECK(cmat.value(1, 2) == 0);

	std::ostringstream oss_cmat, oss_lmat;
	print_matrix_full(lmat, oss_lmat);
	print_matrix_full(cmat, oss_cmat);
	CHECK(oss_lmat.str() == oss_cmat.str());
}

void test_sparse_matrix3() {
	bool exception_thrown;

	LodMatrix lmat(0);
	CHECK(lmat.n_rows() == 0);
	try {
		lmat.value(0, 0);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	try {
		lmat.set_value(0, 0, 0);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	try {
		lmat.clear_row(0);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);

	CsrMatrix cmat({ 0 }, {}, {});
	CHECK(cmat.n_rows() == 0);
	try {
		cmat.value(0, 0);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
	try {
		cmat.is_in_stencil(0, 0);
		exception_thrown = false;
	}
	catch (std::runtime_error&) {
		exception_thrown = true;
	}
	CHECK(exception_thrown);
}


int main()
{
	try {
		test_sparse_matrix();
		test_sparse_matrix2();
		test_sparse_matrix3();
		return EXIT_SUCCESS;
	}
	catch (std::runtime_error& e) {
		std::cout << "Exception thrown: " << std::endl;
		std::cout << "    " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}
