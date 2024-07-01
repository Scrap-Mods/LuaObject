#include "BitStream.hpp"

BitReader::BitReader(const void* data_ptr, std::size_t data_size)
	: m_dataPtr(reinterpret_cast<const std::uint8_t*>(data_ptr)),
	m_dataSize(data_size * 8),
	m_dataIndex(0) {}

bool BitReader::isEnoughData(std::size_t bit_count) const
{
	return (m_dataIndex + bit_count) <= m_dataSize;
}

bool BitReader::readBitAtIdx(std::size_t cur_bit) const
{
	return m_dataPtr[cur_bit >> 3] >> (7 - (cur_bit & 7)) & 1;
}

bool BitReader::readBit(bool* pBit)
{
	if (!this->isEnoughData(1))
		return false;

	*pBit = this->readBitAtIdx(m_dataIndex++);
	return true;
}

bool BitReader::readBits(
	void* data_ptr,
	std::size_t bit_count,
	const bool align_right)
{
	if (bit_count <= 0 || !this->isEnoughData(bit_count))
		return false;

	const std::size_t v_aligned_offset = m_dataIndex & 7;
	if (v_aligned_offset == 0 && (bit_count & 7) == 0)
	{
		std::memcpy(data_ptr, m_dataPtr + (m_dataIndex >> 3), bit_count >> 3);
		m_dataIndex += bit_count;
		return true;
	}

	std::memset(data_ptr, 0, (bit_count + 7) >> 3);

	const std::size_t v_neg_offset = 8 - v_aligned_offset;
	std::uint8_t* pArrData = reinterpret_cast<std::uint8_t*>(data_ptr);
	while (bit_count > 0)
	{
		const std::size_t v_read_byte = m_dataIndex >> 3;

		*pArrData |= m_dataPtr[v_read_byte] << v_aligned_offset;
		if (bit_count > v_neg_offset)
			*pArrData |= m_dataPtr[v_read_byte + 1] >> v_neg_offset;

		if (bit_count >= 8)
		{
			bit_count -= 8;
			m_dataIndex += 8;
			pArrData++;
			continue;
		}

		if (align_right)
			*pArrData >>= (8 - bit_count);

		m_dataIndex += bit_count;
		break;
	}

	return true;
}

void BitReader::alignIndex()
{
	const std::size_t v_offset = m_dataIndex & 7;
	if (v_offset == 0)
		return;

	m_dataIndex += 8 - v_offset;
}

/////////// BIT WRITER ///////////

BitWriter::BitWriter() :
	m_dataIndex(0),
	m_data() {}

void BitWriter::writeBits(
	const void* data_ptr,
	std::size_t bit_count,
	const bool align_right)
{
	if (bit_count == 0)
		return;

	const std::size_t v_new_byte_count = (m_dataIndex + bit_count + 7) >> 3;
	m_data.resize(v_new_byte_count);

	const std::size_t v_aligned_idx = m_dataIndex & 7;

	if (v_aligned_idx == 0 && (bit_count & 7) == 0)
	{
		std::memcpy(m_data.data() + (m_dataIndex >> 3), data_ptr, bit_count >> 3);
		m_dataIndex += bit_count;
		return;
	}


	const std::uint8_t* v_cur_byte = reinterpret_cast<const std::uint8_t*>(data_ptr);
	std::uint8_t v_out_byte;

	while (bit_count > 0)
	{
		v_out_byte = *(v_cur_byte++);

		if (bit_count < 8 && align_right)
			v_out_byte <<= (8 - bit_count);

		const std::size_t v_byte_idx = m_dataIndex >> 3;
		if (v_aligned_idx == 0)
		{
			m_data[v_byte_idx] = v_out_byte;
		}
		else
		{
			m_data[v_byte_idx] |= v_out_byte >> v_aligned_idx;

			const std::size_t v_neg_idx = 8 - v_aligned_idx;
			if (v_neg_idx < 8 && v_neg_idx < bit_count)
				m_data[v_byte_idx + 1] = v_out_byte << v_neg_idx;
		}

		if (bit_count >= 8)
		{
			m_dataIndex += 8;
			bit_count -= 8;
			continue;
		}

		m_dataIndex += bit_count;
		break;
	}
	/*
	void BitStream::WriteBits( const unsigned char* inByteArray, BitSize_t numberOfBitsToWrite, const bool rightAlignedBits )
{
	unsigned char dataByte;
	const unsigned char* inputPtr=inByteArray;

	// Faster to put the while at the top surprisingly enough
	while ( numberOfBitsToWrite > 0 )
		//do
	{
		dataByte = *( inputPtr++ );

		if ( numberOfBitsToWrite < 8 && rightAlignedBits )   // rightAlignedBits means in the case of a partial byte, the bits are aligned from the right (bit 0) rather than the left (as in the normal internal representation)
			dataByte <<= 8 - numberOfBitsToWrite;  // shift left to get the bits on the left, as in our internal representation

		// Writing to a new byte each time
		if ( numberOfBitsUsedMod8 == 0 )
			* ( data + ( numberOfBitsUsed >> 3 ) ) = dataByte;
		else
		{
			// Copy over the new data.
			*( data + ( numberOfBitsUsed >> 3 ) ) |= dataByte >> ( numberOfBitsUsedMod8 ); // First half

			if ( 8 - ( numberOfBitsUsedMod8 ) < 8 && 8 - ( numberOfBitsUsedMod8 ) < numberOfBitsToWrite )   // If we didn't write it all out in the first half (8 - (numberOfBitsUsed%8) is the number we wrote in the first half)
			{
				*( data + ( numberOfBitsUsed >> 3 ) + 1 ) = (unsigned char) ( dataByte << ( 8 - ( numberOfBitsUsedMod8 ) ) ); // Second half (overlaps byte boundary)
			}
		}

		if ( numberOfBitsToWrite >= 8 )
		{
			numberOfBitsUsed += 8;
			numberOfBitsToWrite -= 8;
		}
		else
		{
			numberOfBitsUsed += numberOfBitsToWrite;
			numberOfBitsToWrite=0;
		}
	}
	// } while(numberOfBitsToWrite>0);
}
	*/
}