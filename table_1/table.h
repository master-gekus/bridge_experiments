#ifndef TABLE_H
#define TABLE_H

#include <cstdint>

#include <array>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

#include <yaml-cpp/yaml.h>

enum card : uint16_t
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

card next_card_from_string(const char*& str);
const char* to_string(card c) noexcept;

class suit
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

	inline constexpr suit() noexcept
		: suit_ {0}
	{
	}

	inline constexpr suit(suits s) noexcept
		: suit_ {static_cast<uint8_t>(static_cast<uint8_t>(s) % 4)}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline explicit constexpr suit(T s)
		: suit_ {static_cast<uint8_t>(s % 4)}
	{
	}

	explicit suit(const char* str, bool allow_nt = false);

	inline explicit suit(const YAML::Node& n, bool allow_nt = false)
		: suit {n.as<std::string>().c_str(), allow_nt}
	{
	}

public:
	const char* to_string() const noexcept;
	const char* to_string_short() const noexcept;

private:
	uint8_t suit_;
};

class cards
{
private:
	using underlying_type = std::underlying_type_t<card>;

public:
	inline cards() noexcept
		: cards_ {0}
	{
	}

	inline explicit cards(card c) noexcept
		: cards_ {static_cast<underlying_type>(c)}
	{
	}

	explicit cards(const char* str);

	inline explicit cards(const std::string& str)
		: cards {str.c_str()}
	{
	}

	inline explicit cards(const YAML::Node& n)
		: cards {n.IsNull() ? "" : n.as<std::string>().c_str()}
	{
	}

	~cards() = default;
	cards(const cards&) = default;
	cards(cards&&) = default;
	cards& operator=(const cards&) = default;
	cards& operator=(cards&&) = default;

public:
	std::string to_string() const;

private:
	underlying_type cards_;
};

class side
{
public:
	enum sides : uint8_t
	{
		North = 0,
		East = 1,
		South = 2,
		West = 3,
	};

	inline constexpr side() noexcept
		: side_ {0}
	{
	}

	inline constexpr side(sides v)
		: side_ {static_cast<uint8_t>(static_cast<uint8_t>(v) % 4)}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline constexpr side(T v)
		: side_ {static_cast<uint8_t>(v % 4)}
	{
	}

	template <typename T>
	inline constexpr std::enable_if_t<std::is_integral_v<T>, side>
	operator+(T term) const noexcept
	{
		return side {side_ + term};
	}

	explicit side(const char* str);
	inline explicit side(const YAML::Node& n)
		: side {n.as<std::string>().c_str()}
	{
	}

	~side() = default;
	side(const side&) = default;
	side(side&&) = default;
	side& operator=(const side&) = default;
	side& operator=(side&&) = default;

public:
	const char* to_string() const noexcept;

private:
	uint8_t side_;
};

class hand
{
public:
	hand() = default;
	~hand() = default;

	hand(const hand&) = default;
	hand(hand&&) = default;
	hand& operator=(const hand&) = default;
	hand& operator=(hand&&) = default;

	inline hand(const char* c, const char* d, const char* h, const char* s)
		: suites_ {cards {c}, cards {d}, cards {h}, cards {s}}
	{
	}

	inline explicit hand(const YAML::Node& n)
		: suites_ {cards {n["C"]}, cards {n["D"]}, cards {n["H"]}, cards {n["S"]}}
	{
	}

public:
	void dump(std::ostream& os = std::cout);

private:
	std::array<cards, 4> suites_;
};

class move
{
public:
	move() = default;
	~move() = default;

	move(const move&) = default;
	move(move&&) = default;
	move& operator=(const move&) = default;
	move& operator=(move&&) = default;

	explicit move(const char* str);
	inline explicit move(const YAML::Node& n)
		: move {n.as<std::string>().c_str()}
	{
	}

public:
	inline std::string to_string() const
	{
		return std::string {::to_string(card_)} + suit_.to_string_short();
	}

private:
	card card_;
	suit suit_;
};

class table
{
public:
	table() = default;
	~table() = default;

	table(const table&) = default;
	table(table&&) = default;
	table& operator=(const table&) = default;
	table& operator=(table&&) = default;

	inline explicit table(const YAML::Node& n)
		: hands_ {hand {n["N"]}, hand {n["E"]}, hand {n["S"]}, hand {n["W"]}}
		, trump_ {n["T"], true}
		, turn_starter_ {n["TS"]}
	{
		for (const auto& m : n["M"])
		{
			moves_.emplace_back(m);
		}
	}

public:
	void dump(std::ostream& os = std::cout);

private:
	std::array<hand, 4> hands_;
	suit trump_;
	side turn_starter_;
	std::vector<move> moves_;
};

template <typename T, typename... Types>
using is_one_of = std::disjunction<std::is_same<T, Types>...>;

template <typename T>
inline std::enable_if_t<is_one_of<T, cards, suit, side, move>::value, std::ostream&>
operator<<(std::ostream& os, const T& c)
{
	os << c.to_string();
	return os;
}

#endif // TABLE_H
