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
	ULONG64 DriverObject;
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

struct IOCTLS
{
	ULONG64 Index;
	ULONG64 Addr;
	bool Check;
};

class KernelModules
{
public:
	KernelModules() = default;
	~KernelModules() = default;
public:
	void Init(DRIVER_OBJECT* pdriver)
	{
		_Driver_Object = pdriver;
	}
public:
	void GetKernelModuleListALL();
	bool GetKernelModuleList1();
	std::vector<KernelModulesVector> GetKernelModuleList2();
	std::vector<KernelModulesVector> GetKernelModuleList2Quick();
	std::vector<KernelModulesVector> GetKernelModuleList3(UNICODE_STRING* Directory);
	std::vector<KernelModulesVector> GetKernelModuleList4();
	std::vector<KernelModulesVector> GetKernelModuleList4Quick();
	bool IsAddressInAnyDriversList(ULONG64 Address);
	bool IsAddressInDriversList(PDRIVER_OBJECT DriverObjectAddress, ULONG64 Address);
	bool GetUnLoadKernelModuleList();
	ULONG64 DumpDriver(ULONG64 Address,void*);
	std::vector<IOCTLS> GetIOCTLFunctionR0(ULONG64 Addr);
public:
	std::vector<KernelModulesVector> _KernelModuleList;
	std::vector<KernelUnloadModules> _UnLoadKernelModuleList;
private:
	DRIVER_OBJECT* _Driver_Object = nullptr;
	std::wstring Case_Upper(const std::wstring& str)
	{
		std::wstring result = str;
		std::transform(result.begin(), result.end(), result.begin(), toupper);
		return result;
	}
};



