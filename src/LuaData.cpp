#include "LuaData.hpp"

#include <iostream>

#include <base64.h>
#include <lz4/lz4.h>

void LuaData::copyAssignData(const LuaData& other)
{
	switch (other.m_type)
	{
	case DataType_Boolean:
		m_boolean = other.m_boolean;
		break;
	case DataType_Number:
		m_number = other.m_number;
		break;
	case DataType_String:
	case DataType_Json:
		new (&m_string) std::string(other.m_string);
		break;
	case DataType_Table:
		new (&m_table) LuaData::TableType(other.m_table);
		break;
	case DataType_Int32:
		m_int32 = other.m_int32;
		break;
	case DataType_Int16:
		m_int16 = other.m_int16;
		break;
	case DataType_Int8:
		m_int8 = other.m_int8;
		break;
	case DataType_Userdata:
		m_luaTypeId = other.m_luaTypeId;
		break;
	default:
		break;
	}
}

void LuaData::moveAssignData(LuaData&& other) noexcept
{
	switch (other.m_type)
	{
	case DataType_Boolean:
		m_boolean = other.m_boolean;
		break;
	case DataType_Number:
		m_number = other.m_number;
		break;
	case DataType_String:
	case DataType_Json:
		new (&m_string) std::string(std::move(other.m_string));
		break;
	case DataType_Table:
		new (&m_table) LuaData::TableType(std::move(other.m_table));
		break;
	case DataType_Int32:
		m_int32 = other.m_int32;
		break;
	case DataType_Int16:
		m_int16 = other.m_int16;
		break;
	case DataType_Int8:
		m_int8 = other.m_int8;
		break;
	case DataType_Userdata:
		m_luaTypeId = other.m_luaTypeId;
		break;
	default:
		break;
	}
}

void LuaData::clearData()
{
	switch (m_type)
	{
	case DataType_String:
	case DataType_Json:
		m_string.~basic_string();
		break;
	case DataType_Table:
		m_table.~map();
		break;
	}
}

void LuaData::operator=(LuaData&& other) noexcept
{
	this->clearData();

	m_type = other.m_type;
	this->moveAssignData(std::move(other));
}

void LuaData::operator=(const LuaData& other) noexcept
{
	this->clearData();

	m_type = other.m_type;
	this->copyAssignData(other);
}

bool LuaData::operator<(const LuaData& rhs) const
{
	return this->getHash() < rhs.getHash();
}

void LuaData::toString(std::string& out_string) const
{
	switch (m_type)
	{
	case DataType_Nil:
		out_string.append("nil", 3);
		break;
	case DataType_Boolean:
		out_string.append(m_boolean ? "true" : "false");
		break;
	case DataType_Number:
		out_string.append(std::to_string(m_number));
		break;
	case DataType_String:
		out_string.append("\"" + m_string + "\"");
		break;
	case DataType_Table:
	{
		bool add_comma = false;
		out_string.append("{ ");

		for (const auto& [v_key, v_value] : m_table)
		{
			if (add_comma) out_string.append(", ");
			add_comma = true;

			std::string v_key_str, v_value_str;
			v_key.toString(v_key_str);
			v_value.toString(v_value_str);

			out_string.append("[", 1);
			out_string.append(v_key_str);
			out_string.append("] = ");
			out_string.append(v_value_str);
		}

		out_string.append(" }");
		break;
	}
	case DataType_Int32:
		out_string.append(std::to_string(m_int32));
		break;
	case DataType_Int16:
		out_string.append(std::to_string(m_int16));
		break;
	case DataType_Int8:
		out_string.append(std::to_string(m_int8));
		break;
	case DataType_Json:
		out_string.append("<Json = \"" + m_string + "\">");
		break;
	default:
		out_string.append("UNKNOWN TYPE " + std::to_string(m_type));
		break;
	}
}

std::string LuaData::toString2() const
{
	std::string v_out_str;
	this->toString(v_out_str);

	return v_out_str;
}

std::size_t LuaData::getTypeData() const
{
	switch (m_type)
	{
	case DataType_Boolean:
		return std::size_t(m_boolean);
	case DataType_Number:
		return std::size_t(*reinterpret_cast<const std::uint32_t*>(&m_number));
	case DataType_String:
	case DataType_Json:
		return m_string.size();
	case DataType_Table:
		return m_table.size();
	case DataType_Int32:
		return std::size_t(m_int32);
	case DataType_Int16:
		return std::size_t(m_int16);
	case DataType_Int8:
		return std::size_t(m_int8);
	case DataType_Userdata:
		return std::size_t(m_luaTypeId);
	default:
		return 0;
	}
}

