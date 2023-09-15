#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"

#include <vector>

struct KernelModulesVector
{
	ULONG64 Addr;
	ULONG64 Size;
	WCHAR Name[260];
	char Path[260];
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
public:
	std::vector<KernelModulesVector> _KernelModuleList;
private:

};



