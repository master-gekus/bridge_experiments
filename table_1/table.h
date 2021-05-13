#ifndef TABLE_H
#define TABLE_H

#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>
#include <string>

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

enum suits : uint8_t
{
	Clubs = 0,
	Diamonds = 1,
	Hearts = 2,
	Spades = 3,
	NoTrump = std::numeric_limits<std::underlying_type_t<suits>>::max(),
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

	~cards() = default;
	cards(const cards&) = default;
	cards(cards&&) = default;
	cards& operator=(const cards&) = default;
	cards& operator=(cards&&) = default;

public:
	static cards from_string(const char* str);
	static inline cards from_string(const std::string& str)
	{
		return from_string(str.c_str());
	}

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

inline std::istream& operator>>(std::istream& is, cards& c)
{
	std::string s;
	is >> s;
	c = cards::from_string(s);
	return is;
}

inline std::ostream& operator<<(std::ostream& os, const cards& c)
{
	os << c.to_string();
	return os;
}

#endif // TABLE_H
