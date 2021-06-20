#ifndef TABLE_CACHE_MEMORY_HPP
#define TABLE_CACHE_MEMORY_HPP

#include <cstdint>
#include <cstring>

#include "moves.hpp"
#include "table_hash.hpp"

template<template<typename...> typename MapType>
class table_cache_memory
{
public:
	table_cache_memory() = default;
	~table_cache_memory() = default;

	table_cache_memory(const table_cache_memory&) = delete;
	table_cache_memory(table_cache_memory&&) = delete;
	table_cache_memory& operator=(const table_cache_memory&) = delete;
	table_cache_memory& operator=(table_cache_memory&&) = delete;

public:
	class entry_type
	{
	public:
		entry_type() = default;
		entry_type(const entry_type&) = default;
		entry_type(entry_type&&) = default;
		entry_type& operator=(const entry_type&) = default;
		entry_type& operator=(entry_type&&) = default;
		~entry_type() = default;

		inline entry_type(moves_t* entry, bool reverse, std::size_t max_tricks) noexcept
			: entry_ {entry}
			, max_tricks_ {max_tricks}
			, reverse_ {reverse}
		{
		}

	public:
		inline void update(const moves_t& moves)
		{
			if (nullptr == entry_)
			{
				return;
			}

			if (reverse_)
			{
				for (auto it {moves.rbegin()}; moves.rend() != it; --it)
				{
					entry_->push_back(move_t {it->card(), it->suit(), max_tricks_ - it->tricks()});
				}
			}
			else
			{
				(*entry_) = moves;
			}
		}

	private:
		moves_t* entry_ {nullptr};
		std::size_t max_tricks_ {0};
		bool reverse_ {false};
	};

	static_assert(std::is_trivially_copyable_v<entry_type>);

public:
	inline std::size_t size() const
	{
		return cache_.size();
	}

	template<typename TableType>
	entry_type get_entry(moves_t& moves, const TableType& table)
	{
		moves.clear();

		const auto current_player {table.current_player()};
		const auto max_tricks {table.max_tricks()};

		if ((3 > max_tricks) || (!table.is_first_move()))
		{
			return entry_type {};
		}

		typename TableType::hash_type hash;
		table.get_hash(hash);

		const auto trump {table.trump()};

		auto res {cache_.try_emplace(hash)};
		if (res.second)
		{
			res.first->second.clear();
		}
		moves_t* entry {&(res.first->second.moves_[trump])};

		const bool reverse {current_player.is_ns()};

		if (reverse)
		{
			for (auto it {entry->rbegin()}; entry->rend() != it; --it)
			{
				moves.push_back(move_t {it->card(), it->suit(), max_tricks - it->tricks()});
			}
		}
		else
		{
			moves = (*entry);
		}

		return entry_type {entry, reverse, max_tricks};
	}

private:
	struct moves_block
	{
		moves_t moves_[5];
		inline void clear() noexcept
		{
			std::memset(moves_, 0, sizeof(moves_));
		}
	};

private:
	MapType<table_hash, moves_block> cache_;
};

#endif // TABLE_CACHE_MEMORY_HPP
