#pragma once
#include "EzPdb/EzPdb.h"


class PDBInfo
{
public:
	PDBInfo() = default;
	~PDBInfo() = default;
public:
	bool DownLoadNtos();
public:
	EZPDB _Pdb;
private:
	std::vector<std::string> Split(const std::string& p,
		const std::string& regex)
	{
		std::vector<std::string> resVec;

		if ("" == p)
		{
			return resVec;
		}
		std::string strs = p + regex;
		size_t pos = strs.find(regex);
		size_t size = strs.size();
		while (pos != std::string::npos)
		{
			std::string x = strs.substr(0, pos);
			resVec.push_back(x);
			strs = strs.substr(pos + regex.length(), size);
			pos = strs.find(regex);
		}
		return resVec;
	}
	std::string Replace(std::string& str,
		const std::string& old_value, const std::string& new_value)
	{
		for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
			if ((pos = str.find(old_value, pos)) != std::string::npos)
			{
				str.replace(pos, old_value.length(), new_value);
			}
			else
			{
				break;
			}
		}
		return str;
	}

	std::string Replace_ALL(std::string& str,
		const std::string& old_value, const std::string& new_value)
	{
		while (true) {
			std::string::size_type pos(0);
			if ((pos = str.find(old_value)) != std::string::npos)
			{
				str.replace(pos, old_value.length(), new_value);
			}
			else
			{
				break;
			}
		}
		return str;
	}
};



