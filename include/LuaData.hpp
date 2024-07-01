#pragma once

#include "BitStream.hpp"
#include <string>
#include <map>

enum DataType : std::uint8_t
{
	DataType_None     = 0,
	DataType_Nil      = 1,
	DataType_Boolean  = 2,
	DataType_Number   = 3,
	DataType_String   = 4,
	DataType_Table    = 5,
	DataType_Int32    = 6,
	DataType_Int16    = 7,
	DataType_Int8     = 8,
	DataType_Json     = 9,
	DataType_Userdata = 100,
	DataType_Unknown  = 101
};

#pragma warning(push)
#pragma warning(disable : 26495)

struct JsonString : public std::string
{
	using std::string::basic_string;
};

struct LuaData
{
	using TableType = std::map<LuaData, LuaData>;
	using JsonType = JsonString;

	LuaData() : m_type(DataType_None) {}

	LuaData(std::string&& str) : m_type(DataType_String), m_string(std::move(str)) {}
	LuaData(const std::string& str) : m_type(DataType_String), m_string(str) {}

	LuaData(const JsonType& json_str) : m_type(DataType_Json), m_string(json_str) {}
	LuaData(JsonType&& json_str) : m_type(DataType_Json), m_string(std::move(json_str)) {}

	template<std::size_t N>
	LuaData(const char(&const_str)[N]) : m_type(DataType_String), m_string(const_str, N - 1) {}

	LuaData(bool boolean) : m_type(DataType_Boolean), m_boolean(boolean) {}

	LuaData(float num) : m_type(DataType_Number), m_number(num) {}

	LuaData(TableType&& tbl) : m_type(DataType_Table), m_table(std::move(tbl)) {}

	LuaData(const TableType& tbl) : m_type(DataType_Table), m_table(tbl) {}

	LuaData(std::int32_t num) : m_type(DataType_Int32), m_int32(num) {}

	LuaData(std::int16_t num) : m_type(DataType_Int16), m_int16(num) {}

	LuaData(std::int8_t num) : m_type(DataType_Int8), m_int8(num) {}

	LuaData(std::nullptr_t) : m_type(DataType_Nil) {}

	LuaData(const LuaData& other)
		: m_type(other.m_type)
	{
		this->copyAssignData(other);
	}

	LuaData(LuaData&& other) noexcept
		: m_type(other.m_type)
	{
		this->moveAssignData(std::move(other));
	}

	~LuaData()
	{
		this->clearData();
	}

	void copyAssignData(const LuaData& other);
	void moveAssignData(LuaData&& other) noexcept;
	void clearData();

	void operator=(LuaData&& other) noexcept;
	void operator=(const LuaData& other) noexcept;
	bool operator<(const LuaData& rhs) const;

	void toString(std::string& out_string) const;
	std::string toString2() const;

	std::size_t getTypeData() const;
	std::size_t getHash() const;

	// Serialization functions

private:
	static bool DeserializeInternal(BitReader& reader, LuaData& out_data);
	static bool DeserializeHeader(BitReader& reader);

	static bool SerializeBody(BitWriter& writer, const LuaData& data);

public:
	static bool Deserialize(const std::string& b64_data, LuaData& out_data);
	static bool Serialize(const LuaData& data, std::string& out_b64_data);

	DataType m_type;

	union {
		std::string m_string;
		bool m_boolean;
		float m_number;
		TableType m_table;
		std::int32_t m_int32;
		std::int16_t m_int16;
		std::int8_t m_int8;

		// Userdata type id
		std::uint32_t m_luaTypeId;
	};
};

#pragma warning(pop)

namespace std
{
	template<>
	struct hash<LuaData>
	{
		inline std::size_t operator()(const LuaData& lua_data) const
		{
			return lua_data.getHash();
		}
	};
}