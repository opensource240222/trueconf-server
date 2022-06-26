#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace vs {

// Class that stores a sequence of variable-sized memory blocks minimizing amount of memory allocations.
// Blocks can be added only to the end, and removed only from the front.
// Blocks must be non-empty and smaller that min(2^16 - 1, chunk_size - 4) bytes.
class BlockQueue
{
	using block_size_t = uint16_t;
	using chunk_list = std::vector<std::unique_ptr<char[]>>;

public:
	class ConstBlockRef;
	class BlockRef
	{
		friend ConstBlockRef;
		void* p_block;
	public:
		explicit constexpr BlockRef(void* p = nullptr) noexcept : p_block(p) {}

		constexpr char* Data() const noexcept
		{
			return static_cast<char*>(p_block) + sizeof(block_size_t);
		}
		constexpr block_size_t Size() const noexcept
		{
			return *static_cast<const block_size_t*>(p_block);
		}

		// range-for support
		using value_type = char;
		using iterator = char*;
		using const_iterator = char*;
		constexpr const_iterator begin() const noexcept { return Data(); }
		constexpr const_iterator end()   const noexcept { return Data() + Size(); }
	};
	class ConstBlockRef
	{
		const void* p_block;
	public:
		explicit constexpr ConstBlockRef(const void* p = nullptr) noexcept : p_block(p) {}
		// cppcheck-suppress noExplicitConstructor
		constexpr ConstBlockRef(const BlockRef& x)
			: p_block(x.p_block)
		{
		}

		constexpr const char* Data() const noexcept
		{
			return static_cast<const char*>(p_block) + sizeof(block_size_t);
		}
		constexpr block_size_t Size() const noexcept
		{
			return *static_cast<const block_size_t*>(p_block);
		}

		// range-for support
		using value_type = const char;
		using iterator = const char*;
		using const_iterator = const char*;
		constexpr const_iterator begin() const noexcept { return Data(); }
		constexpr const_iterator end()   const noexcept { return Data() + Size(); }
	};

	explicit BlockQueue(size_t chunk_size = 64*1024);
	BlockQueue(BlockQueue&& x);
	BlockQueue& operator=(BlockQueue&& x);

	// Returns view of the first/last block.
	// The queue must not be empty.
	BlockRef Front() noexcept
	{
		return BlockRef(m_chunks.front().get() + m_first_offset);
	}
	ConstBlockRef Front() const noexcept
	{
		return BlockRef(m_chunks.front().get() + m_first_offset);
	}
	BlockRef Back() noexcept
	{
		return BlockRef(m_chunks.back().get() + m_last_offset);
	}
	ConstBlockRef Back() const noexcept
	{
		return ConstBlockRef(m_chunks.back().get() + m_last_offset);
	}

	// Returns maximum size of the block that can be put into the queue.
	size_t GetMaxBlockSize() const;

	// Returns size of the block that can be put into the queue without causing a new allocation.
	size_t GetChunkFreeSpace() const;

	// Changes size of the last block in the queue, size must be greater than 0.
	// The queue must not be empty.
	void ResizeBack(size_t size);

	// Adds new block to the end of the queue, if size == 0 does nothing.
	// If data == nullptr, then new block's data is left uninitialized.
	// Invalidates all iterators.
	void PushBack(const void* data, size_t size);

	// Removes the first block.
	// The queue must not be empty.
	// Invalidates all iterators.
	void PopFront();

	// Returns true if there are no blocks in the queue.
	bool Empty() const noexcept
	{
		return m_chunks.empty();
	}

	// Returns number of blocks in the queue.
	size_t Size() const noexcept
	{
		return m_n_blocks;
	}

	// Removes all blocks from the queue.
	// Invalidates all iterators.
	void Clear();

	// Releases unused memory.
	void ShrinkToFit() noexcept;

	// range-for support
	using value_type = ConstBlockRef;

	class const_iterator;
	class iterator
	{
		friend const_iterator;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = BlockRef;
		using pointer = const BlockRef*;
		using reference = const BlockRef&;
		using iterator_category = std::forward_iterator_tag;

