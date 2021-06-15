#ifndef TABLE_HASH_HPP
#define TABLE_HASH_HPP

#include <cstdint>
#include <cstring>

#include <functional>

struct table_hash
{
	inline uint8_t& operator[](std::size_t index) noexcept
	{
		return data_[index];
	}

	inline bool operator==(const table_hash& other) const noexcept
	{
		return (0 == std::memcmp(data_, other.data_, sizeof(data_)));
	}

	inline bool operator<(const table_hash& other) const noexcept
	{
		return (0 > std::memcmp(data_, other.data_, sizeof(data_)));
	}

	inline size_t hash() const
	{
		// computes the hash of an employee using a variant
		// of the Fowler-Noll-Vo hash function
		// from https://en.cppreference.com/w/cpp/utility/hash/operator()
		size_t result {2166136261};

		for (const auto c : data_)
		{
			result = (result * 16777619) ^ c;
		}

		return result;

		// const uint64_t* d {reinterpret_cast<const uint64_t*>(data_)};
		// std::hash<uint64_t> h {};
		// return h(d[0]) ^ h(d[1]) ^ h(d[2]) ^ h(d[3]);
	}

private:
	uint8_t data_[sizeof(uint64_t) * 4];
};

namespace std
{
template <>
class hash<table_hash>
{
public:
	inline size_t operator()(const table_hash& h) const
	{
		return h.hash();
	}
};
} // namespace std

#endif // TABLE_HASH_HPP
