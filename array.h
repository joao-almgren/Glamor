#pragma once
#include <vector>
#include <initializer_list>
#include <optional>

template <typename Type>
class Array
{
public:
	Type* data()
	{
		return vec.data();
	}

	size_t size() const
	{
		return vec.size();
	}

	Type& operator[](const size_t index)
	{
		return vec[index];
	}

	void clear()
	{
		vec.clear();
	}

	size_t append(const Type& value)
	{
		const size_t i = size();
		vec.push_back(value);
		return i;
	}

	size_t append(const std::initializer_list<Type>& list)
	{
		const size_t i = size();
		for (const Type& value : list)
			vec.push_back(value);
		return i;
	}

	std::optional<size_t> find(const Type& value) const
	{
		for (size_t i = 0; i < size(); i++)
			if (value == vec[i])
				return i;
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
	std::vector<Type> vec{};
};