std::size_t LuaData::getHash() const
{
	switch (m_type)
	{
	case DataType_Boolean:
		return std::hash<bool>{}(m_boolean);
	case DataType_String:
	case DataType_Json:
		return std::hash<std::string>{}(m_string);
	case DataType_Number:
		return std::hash<decltype(m_number)>{}(m_number);
	case DataType_Int32:
		return std::hash<std::int32_t>{}(m_int32);
	case DataType_Int16:
		return std::hash<std::int16_t>{}(m_int16);
	case DataType_Int8:
		return std::hash<std::int8_t>{}(m_int8);
	default:
		return 0;
	}
}

bool LuaData::DeserializeInternal(BitReader& reader, LuaData& out_data)
{
	DataType v_type = DataType_None;
	reader.readObject<DataType>(&v_type);

	switch (v_type)
	{
	case DataType_Nil:
		new (&out_data) LuaData(nullptr);
		break;
	case DataType_Boolean:
	{
		bool v_boolean;
		if (!reader.readBit(&v_boolean)) return false;

		new (&out_data) LuaData(v_boolean);
		break;
	}
	case DataType_Number:
	{
		float v_number;
		if (!reader.readObject<float, true>(&v_number)) return false;

		new (&out_data) LuaData(v_number);
		break;
	}
	case DataType_String:
	{
		std::uint32_t v_string_sz;
		if (!reader.readObject<std::uint32_t, true>(&v_string_sz)) return false;
		reader.alignIndex();

		std::string v_final_str(v_string_sz, ' ');
		if (!reader.readBits(v_final_str.data(), v_final_str.size() * 8)) return false;

		new (&out_data) LuaData(std::move(v_final_str));
		break;
	}
	case DataType_Table:
	{
		std::uint32_t v_arr_item_count;
		if (!reader.readObject<std::uint32_t, true>(&v_arr_item_count)) return false;

		bool v_is_array = false;
		if (!reader.readBit(&v_is_array)) return false;

		std::map<LuaData, LuaData> v_table_def = {};

		if (v_is_array)
		{
			std::uint32_t v_item_offset;
			reader.readObject<std::uint32_t, true>(&v_item_offset);

			for (std::uint32_t a = 0; a < v_arr_item_count; a++)
			{
				LuaData v_tblValue;
				if (!LuaData::DeserializeInternal(reader, v_tblValue)) return false;

				v_table_def.emplace(std::int32_t(a), std::move(v_tblValue));
			}
		}
		else
		{
			for (std::uint32_t a = 0; a < v_arr_item_count; a++)
			{
				LuaData v_tblKey, v_tblValue;

				// Read the key
				if (!LuaData::DeserializeInternal(reader, v_tblKey)) return false;
				// Read the value
				if (!LuaData::DeserializeInternal(reader, v_tblValue)) return false;

				v_table_def.emplace(std::move(v_tblKey), std::move(v_tblValue));
			}
		}

		new (&out_data) LuaData(std::move(v_table_def));
		break;
	}
	case DataType_Int32:
	{
		std::int32_t v_int32;
		if (!reader.readObject<std::int32_t, true>(&v_int32)) return false;

		new (&out_data) LuaData(v_int32);
		break;
	}
	case DataType_Int16:
	{
		std::int16_t v_int16;
		if (!reader.readObject<std::int16_t, true>(&v_int16)) return false;

		new (&out_data) LuaData(v_int16);
		break;
	}
	case DataType_Int8:
	{
		std::int8_t v_int8;
		if (!reader.readObject<std::int8_t, true>(&v_int8)) return false;

		new (&out_data) LuaData(v_int8);
		break;
	}
	case DataType_Json:
	{
		std::uint32_t v_str_sz;
		if (!reader.readObject<std::uint32_t, true>(&v_str_sz)) return false;
		reader.alignIndex();

		LuaData::JsonType v_str(std::size_t(v_str_sz), ' ');
		if (!reader.readBits(v_str.data(), v_str.size() * 8)) return false;

		new (&out_data) LuaData(std::move(v_str));
		break;
	}
	case DataType_Userdata:
		std::cout << "USERDATA TYPE IS NOT IMPLEMENTED\n";
		return false;
	default:
		return false;
	}

	return true;
}

