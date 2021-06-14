#ifndef TABLE_H
#define TABLE_H

#include <cstdint>
#include <cstring>

#include <iostream>
#include <functional>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "enums.hpp"
#include "moves.hpp"

namespace first
{

class cards_t
{
private:
	using underlying_type = typename card_t::underlying_type;

public:
	inline cards_t() noexcept
		: cards_ {0}
	{
	}

	inline explicit cards_t(card_t c) noexcept
		: cards_ {static_cast<underlying_type>(c)}
	{
	}

	explicit cards_t(const char* str);

	inline explicit cards_t(const std::string& str)
		: cards_t {str.c_str()}
	{
	}

	inline explicit cards_t(const YAML::Node& n)
		: cards_t {n.IsNull() ? "" : n.as<std::string>().c_str()}
	{
	}

	~cards_t() = default;
	cards_t(const cards_t&) = default;
	cards_t(cards_t&&) = default;
	cards_t& operator=(const cards_t&) = default;
	cards_t& operator=(cards_t&&) = default;

public:
	inline bool empty() const noexcept
	{
		return (0 == cards_);
	}

	inline bool contains(card_t c) const noexcept
	{
		return (0 != (cards_ & static_cast<underlying_type>(c)));
	}

	inline bool is_intersect(cards_t other) const noexcept
	{
		return (0 != (cards_ & other.cards_));
	}

	inline void remove(card_t c) noexcept
	{
		cards_ &= ~static_cast<underlying_type>(c);
	}

	inline constexpr bool operator<(const cards_t& other) const
	{
		return cards_ < other.cards_;
	}

	inline void get_hash(void* buffer) const noexcept
	{
		*static_cast<underlying_type*>(buffer) = cards_;
	}

	std::string to_string() const;
	bool append(card_t c) noexcept;
	std::size_t size() const noexcept;
	void get_available_moves(moves_t& moves, const suit_t& suit) const;

private:
	underlying_type cards_;
};

class hand_t
{
public:
	hand_t() = default;
	~hand_t() = default;

	hand_t(const hand_t&) = default;
	hand_t(hand_t&&) = default;
	hand_t& operator=(const hand_t&) = default;
	hand_t& operator=(hand_t&&) = default;

	inline hand_t(const char* c, const char* d, const char* h, const char* s)
		: suites_ {cards_t {c}, cards_t {d}, cards_t {h}, cards_t {s}}
	{
	}

	inline explicit hand_t(const YAML::Node& n)
		: suites_ {cards_t {n["C"]}, cards_t {n["D"]}, cards_t {n["H"]}, cards_t {n["S"]}}
	{
	}

public:
	inline bool empty() const noexcept
	{
		return suites_[0].empty()
			&& suites_[1].empty()
			&& suites_[2].empty()
			&& suites_[3].empty();
	}

	inline bool append(const move_t& m) noexcept
	{
		return suites_[m.suit()].append(m.card());
	}

	inline void remove(const move_t& m) noexcept
	{
		suites_[m.suit()].remove(m.card());
	}

	inline bool contains(const move_t& m) const noexcept
	{
		return suites_[m.suit()].contains(m.card());
	}

	inline bool is_intersect(const hand_t& other) const noexcept
	{
		return suites_[0].is_intersect(other.suites_[0])
			|| suites_[1].is_intersect(other.suites_[1])
			|| suites_[2].is_intersect(other.suites_[2])
			|| suites_[3].is_intersect(other.suites_[3]);
	}

	inline bool is_move_valid(suit_t start_suit, const move_t& m) const noexcept
	{
		return (suites_[m.suit()].contains(m.card())
				&& ((m.suit() == start_suit) || suites_[start_suit].empty()));
	}

	inline std::size_t size() const
	{
		return suites_[0].size() + suites_[1].size() + suites_[2].size() + suites_[3].size();
	}

	void get_available_moves(moves_t& moves, const suit_t& suit) const;
	void dump(std::ostream& os = std::cout) const;

	inline bool operator<(const hand_t& other) const
	{
		return suites_ < other.suites_;
	}

	inline void get_hash(uint8_t* buffer) const noexcept
	{
		suites_[0].get_hash(buffer + 0);
		suites_[1].get_hash(buffer + 2);
		suites_[2].get_hash(buffer + 4);
		suites_[3].get_hash(buffer + 6);
	}

private:
	cards_t suites_[4];
};

class table_t
{
public:
	struct hash_type
	{
		inline uint8_t& operator[](std::size_t index) noexcept
		{
			return data_[index];
		}

