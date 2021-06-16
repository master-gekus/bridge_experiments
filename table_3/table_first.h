#ifndef TABLE_H
#define TABLE_H

#include <cstdint>
#include <cstring>

#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include <yaml-cpp/yaml.h>

#include "enums.hpp"
#include "moves.hpp"
#include "table_hash.hpp"

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

	inline operator underlying_type&()
	{
		return cards_;
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
	inline cards_t& suit(suit_t s) noexcept
	{
		return suites_[s];
	}

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
	using hash_type = table_hash;
	using move_type = move_t;
	using moves_type = moves_t;

public:
	inline table_t()
	{
		moves_.clear();
		update_table();
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
		update_table();
	}

public:
	void dump(std::ostream& os = std::cout) const;
	bool is_valid() const noexcept;
	void get_available_moves(moves_type& moves) const;

	// Returns winner side, if turn is finished,.and turn starter otherwise.
	side_t make_move(const ::move_t& m);

	uint64_t simplify()
	{
		uint64_t mask {0};
		mask |= simplify_suit(hands_[side_t::North].suit(suit_t::Clubs),
							  hands_[side_t::East].suit(suit_t::Clubs),
							  hands_[side_t::South].suit(suit_t::Clubs),
							  hands_[side_t::West].suit(suit_t::Clubs))
			<< 0;
		mask |= simplify_suit(hands_[side_t::North].suit(suit_t::Diamonds),
							  hands_[side_t::East].suit(suit_t::Diamonds),
							  hands_[side_t::South].suit(suit_t::Diamonds),
							  hands_[side_t::West].suit(suit_t::Diamonds))
			<< 16;
		mask |= simplify_suit(hands_[side_t::North].suit(suit_t::Hearts),
							  hands_[side_t::East].suit(suit_t::Hearts),
							  hands_[side_t::South].suit(suit_t::Hearts),
							  hands_[side_t::West].suit(suit_t::Hearts))
			<< 32;
		mask |= simplify_suit(hands_[side_t::North].suit(suit_t::Spades),
							  hands_[side_t::East].suit(suit_t::Spades),
							  hands_[side_t::South].suit(suit_t::Spades),
							  hands_[side_t::West].suit(suit_t::Spades))
			<< 48;
		return mask;
	}

	inline side_t current_player() const noexcept
	{
		return current_player_;
	}

	inline bool is_last_move() const noexcept
	{
		return is_last_move_;
	}

	inline bool is_first_move() const noexcept
	{
		return is_first_move_;
	}

	inline std::size_t max_tricks() const noexcept
	{
		return max_tricks_;
	}

	inline const suit_t& trump() const noexcept
	{
		return trump_;
	}

	inline void set_starter(const side_t& s) noexcept
	{
		turn_starter_ = s;
		update_table();
	}

	inline void set_trump(const suit_t& t) noexcept
	{
		trump_ = t;
		update_table();
	}

	inline bool empty() const noexcept
	{
		return hands_[0].empty()
			&& hands_[1].empty()
			&& hands_[2].empty()
			&& hands_[3].empty();
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
	inline void update_table() noexcept
	{
		current_player_ = turn_starter_ + moves_.size();
		is_last_move_ = (3 == moves_.size());
		is_first_move_ = moves_.empty();
		max_tricks_ = hand(current_player_).size();
	}

	static uint64_t simplify_suit(uint16_t& c1, uint16_t& c2, uint16_t& c3, uint16_t& c4)
	{
		uint16_t mask = ~(c1 | c2 | c3 | c4);

		// Drop down hi bits
		for (uint16_t sm {0x8000}; (0 != sm) && (0 != (sm & mask)); sm >>= 1)
		{
			mask &= ~sm;
		}

		// Shift suits
		for (uint16_t m {mask}, rm {0xFFFE}; m != 0; m >>= 1)
		{
			if (0 != (m & 1))
			{
				uint16_t sm = ~rm;
				c1 = (c1 & sm) | ((c1 & rm) >> 1);
				c2 = (c2 & sm) | ((c2 & rm) >> 1);
				c3 = (c3 & sm) | ((c3 & rm) >> 1);
				c4 = (c4 & sm) | ((c4 & rm) >> 1);
			}
			else {
				rm <<= 1;
			}
		}
		return static_cast<uint64_t>(mask);
	}

private:
	hand_t hands_[4];
	suit_t trump_;
	side_t turn_starter_;
	moves_type moves_;

	side_t current_player_;
	bool is_last_move_;
	bool is_first_move_;
	std::size_t max_tricks_;
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

#endif // TABLE_H
