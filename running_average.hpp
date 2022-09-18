
#pragma once

#include <algorithm>

template <typename T, int size>
class RunningAverage
{
private:
	T values[size];
	int index = 0;
	T sum;

public:
	RunningAverage(T initial) : sum{initial * size}
	{
		std::fill_n(values, size, initial);
	}

	void add(T value)
	{
		T& replaced = values[index];
		sum += value - replaced;
		replaced = value;

		index = (index + 1) % size;
	}

	template <typename U>
	U get()
	{
		return static_cast<U>(sum) / size;
	}
};


