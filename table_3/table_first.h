#ifndef TABLE_H
#define TABLE_H

#include <cstdint>

#include <array>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "enums.hpp"


class move_ex_t;

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
	void get_moves(std::vector<move_ex_t>& res, const suit_t& suit) const;

private:
	underlying_type cards_;
};

class move_t
{
public:
	move_t() = default;
	~move_t() = default;

	move_t(const move_t&) = default;
	move_t(move_t&&) = default;
	move_t& operator=(const move_t&) = default;
	move_t& operator=(move_t&&) = default;

	inline move_t(const card_t& c, const suit_t& s)
		: card_ {c}
		, suit_ {s}
	{
	}

	explicit move_t(const char* str);
	inline explicit move_t(const YAML::Node& n)
		: move_t {n.as<std::string>().c_str()}
	{
	}

public:
	inline const card_t& card() const noexcept
	{
		return card_;
	}

	inline const suit_t& suit() const noexcept
	{
		return suit_;
	}

	inline std::string to_string() const
	{
		return std::string {card_.to_string()} + suit_.to_string_short();
	}

	inline bool constexpr is_beat(const move_t& m, const suit_t& trump) const noexcept
	{
		return (m.suit_ == suit_) ? (card_ > m.card_) : (trump == suit_);
	}

	inline bool constexpr is_neighbor(const move_t& other) const noexcept
	{
		return (suit_ == other.suit_) && card_.is_neighbors(other.card_);
	}

private:
	card_t card_;
	suit_t suit_;
};

class move_ex_t : public move_t
{
public:
	move_ex_t()
		: tricks_ {0}
	{
	}
	move_ex_t(const card_t& c, const suit_t& s)
		: move_t {c, s}
		, tricks_ {0}
	{
	}

	template <typename T, typename = std::enable_if<std::is_integral_v<T>>>
	move_ex_t(const card_t& c, const suit_t& s, T t)
		: move_t {c, s}
		, tricks_ {static_cast<uint8_t>(t)}
	{
	}

	~move_ex_t() = default;

	move_ex_t(const move_ex_t&) = default;
	move_ex_t(move_ex_t&&) = default;
	move_ex_t& operator=(const move_ex_t&) = default;
	move_ex_t& operator=(move_ex_t&&) = default;

	inline bool operator<(const move_ex_t& other) const noexcept
	{
		return tricks_ < other.tricks_;
	}

public:
	inline std::size_t tricks() const noexcept
	{
		return static_cast<std::size_t>(tricks_);
	}

	template <typename T>
	inline std::enable_if_t<std::is_integral_v<T>>
	add_tricks(T tricks_to_add) noexcept
	{
		tricks_ += static_cast<uint8_t>(tricks_to_add);
	}

	template <typename T>
	inline std::enable_if_t<std::is_integral_v<T>>
	set_tricks(T new_tricks) noexcept
	{
		tricks_ = static_cast<uint8_t>(new_tricks);
	}

	inline bool is_max() const noexcept
	{
		return (std::numeric_limits<decltype(tricks_)>::max() == tricks_);
	}

private:
	uint8_t tricks_;
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

	std::vector<move_ex_t> available_moves(const suit_t& suit) const;
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
	std::array<cards_t, 4> suites_;
};

class table_t
{
public:
	using hash_t = std::array<uint8_t, sizeof(uint64_t) * 4>;

public:
	table_t() = default;
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
		for (const auto& m : n["M"])
		{
			moves_.emplace_back(m);
		}
	}

public:
	void dump(std::ostream& os = std::cout) const;
	bool is_valid() const noexcept;
	inline std::vector<move_ex_t> available_moves() const
	{
		return hands_[turn_starter_ + moves_.size()].available_moves(moves_.empty()
																		 ? suit_t::NoTrump
																		 : moves_.front().suit());
	}

	// Returns winner side, if turn is finished,.and turn starter otherwise.
	side_t make_move(const move_t& m);

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

	inline hash_t hash() const noexcept
	{
		hash_t res;
		auto ts {turn_starter_};
		hands_[ts++].get_hash(&res[8 * 0]);
		hands_[ts++].get_hash(&res[8 * 1]);
		hands_[ts++].get_hash(&res[8 * 2]);
		hands_[ts++].get_hash(&res[8 * 3]);
		return res;
	}

private:
	std::array<hand_t, 4> hands_;
	suit_t trump_;
	side_t turn_starter_;
	std::vector<move_t> moves_;
};

template <typename T, typename... Types>
using is_one_of = std::disjunction<std::is_same<T, Types>...>;

template <typename T>
inline std::enable_if_t<is_one_of<T, cards_t, suit_t, side_t, move_t, move_ex_t>::value, std::ostream&>
operator<<(std::ostream& os, const T& c)
{
	os << c.to_string();
	return os;
}

inline std::ostream&
operator<<(std::ostream& os, const std::vector<move_ex_t>& moves)
{
	bool first {true};
	for (const auto& m : moves)
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
