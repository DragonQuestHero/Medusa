#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"

#include <vector>


struct UserModule
{
	ULONG64 Addr;
	ULONG64 Size;
	WCHAR Name[260];
	WCHAR Path[260];
};

class Modules
{
public:
	Modules() = default;
	~Modules() = default;
public:
	std::vector<UserModule> GetWin32MoudleList(ULONG64 PID);
	bool R0MapInject(ULONG64 PID,ULONG64 Size,void *DLLImage);
	UserModule GetNtdll(ULONG64 PID);
	UserModule GetModuleInfoFromAddr(ULONG64 PID,ULONG64 Addr);
private:
	BOOLEAN CompareWcharStrings(PWCHAR str1, PWCHAR str2)
	{
		UNICODE_STRING ustr1, ustr2;

		ustr1.Buffer = str1;
		ustr1.Length = (USHORT)(wcslen(str1) * sizeof(WCHAR));
		ustr1.MaximumLength = ustr1.Length;

		ustr2.Buffer = str2;
		ustr2.Length = (USHORT)(wcslen(str2) * sizeof(WCHAR));
		ustr2.MaximumLength = ustr2.Length;

		LONG cmpResult = RtlCompareUnicodeString(&ustr1, &ustr2, 1);
		return (cmpResult == 0);
	}
	BOOLEAN SimpleCompareWchar(PWCHAR str1, PWCHAR str2, ULONG length)
	{
		for (ULONG i = 0; i < length; i++)
		{
			if (str1[i] != str2[i])
				return FALSE;
		}
		return TRUE;
	}
	BOOLEAN SimpleCompareWchar(PWCHAR str1, PWCHAR str2)
	{
		for (ULONG i = 0; i < wcslen(str2); i++)
		{
			if (str1[i] != str2[i])
				return FALSE;
		}
		return TRUE;
	}
};