bool LuaData::DeserializeHeader(BitReader& reader)
{
	int v_lua_magic = 0;
	if (!reader.readBits(&v_lua_magic, std::size_t(3 * 8)))
		return false;

	if (v_lua_magic != int('AUL'))
	{
		std::cout << "Invalid lua secret\n";
		return false;
	}

	std::uint32_t v_version;
	if (!reader.readObject<std::uint32_t, true>(&v_version))
		return false;

	if (v_version != 1)
	{
		std::cout << "Invalid object version\n";
		return false;
	}

	return true;
}

bool LuaData::SerializeBody(BitWriter& writer, const LuaData& data)
{
	writer.writeObject<DataType>(data.m_type);

	switch (data.m_type)
	{
	case DataType_Nil:
		break;
	case DataType_Boolean:
		writer.writeBit(data.m_boolean);
		break;
	case DataType_Number:
		writer.writeObject<float, true>(data.m_number);
		break;
	case DataType_String:
	{
		writer.writeObject<std::uint32_t, true>(std::uint32_t(data.m_string.size()));
		writer.alignIndex();

		writer.writeBits(data.m_string.data(), data.m_string.size() * 8);
		break;
	}
	case DataType_Table:
	{
		writer.writeObject<std::uint32_t, true>(std::uint32_t(data.m_table.size()));
		// Will currently serialize tables only
		writer.writeBit(0);

		for (const auto& [v_key, v_value] : data.m_table)
		{
			if (!LuaData::SerializeBody(writer, v_key)) return false;
			if (!LuaData::SerializeBody(writer, v_value)) return false;
		}

		break;
	}
	case DataType_Int32:
		writer.writeObject<std::int32_t, true>(data.m_int32);
		break;
	case DataType_Int16:
		writer.writeObject<std::int16_t, true>(data.m_int16);
		break;
	case DataType_Int8:
		writer.writeObject<std::int8_t, true>(data.m_int8);
		break;
	case DataType_Json:
	{
		writer.writeObject<std::uint32_t, true>(std::uint32_t(data.m_string.size()));
		writer.alignIndex();

		writer.writeBits(data.m_string.data(), data.m_string.size() * 8);
		break;
	}
	case DataType_Userdata:
		return false;
	default:
		return false;
	}

	return true;
}

bool LuaData::Deserialize(const std::string& b64_data, LuaData& out_data)
{
	static char v_decompressed_data[0x8000];

	const std::string v_decoded_data = base64_decode(b64_data, false);
	const int v_decomp_sz = LZ4_decompress_safe(
		reinterpret_cast<const char*>(v_decoded_data.data()),
		v_decompressed_data,
		int(v_decoded_data.size()),
		sizeof(v_decompressed_data));

	if (v_decomp_sz <= 0)
	{
		std::cout << "Failed to decompress the data\n";
		return false;
	}

	BitReader v_stream(v_decompressed_data, std::size_t(v_decomp_sz));
	if (!LuaData::DeserializeHeader(v_stream))
		return false;

	return LuaData::DeserializeInternal(v_stream, out_data);
}

bool LuaData::Serialize(const LuaData& data, std::string& out_b64_data)
{
	BitWriter v_writer;

	// Write the secret
	const char v_secret[] = { 'L', 'U', 'A' };
	v_writer.writeBits(v_secret, sizeof(v_secret) * 8);
	// Write version
	v_writer.writeObject<std::uint32_t, true>(1);
	// Write the actual data
	if (!LuaData::SerializeBody(v_writer, data))
		return false;

	// Do the conversion here
	static char v_compressed_data[0x8000];

	const int v_compressed_sz = LZ4_compress_default(
		reinterpret_cast<const char*>(v_writer.m_data.data()),
		v_compressed_data,
		int(v_writer.m_data.size()),
		sizeof(v_compressed_data));

	if (v_compressed_sz <= 0)
		return false;

	out_b64_data = base64_encode(
		reinterpret_cast<std::uint8_t*>(v_compressed_data),
		std::size_t(v_compressed_sz),
		false);

	return true;
}