#pragma once
#include <cmath>

class Random
{
public:
	[[nodiscard]] unsigned long long operator()() noexcept
	{
		x = (a * x + c) % m;
		return x;
	}

	void setseed(const unsigned long long seed) noexcept
	{
		x = seed;
	}

private:
	const unsigned long long c{ 1013904223 };
	const unsigned long long a{ 1664525 };
	const unsigned long long m{ static_cast<unsigned long long>(std::pow(2, 32)) };
	unsigned long long x{ 1 };
};

class Hash
{
public:
	[[nodiscard]] unsigned int operator()(unsigned int key) const noexcept
	{
		key += ~(key << 15);
		key ^= key >> 10;
		key += key << 3;
		key ^= key >> 6;
		key += ~(key << 11);
		key ^= key >> 16;
		return key;
	}

	[[nodiscard]] unsigned int operator()(const unsigned int x, const unsigned int y) const noexcept
	{
		return (*this)(x ^ (*this)(y ^ s));
	}

	void setseed(const unsigned int seed) noexcept
	{
		s = seed;
	}

private:
	unsigned int s{ 1 };
};
