#pragma once

#include <string>
#include <utility>
#include <vector>

namespace BlockStateParseUtil
{
	inline bool ParsePredicate(const std::string& key, std::vector<std::pair<std::string, std::string>>& out)
	{
		if (true == key.empty()) 
			return true;

		size_t pos = 0;
		while (pos < key.size())
		{
			size_t comma = key.find(',', pos);
			size_t end = (comma == std::string::npos) ? key.size() : comma;

			size_t eq = key.find('=', pos);
			if (eq == std::string::npos || eq > end) 
				return false;

			std::string name = key.substr(pos, eq - pos);
			std::string value = key.substr(eq + 1, end - (eq + 1));

			out.emplace_back(name, value);
			pos = end + 1;
		}

		return true;
	}
}