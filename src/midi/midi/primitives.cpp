#include "primitives.h"

namespace midi {

	Duration operator +(const Duration& duration1, const Duration& duration2) {
		return Duration(value(duration1) + value(duration2));
	};

	Duration operator -(const Duration& duration1, const Duration& duration2) {
		return Duration(value(duration1) - value(duration2));
	};

	Duration operator -(const Time& time1, const Time& time2) {
		return Duration(value(time1) - value(time2));
	};

	Time operator +(const Time& time1, const Duration& dur1) {
		return Time(value(time1) + value(dur1));
	};

	Time operator +(const Duration& duration1, const Time& time1) {
		return Time(value(duration1) + value(time1));
	};


	Time& operator +=(Time& time1, const Duration& duration1) {
		value(time1) += value(duration1);
		return time1;
	};

	Duration& operator +=(Duration& duration1, const Duration& duration2) {
		value(duration1) += value(duration2);
		return duration1;
	};

	Duration& operator -=(Duration& duration1, const Duration& duration2) {
		value(duration1) -= value(duration2);
		return duration1;
	};
}