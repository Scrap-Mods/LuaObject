#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>

class BitReader
{
public:
	BitReader(const void* data_ptr, std::size_t data_size);

	bool isEnoughData(std::size_t bit_count) const;

	bool readBitAtIdx(std::size_t cur_bit) const;
	bool readBit(bool* pBit);

	bool readBits(void* data_ptr, std::size_t bit_count, const bool align_right = false);

	void alignIndex();

	template<typename T, bool t_big_endian = false>
	inline bool readObject(T* pObject)
	{
		if (!readBits(pObject, sizeof(T) * 8))
			return false;

		if constexpr (t_big_endian && sizeof(T) > 1)
		{
			std::uint8_t* v_begin = reinterpret_cast<std::uint8_t*>(pObject);
			std::uint8_t* v_end = v_begin + sizeof(T);

			std::reverse(v_begin, v_end);
		}

		return true;
	}

	const std::uint8_t* m_dataPtr;
	std::size_t m_dataSize; // Size in bits
	std::size_t m_dataIndex; // Current index in the data
};

class BitWriter
{
public:
	BitWriter();
	~BitWriter() = default;

	void writeBits(const void* data_ptr, std::size_t bit_count, const bool align_right = false);

	template<typename T, bool t_big_endian = false>
	inline void writeObject(T obj)
	{
		if constexpr (t_big_endian && sizeof(T) > 1)
		{
			std::uint8_t* v_begin = reinterpret_cast<std::uint8_t*>(&obj);
			std::uint8_t* v_end = v_begin + sizeof(T);

			std::reverse(v_begin, v_end);
		}

		this->writeBits(&obj, sizeof(T) * 8);
	}

	inline void writeBit(bool bit)
	{
		this->writeBits(&bit, 1, true);
	}

	inline void alignIndex()
	{
		const std::size_t v_offset = m_dataIndex & 7;
		if (v_offset == 0)
			return;

		m_dataIndex += (8 - v_offset);
	}

	std::size_t m_dataIndex;
	std::vector<std::uint8_t> m_data;
};