		iterator() = default;
		iterator(void* p, chunk_list::const_iterator ch_it_, chunk_list::const_iterator ch_it_end_)
			: block(p)
			, ch_it(ch_it_)
			, ch_it_end(ch_it_end_)
		{
		}

		reference operator*() const noexcept
		{
			return block;
		}
		pointer operator->() const noexcept
		{
			return &block;
		}

		iterator& operator++()
		{
			BlockRef next(block.Data() + block.Size());
			if (next.Size() != 0)
				block = next;
			else if (++ch_it != ch_it_end)
				block = BlockRef(ch_it->get());
			else
				block = BlockRef(nullptr);
			return *this;
		}
		iterator operator++(int)
		{
			iterator tmp(*this);
			operator++();
			return tmp;
		}

		friend bool operator==(const iterator& l, const iterator& r) noexcept
		{
			return l.block.Data() == r.block.Data();
		}
		friend bool operator!=(const iterator& l, const iterator& r) noexcept
		{
			return l.block.Data() != r.block.Data();
		}

	private:
		BlockRef block;
		chunk_list::const_iterator ch_it;
		chunk_list::const_iterator ch_it_end;
	};
	class const_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = ConstBlockRef;
		using pointer = const ConstBlockRef*;
		using reference = const ConstBlockRef&;
		using iterator_category = std::forward_iterator_tag;

		const_iterator() = default;
		const_iterator(const void* p, chunk_list::const_iterator ch_it_, chunk_list::const_iterator ch_it_end_)
			: block(p)
			, ch_it(ch_it_)
			, ch_it_end(ch_it_end_)
		{
		}
		// cppcheck-suppress noExplicitConstructor
		const_iterator(iterator x)
			: block(x.block)
			, ch_it(x.ch_it)
			, ch_it_end(x.ch_it_end)
		{
		}

		reference operator*() const noexcept
		{
			return block;
		}
		pointer operator->() const noexcept
		{
			return &block;
		}

		const_iterator& operator++()
		{
			ConstBlockRef next(block.Data() + block.Size());
			if (next.Size() != 0)
				block = next;
			else if (++ch_it != ch_it_end)
				block = ConstBlockRef(ch_it->get());
			else
				block = ConstBlockRef(nullptr);
			return *this;
		}
		const_iterator operator++(int)
		{
			const_iterator tmp(*this);
			operator++();
			return tmp;
		}

		friend bool operator==(const const_iterator& l, const const_iterator& r) noexcept
		{
			return l.block.Data() == r.block.Data();
		}
		friend bool operator!=(const const_iterator& l, const const_iterator& r) noexcept
		{
			return l.block.Data() != r.block.Data();
		}

	private:
		ConstBlockRef block;
		chunk_list::const_iterator ch_it;
		chunk_list::const_iterator ch_it_end;
	};

	iterator begin()
	{
		if (m_chunks.empty())
			return iterator();
		else
			return iterator(m_chunks.front().get() + m_first_offset, m_chunks.begin(), m_chunks.end());
	}
	// End iterator is special value identical for all BlockQueue instances.
	// This function can't be made static because we want to overload it for const obejct.
	// cppcheck-suppress functionStatic
	iterator end()
	{
		return iterator();
	}
	const_iterator begin() const
	{
		if (m_chunks.empty())
			return const_iterator();
		else
			return const_iterator(m_chunks.front().get() + m_first_offset, m_chunks.begin(), m_chunks.end());
	}
	// cppcheck-suppress functionStatic
	const_iterator end() const
	{
		return const_iterator();
	}

private:
	void Grow();

	chunk_list m_chunks;
	size_t m_first_offset; // Offset of the first block in the first chunk, only valid if m_chunks.empty() == false
	size_t m_last_offset; // Offset of the last block in the last chunk, only valid if m_chunks.empty() == false
	size_t m_chunk_size;
	size_t m_n_blocks;
	chunk_list m_free_chunks;
};

}
