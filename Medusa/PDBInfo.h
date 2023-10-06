#pragma once
#include "EzPdb/EzPdb.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

struct NTOSSYMBOL
{
	ULONG64 RVA;
	ULONG64 Addr;
	ULONG64 Size;
	std::string Name;
};

struct SYMBOLSTRUCT
{
	ULONG64 Offset;
	std::string Name;
};

struct MedusaPDBInfo
{
	ULONG64 MiProcessLoaderEntry;
	ULONG64 PiDDBLock;
	ULONG64 PiDDBCacheTable;
	ULONG64 MmUnloadedDrivers;
	ULONG64 MmLastUnloadedDriver;
	ULONG64 KernelHashBucketList;
	ULONG64 HashCacheLock;
	ULONG64 CiEaCacheLookasideList;
	ULONG64 RtlpLookupFunctionEntryForStackWalks;
};

class PDBInfo
{
public:
	PDBInfo()
	{
		RtlZeroMemory(&_Pdb, sizeof(EZPDB));
		RtlZeroMemory(&_MedusaPDBInfo, sizeof(MedusaPDBInfo));
	}
	~PDBInfo() = default;
public:
	bool DownLoadNtos();
	bool DownLoad(std::string, bool);
	bool GetALL();
	void UnLoad();
	std::vector<SYMBOLSTRUCT> PdbGetStruct(IN PEZPDB Pdb, IN std::string StructName);
	bool SendMedusaPDBInfo();
public:
	EZPDB _Pdb;
	std::vector<NTOSSYMBOL> _Symbol;
	ULONG64 _BaseAddr;
	MedusaPDBInfo _MedusaPDBInfo;
	std::string _SymbolServer = "https://msdl.microsoft.com/download/symbols/";
private:
	
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
	std::string W_TO_C(std::wstring str)
	{
		std::string result;
		DWORD strsize = WideCharToMultiByte(CP_ACP, 0, str.data(), -1, NULL, 0, NULL, NULL);
		char* pstr = new char[strsize];
		WideCharToMultiByte(CP_ACP, 0, str.data(), -1, pstr, strsize, NULL, NULL);
		result = pstr;
		return result;
	}
	std::string Case_Upper(const std::string& str)
	{
		std::string result = str;
		std::transform(result.begin(), result.end(), result.begin(), toupper);
		return result;
	}
};



