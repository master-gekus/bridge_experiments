#ifndef TABLE_H
#define TABLE_H

#include <cstdint>

#include <array>
#include <iostream>
#include <limits>
#include <type_traits>
#include <string>

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

card next_card_from_string(const char* &str);
const char* to_string(card c);

enum suit : uint8_t
{
	Clubs = 0,
	Diamonds = 1,
	Hearts = 2,
	Spades = 3,
	NoTrump = std::numeric_limits<std::underlying_type_t<suit>>::max(),
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
	{}

	inline explicit cards(const YAML::Node& n)
		: cards {n.IsNull() ? "" : n.as<std::string>().c_str()}
	{}

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

enum sides : uint8_t
{
	North = 0,
	East = 1,
	South = 2,
	West = 3,
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
	{
	}

public:
	void dump(std::ostream& os = std::cout);

private:
	std::array<hand, 4> hands_;
};

inline std::istream& operator>>(std::istream& is, cards& c)
{
	std::string s;
	is >> s;
	c = cards {s};
	return is;
}

inline std::ostream& operator<<(std::ostream& os, const cards& c)
{
	os << c.to_string();
	return os;
}

#endif // TABLE_H
