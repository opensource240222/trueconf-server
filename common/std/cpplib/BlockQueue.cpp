#include "std/cpplib/BlockQueue.h"

#include "std-generic/compat/memory.h"
#include <cassert>
#include <cstring>
#include <limits>

namespace vs {

// Values of m_first_offset and m_last_offset matter only when m_chunks isn't empty
// cppcheck-suppress uninitMemberVar symbolName=BlockQueue::m_first_offset
// cppcheck-suppress uninitMemberVar symbolName=BlockQueue::m_last_offset
BlockQueue::BlockQueue(size_t chunk_size)
	: m_chunk_size(chunk_size)
	, m_n_blocks(0)
{
}

BlockQueue::BlockQueue(BlockQueue&& x)
	: m_chunks(std::move(x.m_chunks))
	, m_first_offset(x.m_first_offset)
	, m_last_offset(x.m_last_offset)
	, m_chunk_size(x.m_chunk_size)
	, m_n_blocks(x.m_n_blocks)
{
	x.m_chunks.clear();
	x.m_n_blocks = 0;
}

BlockQueue& BlockQueue::operator=(BlockQueue&& x)
{
	if (this == &x)
		return *this;

	m_chunks = std::move(x.m_chunks);
	m_first_offset = x.m_first_offset;
	m_last_offset = x.m_last_offset;
	m_chunk_size = x.m_chunk_size;
	m_n_blocks = x.m_n_blocks;

	x.m_chunks.clear();
	x.m_n_blocks = 0;
	return *this;
}

size_t BlockQueue::GetMaxBlockSize() const
{
	const auto result = m_chunk_size - 2 * sizeof(block_size_t);
	if (result <= std::numeric_limits<block_size_t>::max())
		return result;
	else
		return std::numeric_limits<block_size_t>::max();
}

size_t BlockQueue::GetChunkFreeSpace() const
{
	if (m_chunks.empty())
		return 0;

	const auto next_offset = m_last_offset + sizeof(block_size_t) + Back().Size();
	assert(next_offset + sizeof(block_size_t) <= m_chunk_size);
	if (next_offset + 2 * sizeof(block_size_t) > m_chunk_size)
		return 0; // No space even for an extra 0-sized ternimator block.

	return m_chunk_size - next_offset - 2 * sizeof(block_size_t);
}

void BlockQueue::ResizeBack(size_t size)
{
	assert(size <= GetMaxBlockSize());

	assert(size > 0);
	if (size == 0)
		return;

	char* p = m_chunks.back().get() + m_last_offset;
	if (m_last_offset + sizeof(block_size_t) + size + sizeof(block_size_t) > m_chunk_size)
	{
		// Not enough space left in the last chunk
		const auto orig_size = *reinterpret_cast<block_size_t*>(p);
		const char* orig_data = p + sizeof(block_size_t);
		assert(orig_size <= size);

		// Replace last block with 0-sized ternimator block.
		*reinterpret_cast<block_size_t*>(p) = 0;

		Grow();
		m_last_offset = 0;

		p = m_chunks.back().get() + m_last_offset;
		*reinterpret_cast<block_size_t*>(p) = static_cast<block_size_t>(size);
		p += sizeof(block_size_t);
		std::memcpy(p, orig_data, orig_size);
		p += size;
	}
	else
	{
		*reinterpret_cast<block_size_t*>(p) = static_cast<block_size_t>(size);
		p += sizeof(block_size_t) + size;
	}

	// Store 0-sized ternimator block
	*reinterpret_cast<block_size_t*>(p) = 0;
}

void BlockQueue::PushBack(const void* data, size_t size)
{
	assert(size <= GetMaxBlockSize());

	if (size == 0)
		return;

	if (m_chunks.empty())
	{
		Grow();
		m_first_offset = 0;
		m_last_offset = 0;
	}
	else
	{
		// Skip last block
		m_last_offset += sizeof(block_size_t) + Back().Size();
		assert(m_last_offset + sizeof(block_size_t) <= m_chunk_size);
	}

	if (m_last_offset + sizeof(block_size_t) + size + sizeof(block_size_t) > m_chunk_size)
	{
		// Not enough space left in the last chunk
		Grow();
		m_last_offset = 0;
	}

	// Store new block
	char* p = m_chunks.back().get() + m_last_offset;
	*reinterpret_cast<block_size_t*>(p) = static_cast<block_size_t>(size);
	p += sizeof(block_size_t);
	if (data)
		std::memcpy(p, data, size);
	p += size;
	// Store 0-sized ternimator block
	*reinterpret_cast<block_size_t*>(p) = 0;

	++m_n_blocks;
}

void BlockQueue::PopFront()
{
	assert(!m_chunks.empty());
	assert(m_n_blocks > 0);

	m_first_offset += sizeof(block_size_t) + Front().Size();
	assert(m_first_offset + sizeof(block_size_t) <= m_chunk_size);

	if (Front().Size() == 0)
	{
		// No more blocks in the first chuck, switch to the next chunk
		m_free_chunks.push_back(std::move(m_chunks.front()));
		m_chunks.erase(m_chunks.begin());
		m_first_offset = 0;
	}

	--m_n_blocks;
}

void BlockQueue::Clear()
{
	for (auto& chunk : m_chunks)
		m_free_chunks.push_back(std::move(chunk));
	m_chunks.clear();
	m_n_blocks = 0;
}

void BlockQueue::ShrinkToFit() noexcept
{
	m_free_chunks.clear();
}

void BlockQueue::Grow()
{
	if (m_free_chunks.empty())
		m_chunks.push_back(vs::make_unique_default_init<char[]>(m_chunk_size));
	else
	{
		m_chunks.push_back(std::move(m_free_chunks.back()));
		m_free_chunks.pop_back();
	}
}

}
