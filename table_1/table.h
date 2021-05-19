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

enum card_t : uint16_t
{
	C_2 = (1 << 0),
	C_3 = (1 << 1),
	C_4 = (1 << 2),
	C_5 = (1 << 3),
	C_6 = (1 << 4),
	C_7 = (1 << 5),
	C_8 = (1 << 6),
	C_9 = (1 << 7),
	C_10 = (1 << 8),
	Jack = (1 << 9),
	Queen = (1 << 10),
	King = (1 << 11),
	Ace = (1 << 12),
};

card_t next_card_from_string(const char*& str);
const char* to_string(card_t c) noexcept;
inline constexpr bool is_neighbors(card_t a, card_t b)
{
	return ((a << 1) == b) || ((b << 1) == a);
}

class suit_t
{
public:
	enum suits : uint8_t
	{
		Clubs = 0,
		Diamonds = 1,
		Hearts = 2,
		Spades = 3,
		NoTrump = std::numeric_limits<uint8_t>::max(),
	};

	inline constexpr suit_t() noexcept
		: suit_ {0}
	{
	}

	inline constexpr suit_t(suits s) noexcept
		: suit_ {(NoTrump == s) ? static_cast<uint8_t>(NoTrump) : static_cast<uint8_t>(static_cast<uint8_t>(s) % 4)}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline constexpr suit_t(T s)
		: suit_ {static_cast<uint8_t>(s % 4)}
	{
	}

	explicit suit_t(const char* str, bool allow_nt = false);

	inline explicit suit_t(const YAML::Node& n, bool allow_nt = false)
		: suit_t {n.as<std::string>().c_str(), allow_nt}
	{
	}

	inline operator std::size_t() const noexcept
	{
		return static_cast<std::size_t>(suit_);
	}

	inline suit_t& operator++()
	{
		suit_ = static_cast<uint8_t>((suit_ + 1) % 4);
		return *this;
	}

public:
	const char* to_string() const noexcept;
	const char* to_string_short() const noexcept;

private:
	uint8_t suit_;
};

class move_ex_t;

class cards_t
{
private:
	using underlying_type = std::underlying_type_t<card_t>;

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

	std::string to_string() const;
	bool append(card_t c) noexcept;
	std::size_t size() const noexcept;
	void get_moves(std::vector<move_ex_t>& res, const suit_t& suit) const;

private:
	underlying_type cards_;
};

class side_t
{
public:
	enum sides : uint8_t
	{
		North = 0,
		East = 1,
		South = 2,
		West = 3,
	};

	inline constexpr side_t() noexcept
		: side_ {0}
	{
	}

	inline constexpr side_t(sides v)
		: side_ {static_cast<uint8_t>(static_cast<uint8_t>(v) % 4)}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline constexpr side_t(T v)
		: side_ {static_cast<uint8_t>(v % 4)}
	{
	}

	template <typename T>
	inline constexpr std::enable_if_t<std::is_integral_v<T>, side_t>
	operator+(T term) const noexcept
	{
		return side_t {side_ + 4 + (term % 4)};
	}

	template <typename T>
	inline constexpr std::enable_if_t<std::is_integral_v<T>, side_t>
	operator-(T term) const noexcept
	{
		return side_t {side_ + 4 - (term % 4)};
	}

	explicit side_t(const char* str);
	inline explicit side_t(const YAML::Node& n)
		: side_t {n.as<std::string>().c_str()}
	{
	}

	~side_t() = default;
	side_t(const side_t&) = default;
	side_t(side_t&&) = default;
	side_t& operator=(const side_t&) = default;
	side_t& operator=(side_t&&) = default;

	inline operator std::size_t() const noexcept
	{
		return static_cast<std::size_t>(side_);
	}

	inline side_t& operator++()
	{
		side_ = static_cast<uint8_t>((side_ + 1) % 4);
		return *this;
	}

public:
	inline bool is_ns() const noexcept
	{
		return (North == static_cast<sides>(side_)) || (South == static_cast<sides>(side_));
	}

	const char* to_string() const noexcept;
	const char* to_string_short() const noexcept;

private:
	uint8_t side_;
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
		return std::string {::to_string(card_)} + suit_.to_string_short();
	}

	inline bool is_beat(const move_t& m, const suit_t& trump) const noexcept
	{
		return (m.suit_ == suit_) ? (card_ > m.card_) : (trump == suit_);
	}

	inline bool is_neighbor(const move_t& other) const noexcept
	{
		return (suit_ == other.suit_) && is_neighbors(card_, other.card_);
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
	inline int tricks() const
	{
		return static_cast<int>(tricks_);
	}

	inline void add_tricks(uint8_t new_tricks)
	{
		tricks_ += new_tricks;
	}

public:
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

private:
	std::array<cards_t, 4> suites_;
};

class table_t
{
public:
	table_t() = default;
	~table_t() = default;

	table_t(const table_t&) = default;
	table_t(table_t&&) = default;
	table_t& operator=(const table_t&) = default;
	table_t& operator=(table_t&&) = default;

	inline explicit table_t(const YAML::Node& n)
		: hands_ {hand_t {n["N"]}, hand_t {n["E"]}, hand_t {n["S"]}, hand_t {n["W"]}}
		, trump_ {n["T"], true}
		, turn_starter_ {n["TS"]}
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

	std::pair<bool, side_t> make_move(const move_t& m);

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
