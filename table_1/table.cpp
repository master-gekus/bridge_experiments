#include "table.h"

#include <cctype>

#include <iomanip>
#include <stdexcept>

card_t next_card_from_string(const char*& str)
{
	switch (std::toupper(*(str++)))
	{
	case '1':
		if ('0' != (*str))
		{
			break;
		}
		++str;
		return C_10;
	case '2':
		return C_2;
	case '3':
		return C_3;
	case '4':
		return C_4;
	case '5':
		return C_5;
	case '6':
		return C_6;
	case '7':
		return C_7;
	case '8':
		return C_8;
	case '9':
		return C_9;
	case 'J':
		return Jack;
	case 'Q':
		return Queen;
	case 'K':
		return King;
	case 'A':
		return Ace;
	default:
		break;
	}
	throw std::invalid_argument {"invalid character in string passed into next_card_from_string()"};
}

const char* to_string(card_t c) noexcept
{
	switch (c)
	{
	case C_2:
		return "2";
	case C_3:
		return "3";
	case C_4:
		return "4";
	case C_5:
		return "5";
	case C_6:
		return "6";
	case C_7:
		return "7";
	case C_8:
		return "8";
	case C_9:
		return "9";
	case C_10:
		return "10";
	case Jack:
		return "J";
	case Queen:
		return "Q";
	case King:
		return "K";
	case Ace:
		return "A";
	default:
		return "";
	}
}

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
		cards_ |= static_cast<underlying_type>(next_card_from_string(str));
	}
}

std::string cards_t::to_string() const
{
	if (0 == cards_)
	{
		return "-";
	}

	std::string res;
	for (auto c = static_cast<underlying_type>(Ace); c; c >>= 1)
	{
		if (0 != (cards_ & c))
		{
			res += ::to_string(static_cast<card_t>(c));
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

side_t::side_t(const char* str)
{
	if (nullptr == str)
	{
		throw std::invalid_argument {"nullptr passed into side::side()"};
	}

	const auto* s {str};
	switch (std::toupper(*(s++)))
	{
	case '\0':
		throw std::invalid_argument {"empty string into side::side()"};
	case 'N':
		side_ = static_cast<uint8_t>(North);
		break;
	case 'E':
		side_ = static_cast<uint8_t>(East);
		break;
	case 'S':
		side_ = static_cast<uint8_t>(South);
		break;
	case 'W':
		side_ = static_cast<uint8_t>(West);
		break;
	default:
		--s;
		break;
	}

	if ('\0' != (*s))
	{
		throw std::invalid_argument {"string \"" + std::string {str} + "\" can not be parsed as valid side"};
	}
}

const char* side_t::to_string() const noexcept
{
	switch (static_cast<sides>(side_))
	{
	case North:
		return "North";
	case East:
		return "East";
	case South:
		return "South";
	case West:
		return "West";
	default:
		return "<invalid>";
	}
}

suit_t::suit_t(const char* str, bool allow_nt)
{
	if (nullptr == str)
	{
		throw std::invalid_argument {"nullptr passed into suit::suit()"};
	}

	const auto* s {str};
	switch (std::toupper(*(s++)))
	{
	case '\0':
		throw std::invalid_argument {"empty string into suit::suit()"};
	case 'C':
		suit_ = static_cast<uint8_t>(Clubs);
		break;
	case 'D':
		suit_ = static_cast<uint8_t>(Diamonds);
		break;
	case 'H':
		suit_ = static_cast<uint8_t>(Hearts);
		break;
	case 'S':
		suit_ = static_cast<uint8_t>(Spades);
		break;
	case 'N':
		if ('T' != std::toupper(*(s++)))
		{
			s -= 2;
			break;
		}
		if (!allow_nt)
		{
			throw std::invalid_argument {"NT suit is not allowed."};
		}
		suit_ = static_cast<uint8_t>(NoTrump);
		break;
	default:
		--s;
		break;
	}

	if ('\0' != (*s))
	{
		throw std::invalid_argument {"string \"" + std::string {str} + "\" can not be parsed as valid suit"};
	}
}

const char* suit_t::to_string() const noexcept
{
	switch (static_cast<suits>(suit_))
	{
	case Clubs:
		return "Clubs";
	case Diamonds:
		return "Diamonds";
	case Hearts:
		return "Hearts";
	case Spades:
		return "Spades";
	case NoTrump:
		return "No Trump";
	default:
		return "<invalid>";
	}
}

const char* suit_t::to_string_short() const noexcept
{
	switch (static_cast<suits>(suit_))
	{
	case Clubs:
		return "C";
	case Diamonds:
		return "D";
	case Hearts:
		return "H";
	case Spades:
		return "S";
	case NoTrump:
		return "NT";
	default:
		return "<invalid>";
	}
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
	card_ = next_card_from_string(str);
	suit_ = suit_t {str};
}

void table_t::dump(std::ostream& os) const
{
	for (std::size_t i = 0; i < hands_.size(); ++i)
	{
		std::cout << "  " << side_t {i} << ":" << std::endl;
		hands_[i].dump(os);
	}

	std::cout << "  Trump        : " << trump_ << std::endl;
	std::cout << "  Turn starter : " << turn_starter_ << std::endl;

	std::cout << "  Made moves   : ";
	for (const auto& m : moves_)
	{
		std::cout << m << " ";
	}
	std::cout << std::endl;

	std::cout << "  Next move    : " << turn_starter_ + moves_.size() << std::endl;
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
	if ((hands[1].size() != sz) || (hands[2].size() != sz) || (hands[3].size() != sz))
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
