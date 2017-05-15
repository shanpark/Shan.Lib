//
//  matrix.h
//  MFNN
//
//  Created by Sung Han Park on 2017. 5. 8..
//  Copyright © 2017 Sung Han Park. All rights reserved.
//

#ifndef shan_math_matrix_h
#define shan_math_matrix_h

#include <cassert>
#include <random>
#include "object.h"
#include "util.h"

namespace shan {
namespace math {

template<typename T>
class matrix : public object {
public:
	using apply_func_t = T(T);
	struct index {
		std::size_t row;
		std::size_t col;
	};
	using index_t = index;

public:
	matrix()
	: _rows(0), _cols(0), _data_ptr(nullptr) {}

	matrix(std::size_t rows, std::size_t cols)
	: _rows(rows), _cols(cols), _data_ptr(new T[rows * cols]) {}

	std::size_t rows() const {
		return _rows;
	}
	std::size_t cols() const {
		return _cols;
	}
	const T* data() const {
		return _data_ptr.get();
	}
	T* data() {
		return _data_ptr.get();
	}

	T& at(std::size_t rows, std::size_t cols) {
		assert((rows < _rows) && (cols < _cols));
		return _data_ptr.get()[rows * _cols + cols];
	}
	const T& at(std::size_t rows, std::size_t cols) const {
		assert((rows < _rows) && (cols < _cols));
		return _data_ptr.get()[rows * _cols + cols];
	}

	void load(const std::vector<T>& data) {
		std::size_t len = std::min(data.size(), _rows * _cols);
		std::memcpy(_data_ptr.get(), &data[0], len * sizeof(T));
	}

	void load(const T* data) {
		std::size_t len = _rows * _cols;
		std::memcpy(_data_ptr.get(), data, len * sizeof(T));
	}

	void random(double low, double high) {
		std::uniform_real_distribution<double> urd(low, high);

		std::size_t count = _rows * _cols;
		T* dest = _data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = urd(random_engine());
	}

	T min(index_t* index_p = nullptr) {
		T val = at(0, 0);
		if (index_p) {
			index_p->row = 0;
			index_p->col = 0;
		}

		std::size_t count = _rows * _cols;
		const T* src = _data_ptr.get();
		for (std::size_t inx = 1 ; inx < count ; inx++) {
			if (src[inx] < val) {
				val = src[inx];
				if (index_p) {
					index_p->row = inx / _cols;
					index_p->col = inx % _cols;;
				}
			}
		}

		return val;
	}

	T max(index_t* index_p = nullptr) {
		T val = at(0, 0);
		if (index_p) {
			index_p->row = 0;
			index_p->col = 0;
		}

		std::size_t count = _rows * _cols;
		const T* src = _data_ptr.get();
		for (std::size_t inx = 1 ; inx < count ; inx++) {
			if (src[inx] > val) {
				val = src[inx];
				if (index_p) {
					index_p->row = inx / _cols;
					index_p->col = inx % _cols;;
				}
			}
		}

		return val;
	}

	matrix<T> dot(const matrix<T>& input) const {
		assert(_cols == input._rows);

		matrix<T> m(_rows, input._cols);

		T* dest = m._data_ptr.get();
		const T* row_src_start = _data_ptr.get();
		const T* col_src_start = input._data_ptr.get();
		const T* lpos;
		const T* rpos;
		for (std::size_t r = 0 ; r < m._rows ; r++) {
			for (std::size_t c = 0 ; c < m._cols ; c++) {
				lpos = row_src_start;
				rpos = col_src_start;
				*dest = 0;
				for (std::size_t inx = 0 ; inx < _cols ; inx++) {
					*dest += (*lpos * *rpos);

					lpos++;
					rpos += input._cols;
				}
				dest++;
				col_src_start++;
			}
			col_src_start = input._data_ptr.get(); // rewind.
			row_src_start += _cols;
		}

		return m;
	}

	matrix<T> transpose() const {
		matrix<T> m(_cols, _rows);

		const T* src = _data_ptr.get();
		T* dest = m._data_ptr.get();
		const T* row_pos;
		for (std::size_t c = 0 ; c < _cols ; c++) {
			row_pos = src + c;
			for (std::size_t r = 0 ; r < _rows ; r++) {
				*dest = *row_pos;
				dest++;
				row_pos += _cols;
			}
		}

		return m;
	}

