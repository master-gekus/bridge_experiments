#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>

#include <type_traits>

/**
 *****************************************************************************
 * @brief The card_t struct - значение карты
 */
struct card_t
{
	using underlying_type = uint16_t;

	enum cards : underlying_type
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

	card_t() = default;
	card_t(const card_t&) = default;
	card_t(card_t&&) = default;
	card_t& operator=(const card_t&) = default;
	card_t& operator=(card_t&&) = default;
	~card_t() = default;

	inline constexpr card_t(cards c) noexcept
		: card_ {static_cast<underlying_type>(c)}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline constexpr card_t(T c) noexcept
		: card_ {static_cast<underlying_type>(c)}
	{
	}

	inline constexpr operator underlying_type() const noexcept
	{
		return card_;
	}

	inline constexpr bool is_neighbors(card_t other) const noexcept
	{
		return ((card_ << 1) == other.card_) || ((other.card_ << 1) == card_);
	}

	static card_t next_card_from_string(const char*& str);

	const char* to_string() const noexcept;

private:
	underlying_type card_;
};

static_assert(std::is_trivial_v<card_t>);
static_assert(sizeof(card_t) == sizeof(uint16_t));
static_assert(alignof(card_t) == alignof(uint16_t));

/**
 *****************************************************************************
 * @brief The suit_t struct - масть
 */
struct suit_t
{
	using underlying_type = uint8_t;

	enum suits : underlying_type
	{
		Clubs = 0,
		Diamonds = 1,
		Hearts = 2,
		Spades = 3,
		NoTrump = 4,
	};

	suit_t() = default;
	suit_t(const suit_t&) = default;
	suit_t(suit_t&&) = default;
	suit_t& operator=(const suit_t&) = default;
	suit_t& operator=(suit_t&&) = default;
	~suit_t() = default;

	inline constexpr suit_t(suits s) noexcept
		: suit_ {(NoTrump == s) ? static_cast<underlying_type>(NoTrump)
								: static_cast<underlying_type>(static_cast<uint8_t>(s) % 4)}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline constexpr suit_t(T s)
		: suit_ {static_cast<underlying_type>(s % 4)}
	{
	}

	explicit suit_t(const char* str, bool allow_nt = false);

	inline constexpr operator underlying_type() const noexcept
	{
		return static_cast<underlying_type>(suit_);
	}

	inline constexpr bool operator==(const suit_t& other) const noexcept
	{
		return suit_ == other.suit_;
	}

	inline suit_t& operator++()
	{
		suit_ = static_cast<underlying_type>((suit_ + 1) % 4);
		return *this;
	}

	const char* to_string() const noexcept;
	const char* to_string_short() const noexcept;

private:
	underlying_type suit_;
};

static_assert(std::is_trivial_v<suit_t>);
static_assert(sizeof(suit_t) == sizeof(uint8_t));
static_assert(alignof(suit_t) == alignof(uint8_t));

/**
 *****************************************************************************
 * @brief The side_t struct - сторона
 */
struct side_t
{
	using underlying_type = uint8_t;

	enum sides : underlying_type
	{
		North = 0,
		East = 1,
		South = 2,
		West = 3,
	};

	side_t() = default;
	side_t(const side_t&) = default;
	side_t(side_t&&) = default;
	side_t& operator=(const side_t&) = default;
	side_t& operator=(side_t&&) = default;
	~side_t() = default;

	inline constexpr side_t(sides v)
		: side_ {static_cast<underlying_type>(static_cast<underlying_type>(v) % 4)}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	inline constexpr side_t(T v)
		: side_ {static_cast<underlying_type>(v % 4)}
	{
	}

	explicit side_t(const char* str);

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

	inline constexpr operator underlying_type() const noexcept
	{
		return static_cast<underlying_type>(side_);
	}

	inline side_t& operator++()
	{
		side_ = static_cast<uint8_t>((side_ + 1) % 4);
		return *this;
	}

	inline side_t operator++(int)
	{
		uint8_t save {side_};
		side_ = static_cast<uint8_t>((side_ + 1) % 4);
		return side_t {save};
	}

	inline constexpr bool is_ns() const noexcept
	{
		return (North == static_cast<sides>(side_)) || (South == static_cast<sides>(side_));
	}

	const char* to_string() const noexcept;
	const char* to_string_short() const noexcept;

private:
	uint8_t side_;
};

static_assert(std::is_trivial_v<side_t>);
static_assert(sizeof(side_t) == sizeof(uint8_t));
static_assert(alignof(side_t) == alignof(uint8_t));

#endif // ENUMS_HPP
