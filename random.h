#pragma once
#include <cmath>

//*********************************************************************************************************************

class Random
{
public:
	unsigned long long operator()()
	{
		x = (a * x + c) % m;
		return x;
	}

	void setseed(unsigned long long seed)
	{
		x = seed;
	}

private:
	const unsigned long long c = 1013904223;
	const unsigned long long a = 1664525;
	const unsigned long long m = static_cast<unsigned long long>(std::pow(2, 32));
	unsigned long long x = 1;
};

//*********************************************************************************************************************

class Hash
{
public:
	unsigned int operator()(unsigned int key)
	{
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}

	unsigned int operator()(unsigned int x, unsigned int y)
	{
		return (*this)(x ^ (*this)(y ^ s));
	}

	void setseed(unsigned int seed)
	{
		s = seed;
	}

private:
	unsigned int s = 1;
};

//*********************************************************************************************************************