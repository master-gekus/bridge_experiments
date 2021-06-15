#ifndef MOVES_HPP
#define MOVES_HPP

#include <cassert>
#include <cstdint>

#include <string>

#include <type_traits>

#include "enums.hpp"

class move_t
{
public:
	move_t() = default;
	move_t(const move_t&) = default;
	move_t(move_t&&) = default;
	move_t& operator=(const move_t&) = default;
	move_t& operator=(move_t&&) = default;
	~move_t() = default;

	explicit move_t(const char* str)
		: move_t {card_t::next_card_from_string(str), suit_t {str}, 0}
	{
	}

	template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
	move_t(const card_t& c, const suit_t& s, T tr) noexcept
		: total_ {0}
		, tricks_ {static_cast<uint64_t>(tr)}
	{
		cards_[s] = static_cast<uint16_t>(c);
	}

public:
	inline suit_t suit() const
	{
		return cards_[suit_t::Clubs]   ? suit_t::Clubs
			: cards_[suit_t::Diamonds] ? suit_t::Diamonds
			: cards_[suit_t::Hearts]   ? suit_t::Hearts
									   : suit_t::Spades;
	}

	inline card_t card() const
	{
		return cards_[suit_t::Clubs]   ? cards_[suit_t::Clubs]
			: cards_[suit_t::Diamonds] ? cards_[suit_t::Diamonds]
			: cards_[suit_t::Hearts]   ? cards_[suit_t::Hearts]
									   : cards_[suit_t::Spades];
	}

	inline bool operator<(const move_t& other) const noexcept
	{
		return tricks_ < other.tricks_;
	}

	inline uint8_t tricks() const noexcept
	{
		return static_cast<uint8_t>(tricks_);
	}

	inline bool is_beat(const move_t& m, const suit_t& trump) const noexcept
	{
		const auto s {suit()};
		return (m.suit() == s) ? (card() > m.card()) : (trump == s);
	}

	inline bool constexpr is_neighbor(const move_t& other) const noexcept
	{
		return ((total_ << 1) == other.total_) || ((other.total_ << 1) == total_);
	}

	template <typename T>
	inline std::enable_if_t<std::is_integral_v<T>>
	add_tricks(T to_add)
	{
		tricks_ += static_cast<uint64_t>(to_add);
	}

	template <typename T>
	inline std::enable_if_t<std::is_integral_v<T>>
	set_tricks(T new_tricks)
	{
		tricks_ = static_cast<uint64_t>(new_tricks);
	}

	inline std::string to_string() const
	{
		return std::string {card().to_string()} + suit().to_string_short();
	}

private:
	union
	{
		uint64_t total_;
		uint16_t cards_[4];
	};
	uint64_t tricks_;
};

static_assert(std::is_trivial_v<move_t>);
static_assert(sizeof(move_t) == (2 * sizeof(uint64_t)));
static_assert(alignof(move_t) == alignof(uint64_t));

class moves_t
{
public:
	inline std::size_t size() const noexcept
	{
		return size_;
	}

	inline bool empty() const noexcept
	{
		return (0 == size_);
	}

	inline const move_t& operator[](std::size_t index) const noexcept
	{
		return moves_[index];
	}

	inline move_t& operator[](std::size_t index) noexcept
	{
		return moves_[index];
	}

	inline const move_t& front() const noexcept
	{
		return moves_[0];
	}

	inline move_t& front() noexcept
	{
		return moves_[0];
	}

	inline const move_t& back() const noexcept
	{
		assert(0 != size_);
		return moves_[size_ - 1];
	}

	inline move_t& back() noexcept
	{
		assert(0 != size_);
		return moves_[size_ - 1];
	}

	inline void clear() noexcept
	{
		size_ = 0;
	}

	inline void push_back(move_t m) noexcept
	{
		assert(13 > size_);
		moves_[size_++] = m;
	}

public:
	using iterator = move_t*;
	using const_iterator = const move_t*;

	iterator begin() noexcept
	{
		return moves_;
	}

	const_iterator begin() const noexcept
	{
		return moves_;
	}

	iterator end() noexcept
	{
		return moves_ + size_;
	}

	const_iterator end() const noexcept
	{
		return moves_ + size_;
	}

	iterator rbegin() noexcept
	{
		return moves_ + size_ - 1;
	}

	const_iterator rbegin() const noexcept
	{
		return moves_ + size_ - 1;
	}

	iterator rend() noexcept
	{
		return moves_ - 1;
	}

	const_iterator rend() const noexcept
	{
		return moves_ - 1;
	}

private:
	move_t moves_[13];
	std::size_t size_;
};

static_assert(std::is_trivial_v<moves_t>);
static_assert(sizeof(moves_t) == (27 * sizeof(uint64_t))); // 13 * 2 +1
static_assert(alignof(moves_t) == alignof(uint64_t));

#endif // MOVES_HPP
