#pragma once
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <rapidjson\stringbuffer.h>

class CRapidJsonParsorWrapper
{
public:
	static bool ReadFloat(const rapidjson::Value& value, float& fOut)
	{
		if (value.IsNumber())
			return false;
		fOut = value.GetFloat();
		return true;
	}

	static bool ReadVector3(const rapidjson::Value& arr, float fOut[3])
	{
		if (!arr.IsArray() || arr.Size() < 3) return false;
		for (rapidjson::SizeType i = 0; i < 3; ++i)
		{
			if (!arr[i].IsNumber()) return false;
			fOut[i] = arr[i].GetFloat();
		}
		return true;
	}

	static bool ReadUV4(const rapidjson::Value& arr, float fOut[4])
	{
		if (!arr.IsArray() || arr.Size() < 4) return false;
		for (rapidjson::SizeType i = 0; i < 4; ++i)
		{
			if (!arr[i].IsNumber()) return false;
			fOut[i] = arr[i].GetFloat();
		}
		return true;
	}
};