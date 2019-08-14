#include <io/vli.h>

namespace io {
	bool leftBitIsOne(uint8_t byte) {
		return (byte >> 7) == 1;
	}


	uint8_t rightest7Bits(uint8_t byte) {
		return byte & 0b0111'1111;
	}

	uint64_t read_variable_length_integer(std::istream& in) {
		uint8_t byte = read<uint8_t>(in);
		uint64_t count = 0; 
		while (leftBitIsOne(byte)) {
			count = (count << 7) | rightest7Bits(byte);
			byte = read<uint8_t>(in);
		}
		/*nog 1 x lezen aangezien de laatste vli bit een 0 is.*/ 
		count = (count << 7) | rightest7Bits(byte);
		

		return count;
	}
}