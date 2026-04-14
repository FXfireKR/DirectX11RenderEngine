#pragma once

class IFileStreamWrapper
{
public:
	static bool ReadAllStream(const string& strPath
		, stringstream& outBuffer
		, ios_base::openmode mode = std::ios::binary) 
	{
		outBuffer.clear();

		ifstream file(strPath.c_str(), mode);
		if (true == file.is_open()) 
		{
			outBuffer << file.rdbuf();
			file.close();
			return true;
		}
		return false;
	}
};