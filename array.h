#pragma once
#include <vector>
#include <initializer_list>
#include <optional>

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
		return vec[index];
	}

	void clear() noexcept
	{
		vec.clear();
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

	// TODO: maybe a hash could make this faster?
	[[nodiscard]] std::optional<size_t> find(const Type& value) const
	{
		const size_t s = vec.size();
		for (size_t i = 0; i < s; i++)
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
