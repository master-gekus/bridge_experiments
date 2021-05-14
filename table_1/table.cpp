#include "table.h"

#include <cctype>

#include <stdexcept>

card next_card_from_string(const char*& str)
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

const char* to_string(card c)
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

cards::cards(const char* str)
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

std::string cards::to_string() const
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
			res += ::to_string(static_cast<card>(c));
		}
	}

	return res;
};

void hand::dump(std::ostream& os)
{
	os << "    C: " << suites_[0] << std::endl;
	os << "    D: " << suites_[1] << std::endl;
	os << "    H: " << suites_[2] << std::endl;
	os << "    S: " << suites_[3] << std::endl;
}
