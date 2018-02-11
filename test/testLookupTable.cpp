
#include <assert.h>
#include <iostream>
#include "LookupTable.h"
#include "AudioMath.h"

using namespace std;

// test that we can call all the functions
template<typename T>
static void test0() {
	LookupTableParams<T> p;
	const int tableSize = 512;

	std::function<double(double)> f = [](double d) {
		return 0;
	};

	LookupTable<T>::init(p, tableSize, 0, 1, f);
	LookupTable<T>::lookup(p, 0);
}

// test that simple lookup works
template<typename T>
static void test1() {
	LookupTableParams<T> p;
	const int tableSize = 512;

	std::function<double(double)> f = [](double d) {
		return 100;
	};

	LookupTable<T>::init(p, tableSize, 0, 1, f);

	assert(LookupTable<T>::lookup(p, 0) == 100);
	assert(LookupTable<T>::lookup(p, 1) == 100);
	assert(LookupTable<T>::lookup(p, T(.342)) == 100);
}


// test that sin works
template<typename T>
static void test2() {
	LookupTableParams<T> p;
	const int tableSize = 512;

	std::function<double(double)> f = [](double d) {
		return std::sin(d);
	};

	LookupTable<T>::init(p, tableSize, 0, 1, f);

	const T tolerance = T(0.000001);
	for (double d = 0; d < 1; d += .0001) {
		T output = LookupTable<T>::lookup(p, T(d));

		const bool t = AudioMath::closeTo(output, std::sin(d), tolerance);
		if (!t) {
			cout << "failing with expected=" << std::sin(d) << " actual=" << output << " delta=" << std::abs(output - std::sin(d));
			assert(false);
		}
	}
}


// test that sin works on domain 10..32
template<typename T>
static void test3() {
	LookupTableParams<T> p;
	const int tableSize = 16;

	std::function<double(double)> f = [](double d) {
		const double s = (d - 10) / 3;
		return std::sin(s);
	};

	LookupTable<T>::init(p, tableSize, 10, 13, f);

	const T tolerance = T(0.01);
	for (double d = 10; d < 13; d += .0001) {
		T output = LookupTable<T>::lookup(p, T(d));

		const T expected = (T)std::sin((d - 10.0) / 3);
		const bool t = AudioMath::closeTo(output, expected, tolerance);
		if (!t) {
			cout << "failing with d=" << d << " expected=" << expected << " actual=" << output << " delta=" << std::abs(output - std::sin(d));
			assert(false);
		}
	}
}

template<typename T>
static void test() {
	test0<T>();
	test1<T>();
	test2<T>();
	test3<T>();
}

void testLookupTable() {
	test<float>();
	test<double>();
}