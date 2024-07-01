#include <iostream>

#include "BitStream.hpp"
#include "LuaData.hpp"

int main()
{
	{
		const LuaData v_lua_object = LuaData::TableType
		{
			{ LuaData(1), LuaData("Some String") },
			{
				LuaData("SomeTable"),
				LuaData::TableType{
					{ LuaData(true), LuaData(1.1234f) },
					{ LuaData("Test"), LuaData(false) }
				}
			}
		};

		std::cout << "Data   : " << v_lua_object.toString2() << std::endl;

		std::string v_b64_str;
		LuaData::Serialize(v_lua_object, v_b64_str);

		std::cout << "B64    : " << v_b64_str << std::endl;

		LuaData v_new_object;
		LuaData::Deserialize(v_b64_str, v_new_object);

		std::cout << "NewData: " << v_new_object.toString2() << std::endl;
	}

	{
		LuaData v_data;
		LuaData::Deserialize("8ARMVUEAAAABBQAAAAGAAAAABAAA", v_data);

		v_data.m_table["JsonData"] = LuaData::JsonType("{ \"1\": [ { \"test\": true } ] }");

		std::string v_b64;
		LuaData::Serialize(v_data, v_b64);

		LuaData v_data2;
		LuaData::Deserialize(v_b64, v_data2);

		std::cout << v_data2.toString2() << std::endl;
	}

	return 0;
}