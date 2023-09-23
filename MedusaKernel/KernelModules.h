#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

struct KernelModulesVector
{
	ULONG64 Addr;
	ULONG64 Size;
	WCHAR Name[260];
	char Path[260];
	USHORT Check;
};

struct KernelUnloadModules
{
	ULONG64 Addr;
	ULONG64 Size;
	ULONG64 UnLoadTime;
	WCHAR Name[260];
	USHORT Check;
};

class KernelModules
{
public:
	KernelModules() = default;
	~KernelModules() = default;
public:
	void GetKernelModuleListALL(PDRIVER_OBJECT  pdriver);
	bool GetKernelModuleList1();
	std::vector<KernelModulesVector> GetKernelModuleList2(PDRIVER_OBJECT  pdriver);
	std::vector<KernelModulesVector> GetKernelModuleList3(UNICODE_STRING* Directory);
	bool IsAddressInDriversList(ULONG64 Address);
	bool GetUnLoadKernelModuleList(MedusaPDBInfo *_MedusaPDBInfo, PDRIVER_OBJECT);
public:
	std::vector<KernelModulesVector> _KernelModuleList;
	std::vector<KernelUnloadModules> _UnLoadKernelModuleList;
private:
	std::wstring Case_Upper(const std::wstring& str)
	{
		std::wstring result = str;
		std::transform(result.begin(), result.end(), result.begin(), toupper);
		return result;
	}
};



