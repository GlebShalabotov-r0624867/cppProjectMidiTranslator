#include "io/endianness.h"

namespace io {
	void switch_endianness(uint16_t* n) {
		*n = (*n >> 8) | (*n << 8);
	}

	void switch_endianness(uint32_t* n) {
		*n = (*n >> 24) | (*n << 24) | (*n & 0x00FF0000) >> 8 | (*n & 0x0000FF00) << 8;
	}

	void switch_endianness(uint64_t* n) {
		uint64_t byte_H = (*n) >> 56;
		uint64_t byte_G = (*n & 0x00FF000000000000) >> 40;
		uint64_t byte_F = (*n & 0x0000FF0000000000) >> 24;
		uint64_t byte_E = (*n & 0x000000FF00000000) >> 8;
		uint64_t byte_D = (*n & 0x00000000FF000000) << 8;
		uint64_t byte_C = (*n & 0x0000000000FF0000) << 24;
		uint64_t byte_B = (*n & 0x000000000000FF00) << 40;
		uint64_t byte_A = (*n) << 56;

		*n = byte_A | byte_B | byte_C | byte_D | byte_E | byte_F | byte_G | byte_H;
	}
}