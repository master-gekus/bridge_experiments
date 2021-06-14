#include "table_first.h"

#include <cctype>

#include <iomanip>
#include <stdexcept>

namespace first
{

cards_t::cards_t(const char* str)
	: cards_ {0}
{
	if (nullptr == str)
	{
		throw std::invalid_argument {"nullptr passed into cards::from_string()"};
	}

	if ('-' == (*str))
	{
		return;
	}

	while (*str)
	{
		cards_ |= static_cast<underlying_type>(card_t::next_card_from_string(str));
	}
}

std::string cards_t::to_string() const
{
	if (0 == cards_)
	{
		return "-";
	}

	std::string res;
	for (const auto& c : card_t::all())
	{
		if (0 != (cards_ & c))
		{
			res += c.to_string();
		}
	}

	return res;
};

bool cards_t::append(card_t c) noexcept
{
	auto cu {static_cast<underlying_type>(c)};

	if (0 != (cards_ & cu))
	{
		return false;
	}

	cards_ |= cu;

	return true;
}

std::size_t cards_t::size() const noexcept
{
	std::size_t res {0};
	for (underlying_type c = cards_; 0 != c; c >>= 1)
	{
		if (0 != (c & 1))
		{
			++res;
		}
	}
	return res;
}

void cards_t::get_moves(std::vector<move_ex_t>& res, const suit_t& suit) const
{
	for (const auto& c : card_t::all())
	{
		if (0 != (cards_ & c))
		{
			res.emplace_back(c, suit);
		}
	}
}

std::vector<move_ex_t> hand_t::available_moves(const suit_t& suit) const
{
	std::vector<move_ex_t> res;
	res.reserve(13);

	if ((suit_t::NoTrump != suit) && (!suites_[suit].empty()))
	{
		suites_[suit].get_moves(res, suit);
	}
	else
	{
		for (std::size_t i = 0; i < suites_.size(); ++i)
		{
			suites_[i].get_moves(res, suit_t {i});
		}
	}

	return res;
}

void hand_t::dump(std::ostream& os) const
{
	for (std::size_t i = 0; i < suites_.size(); ++i)
	{
		os << std::setw(12) << suit_t {i} << " : " << suites_[i] << std::endl;
	}
}

move_t::move_t(const char* str)
{
	card_ = card_t::next_card_from_string(str);
	suit_ = suit_t {str};
}

void table_t::dump(std::ostream& os) const
{
	for (std::size_t i = 0; i < hands_.size(); ++i)
	{
		std::cout << "  " << side_t {i} << ":" << std::endl;
		hands_[i].dump(os);
	}

	std::cout << "  Trump           : " << trump_ << std::endl;
	std::cout << "  Turn starter    : " << turn_starter_ << std::endl;

	std::cout << "  Made moves      : ";
	for (const auto& m : moves_)
	{
		std::cout << m << " ";
	}
	std::cout << std::endl;

	if (!is_valid())
	{
		std::cout << "  TABLE IS INVALID!" << std::endl;
	}
	else
	{
		std::cout << "  Next player     : " << turn_starter_ + moves_.size() << std::endl;

		std::cout << "  Available moves : ";
		for (const auto& m : available_moves())
		{
			std::cout << m << " ";
		}
		std::cout << std::endl;
	}
}

bool table_t::is_valid() const noexcept
{
	std::array<hand_t, 4> hands {hands_};

	if (!moves_.empty())
	{
		if (3 < moves_.size())
		{
			return false;
		}

		// "Вернём" сыгравшие карты и проверим, а можно ли было ими играть
		side_t s {turn_starter_};
		suit_t start_suit {moves_.front().suit()};
		for (const auto& m : moves_)
		{
			if ((!hands[s].append(m)) || (!hands[s].is_move_valid(start_suit, m)))
			{
				return false;
			}
			++s;
		}
	}

	const std::size_t sz {hands[0].size()};
	// В принципе, проверка на "не более 13" особо не нужна - карт всего 52, и если будет
	// больше 13, сработает проверка в следующем цикле на пересечения
	if ((13 < sz) || (hands[1].size() != sz) || (hands[2].size() != sz) || (hands[3].size() != sz))
	{
		return false;
	}

	for (std::size_t i = 0; i < 3; ++i)
	{
		for (std::size_t j = i + 1; j < 4; ++j)
		{
			if (hands[i].is_intersect(hands[j]))
			{
				return false;
			}
		}
	}

	return true;
}

void table_t::get_available_moves(moves_type& moves) const
{
	moves.clear();

	for (const auto m : available_moves())
	{
		moves.push_back(move_type {m.card(), m.suit(), m.tricks()});
	}
}

side_t table_t::make_move(const ::move_t& m)
{
	suit_t suit {moves_.empty() ? m.suit() : moves_.front().suit()};
	side_t side {turn_starter_ + moves_.size()};

	if (!hands_[side].is_move_valid(suit, m))
	{
		throw std::logic_error {"Trying to make invalid move."};
	}

	hands_[side].remove(m);
	moves_.push_back(m);

	if (4 == moves_.size())
	{
		std::size_t winer {0};
		for (std::size_t i = 1; i < moves_.size(); ++i)
		{
			if (moves_[i].is_beat(moves_[winer], trump_))
			{
				winer = i;
			}
		}
		turn_starter_ = turn_starter_ + winer;
		moves_.clear();
	}

	return turn_starter_;
}

} // namespace first