		inline bool operator==(const hash_type& other) const noexcept
		{
			return (0 == std::memcmp(data_, other.data_, sizeof(data_)));
		}

		inline bool operator<(const hash_type& other) const noexcept
		{
			return (0 > std::memcmp(data_, other.data_, sizeof(data_)));
		}

		inline size_t hash() const
		{
			const uint64_t* d {reinterpret_cast<const uint64_t*>(data_)};
			std::hash<uint64_t> h {};
			return h(d[0]) ^ h(d[1]) ^ h(d[2]) ^ h(d[3]);
		}

	private:
		uint8_t data_[sizeof(uint64_t) * 4];
	};

	using move_type = ::move_t;
	using moves_type = ::moves_t;

public:
	inline table_t()
	{
		moves_.clear();
	};

	~table_t() = default;

	table_t(const table_t&) = default;
	table_t(table_t&&) = default;
	table_t& operator=(const table_t&) = default;
	table_t& operator=(table_t&&) = default;

	inline explicit table_t(const YAML::Node& n)
		: hands_ {hand_t {n["N"]}, hand_t {n["E"]}, hand_t {n["S"]}, hand_t {n["W"]}}
		, trump_ {n["T"].as<std::string>().c_str(), true}
		, turn_starter_ {n["TS"].as<std::string>().c_str()}
	{
		moves_.clear();
		for (const auto& m : n["M"])
		{
			moves_.push_back(move_type {m.as<std::string>().c_str()});
		}
	}

public:
	void dump(std::ostream& os = std::cout) const;
	bool is_valid() const noexcept;
	void get_available_moves(moves_type& moves) const;

	// Returns winner side, if turn is finished,.and turn starter otherwise.
	side_t make_move(const ::move_t& m);

	inline side_t current_player() const noexcept
	{
		return turn_starter_ + moves_.size();
	}

	inline const suit_t& trump() const noexcept
	{
		return trump_;
	}

	inline void set_starter(const side_t& s) noexcept
	{
		turn_starter_ = s;
	}

	inline void set_trump(const suit_t& t) noexcept
	{
		trump_ = t;
	}

	inline bool empty() const noexcept
	{
		return hands_[0].empty()
			&& hands_[1].empty()
			&& hands_[2].empty()
			&& hands_[3].empty();
	}

	inline bool is_last_move() const noexcept
	{
		return (3 == moves_.size());
	}

	inline bool is_first_move() const noexcept
	{
		return moves_.empty();
	}

	inline const hand_t& hand(const side_t& s) const noexcept
	{
		return hands_[s];
	}

	inline bool operator<(const table_t& other) const noexcept
	{
		if (trump_ != other.trump_)
		{
			return trump_ < other.trump_;
		}

		if (turn_starter_ != other.turn_starter_)
		{
			return turn_starter_ < other.turn_starter_;
		}

		return hands_ < other.hands_;
	}

	inline void get_hash(hash_type& res) const noexcept
	{
		auto ts {turn_starter_};
		hands_[ts++].get_hash(&res[8 * 0]);
		hands_[ts++].get_hash(&res[8 * 1]);
		hands_[ts++].get_hash(&res[8 * 2]);
		hands_[ts++].get_hash(&res[8 * 3]);
	}

private:
	hand_t hands_[4];
	suit_t trump_;
	side_t turn_starter_;
	moves_type moves_;
};

} // namespace first

template <typename T, typename... Types>
using is_one_of = std::disjunction<std::is_same<T, Types>...>;

template <typename T>
inline std::enable_if_t<is_one_of<T,
								  suit_t,
								  side_t,
								  move_t,
								  first::cards_t>::value,
						std::ostream&>
operator<<(std::ostream& os, const T& c)
{
	os << c.to_string();
	return os;
}

template <typename T>
inline std::enable_if_t<is_one_of<T, moves_t>::value,
						std::ostream&>
operator<<(std::ostream& os, const T& values)
{
	bool first {true};
	for (const auto& m : values)
	{
		if (!first)
		{
			std::cout << ", ";
		}
		first = false;
		std::cout << m;
	}
	return os;
}

namespace std
{
template <>
class hash<first::table_t::hash_type>
{
public:
	inline size_t operator()(const first::table_t::hash_type& h) const
	{
		return h.hash();
	}
};
}

#endif // TABLE_H
