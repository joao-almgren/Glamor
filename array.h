#pragma once
#include <vector>
#include <initializer_list>
#include <optional>
#include <map>

template <typename Type>
class Array
{
public:
	[[nodiscard]] Type* data() noexcept
	{
		return vec.data();
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return vec.size();
	}

	[[nodiscard]] Type& operator[](const size_t index)
	{
		hash.clear(); // TODO: maybe erase instead
		return vec[index];
	}

	void clear() noexcept
	{
		vec.clear();
		hash.clear();
	}

	size_t append(const Type& value)
	{
		const size_t i = vec.size();
		vec.push_back(value);
		return i;
	}

	size_t append(const std::initializer_list<Type>& list)
	{
		const size_t i = vec.size();
		for (const Type& value : list)
			vec.push_back(value);
		return i;
	}

	[[nodiscard]] std::optional<size_t> find(const Type& value)
	{
		auto iter = hash.find(value);
		if (iter != hash.end())
			return iter->second;
		const size_t s = vec.size();
		for (size_t i = 0; i < s; i++)
			if (value == vec[i])
			{
				hash.insert({ value, i });
				return i;
			}
		return std::nullopt;
	}

	size_t appendAbsent(const Type& value)
	{
		const std::optional<size_t> i = find(value);
		if (i.has_value())
			return i.value();
		return append(value);
	}

private:
	std::vector<Type> vec;
	std::map<Type, size_t> hash;
};