	matrix<T> apply(std::function<apply_func_t> func) const {
		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* src = _data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = func(src[inx]);

		return m;
	}

	/**
	 element-wise plus functions.
	 */
	matrix<T> operator+(const matrix<T>& rhs) const {
		assert((_rows == rhs._rows) && (_cols == rhs._cols));

		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* lsrc = _data_ptr.get();
		const T* rsrc = rhs._data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = lsrc[inx] + rsrc[inx];

		return m;
	}

	/**
	 scalar plus functions.
	 */
	matrix<T> operator+(const T& rhs) const {
		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* src = _data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = src[inx] + rhs;

		return m;
	}

	/**
	 element-wise minus functions.
	 */
	matrix<T> operator-(const matrix<T>& rhs) const {
		assert((_rows == rhs._rows) && (_cols == rhs._cols));

		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* lsrc = _data_ptr.get();
		const T* rsrc = rhs._data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = lsrc[inx] - rsrc[inx];

		return m;
	}

	/**
	 scalar minus functions.
	 */
	matrix<T> operator-(const T& rhs) const {
		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* src = _data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = src[inx] - rhs;

		return m;
	}

	/**
	 element-wise multiplication functions.
	 */

	matrix<T> operator*(const matrix<T>& rhs) const {
		assert((_rows == rhs._rows) && (_cols == rhs._cols));

		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* lsrc = _data_ptr.get();
		const T* rsrc = rhs._data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = lsrc[inx] * rsrc[inx];

		return m;
	}

	/**
	 scalar multiplication functions.
	 */
	matrix<T> operator*(const T& rhs) const {
		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* src = _data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = src[inx] * rhs;

		return m;
	}

	/**
	 element-wise division functions.
	 */
	matrix<T> operator/(const matrix<T>& rhs) const {
		assert((_rows == rhs._rows) && (_cols == rhs._cols));

		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* lsrc = _data_ptr.get();
		const T* rsrc = rhs._data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = lsrc[inx] / rsrc[inx];

		return m;
	}

	/**
	 scalar division functions.
	 */
	matrix<T> operator/(const T& rhs) const {
		matrix<T> m(_rows, _cols);

		std::size_t count = _rows * _cols;
		const T* src = _data_ptr.get();
		T* dest = m._data_ptr.get();
		for (std::size_t inx = 0 ; inx < count ; inx++)
			dest[inx] = src[inx] / rhs;

		return m;
	}

	virtual std::ostream& str(std::ostream& os) const {
		os << "[[";
		for (std::size_t r = 0 ; r < _rows ; r++) {
			for (std::size_t c = 0 ; c < _cols ; c++) {
				os << at(r, c);
				if (c < _cols - 1)
					os << ", ";
			}
			os << "]";
			if (r < _rows - 1)
				os << "," << std::endl << " [";
		}
		os << "]";

		return os;
	}
	
private:
	std::default_random_engine& random_engine() {
		static std::default_random_engine re(static_cast<std::default_random_engine::result_type>(std::time(nullptr)));
		return re;
	}

private:
	std::size_t _rows;
	std::size_t _cols;
	std::unique_ptr<T[]> _data_ptr;
};

template<typename T>
matrix<T> operator+(const T& lhs, const matrix<T>& rhs) {
	return (rhs + lhs);
}

template<typename T>
matrix<T> operator-(const T& lhs, const matrix<T>& rhs) {
	matrix<T> m(rhs.rows(), rhs.cols());

	std::size_t count = rhs.rows() * rhs.cols();
	const T* src = rhs.data();
	T* dest = m.data();
	for (std::size_t inx = 0 ; inx < count ; inx++)
		dest[inx] = lhs - src[inx];

	return m;
}

template<typename T>
matrix<T> operator*(const T& lhs, const matrix<T>& rhs) {
	return (rhs * lhs);
}

template<typename T>
matrix<T> operator/(const T& lhs, const matrix<T>& rhs) {
	matrix<T> m(rhs.rows(), rhs.cols());

	std::size_t count = rhs.rows() * rhs.cols();
	const T* src = rhs.data();
	T* dest = m.data();
	for (std::size_t inx = 0 ; inx < count ; inx++)
		dest[inx] = lhs / src[inx];

	return m;
}

} // namespace math
} // namespace shan

#endif /* shan_math_matrix_h */
