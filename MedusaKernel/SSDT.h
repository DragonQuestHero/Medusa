#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"

#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>



struct SSDT_STRUCT
{
	ULONG64 Index;
	ULONG64 Addr;
};

class SSDT
{
public:
	SSDT() = default;
	~SSDT() = default;
public:
	ULONG64 _KeServiceDescriptorTableShadow = 0;
	std::vector<SSDT_STRUCT> _SSDTALL;
	std::vector<SSDT_STRUCT> _SSSDTALL;
public:
	bool GetAllSSDT();
	bool GetAllShadowSSDT();
private:

};

