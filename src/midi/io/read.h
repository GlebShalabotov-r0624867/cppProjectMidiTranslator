#ifndef READ_H
#define READ_H

#include <cstdint>
#include <istream>
#include "logging.h"

namespace io {
	template <typename T>
	void read_to(std::istream& input, T* buffer, size_t size) {
		input.read(reinterpret_cast<char*>(buffer), sizeof(T) * size);
		CHECK(!input.fail()) << __FUNCTION__ << "didn't pass" ;
	};

	template<typename T>
	void read_to(std::istream& input, T* buffer) {
		read_to(input, buffer, 1);
	};

	template<typename T, typename std::enable_if<std::is_fundamental<T>::value, T>::type* = nullptr>
	T read(std::istream& input)
	{
		T buffer;
		read_to(input, &buffer);
		return buffer;
	};

	template<typename T>
	std::unique_ptr<T[]> read_array(std::istream& in, size_t n) {
		std::unique_ptr<T[]> object = std::make_unique<T[]>(n);
		read_to(in, object.get(), n);
		return object;
	};
}

#endif