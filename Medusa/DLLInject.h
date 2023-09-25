#pragma once
#include <windows.h>
#include <fstream>


class DLLInject
{
public:
	DLLInject() = default;
	~DLLInject() = default;
public:
	bool R3CreateThread(ULONG64 PID);
	bool R3APCInject(ULONG64 PID);
	bool R3MapInject(ULONG64 PID);
private:
	std::string Read_ALL(std::string file_name)
	{
		std::string result;
		//n 和 binary 是 ios 类里定义的以二进制方式打开文件用于读入，用 | 或 运算隔开
		std::fstream file(file_name, std::ios::in | std::ios::binary);
		if (file.is_open() == false)
		{
			return "";
		}
		std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		result = str;
		return result;
	}
};

