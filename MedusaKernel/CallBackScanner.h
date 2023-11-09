#pragma once
#include <ntifs.h>
#include <intrin.h>

#include <vector>

#include "CRT/NtSysAPI_Func.hpp"


struct ObjectCallBackList
{
	ULONG64 PreOperation;
	ULONG64 PostOperation;
	ULONG64 CallbackEntry;
	ULONG64 Type;
	WCHAR Name[260];
};



class CallBackScanner
{
public:
	CallBackScanner()
	{
		RTL_OSVERSIONINFOW VersionInfo = { 0 };
		VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
		NTSTATUS status = RtlGetVersion(&VersionInfo);
		if (NT_SUCCESS(status))
		{
			if (VersionInfo.dwBuildNumber >= 9200)
				_CallbackListOffset = 0xC8;
			else if (VersionInfo.dwBuildNumber >= 7600)
				_CallbackListOffset = 0xC0;

			_LoadImageNotifyRoutine = FindPspLoadImageNotifyRoutine();
			_ProcessNotifyRoutine = FindPspCreateProcessNotifyRoutine();
			_ThreadNotifyRoutine = FindPspCreateThreadNotifyRoutine();
		}
	}
	~CallBackScanner() = default;
public:
	std::vector<ObjectCallBackList> GetALLCallBackList();
	std::vector<ObjectCallBackList> GetObjectCallBackList(POBJECT_TYPE pObject);
	std::vector<ObjectCallBackList> GetNotifyRoutineList(ULONG64 Type);
public:
	std::vector<ObjectCallBackList> _ObjectCallBackList;
private:
	ULONG64 _CallbackListOffset = 0;
	ULONG64 _LoadImageNotifyRoutine = 0;
	ULONG64 _ProcessNotifyRoutine = 0;
	ULONG64 _ThreadNotifyRoutine = 0;

	bool pattern_check(const char* data, const char* pattern, const char* mask)
	{
		size_t len = strlen(mask);

		for (size_t i = 0; i < len; i++)
		{
			if (data[i] == pattern[i] || mask[i] == '?')
				continue;
			else
				return false;
		}

		return true;
	}
	unsigned long long find_pattern(unsigned long long addr, unsigned long size, const char* pattern, const char* mask)
	{
		size -= (unsigned long)strlen(mask);

		for (unsigned long i = 0; i < size; i++)
		{
			if (pattern_check((const char*)addr + i, pattern, mask))
				return addr + i;
		}

		return 0;
	}
	UINT64 FindPspLoadImageNotifyRoutine()
	{
		UNICODE_STRING usFunc = { 0 };
		RtlInitUnicodeString(&usFunc, L"PsSetLoadImageNotifyRoutine");
		UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);

		if (!Function)
			return 0;



		char Pattern[3] = { 0x48,0x8D,0x0D };
		Function = find_pattern(Function, 0xFF, Pattern, "xxx");
		//Function = FindPattern(Function, 0xFF, Pattern, sizeof(Pattern));
		if (Function)
		{
			LONG Offset = 0;
			memcpy(&Offset, (UCHAR*)(Function + 3), 4);
			Function = Function + 7 + Offset;
		}

		return Function;
	}
	UINT64 FindPspCreateProcessNotifyRoutine()
	{
		UNICODE_STRING usFunc = { 0 };
		RtlInitUnicodeString(&usFunc, L"PsSetCreateProcessNotifyRoutineEx");
		UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);

		if (!Function)
			return 0;

		char Pattern1[1] = { 0xE8 };
		Function = find_pattern(Function, 20, Pattern1, "x");
		//Function = FindPattern(Function, 20, Pattern1, sizeof(Pattern1));
		if (Function)
		{
			LONG Offset = 0;
			memcpy(&Offset, (UCHAR*)(Function + 1), 4);
			Function = Function + 5 + Offset + 9;
		}

		char Pattern2[3] = { 0x4C,0x8D,0x2D };
		Function = find_pattern(Function, 0xFF, Pattern2, "xxx");
		//Function = FindPattern(Function, 0xFF, Pattern2, sizeof(Pattern2));
		if (Function)
		{
			LONG Offset = 0;
			memcpy(&Offset, (UCHAR*)(Function + 3), 4);
			UINT64 Addy = Function + 7 + Offset;
			return Addy;
		}

		return 0;
	}
	UINT64 FindPspCreateThreadNotifyRoutine()
	{
		UNICODE_STRING usFunc = { 0 };
		RtlInitUnicodeString(&usFunc, L"PsSetCreateThreadNotifyRoutine");
		UINT64 Function = (UINT64)MmGetSystemRoutineAddress(&usFunc);

		if (!Function)
			return 0;

		char Pattern1[3] = { 0x33,0xD2,0xE8 };
		Function = find_pattern(Function, 20, Pattern1, "xxx");
		//Function = FindPattern(Function, 20, Pattern1, sizeof(Pattern1));
		if (Function)
		{
			LONG Offset = 0;
			memcpy(&Offset, (UCHAR*)(Function + 3), 4);
			Function = Function + 5 + Offset;
		}

		char Pattern2[3] = { 0x48,0x8D,0x0D };
		Function = find_pattern(Function, 0xFF, Pattern2, "xxx");
		//Function = FindPattern(Function, 0xFF, Pattern2, sizeof(Pattern2));
		if (Function)
		{
			LONG Offset = 0;
			memcpy(&Offset, (UCHAR*)(Function + 3), 4);
			UINT64 Addy = Function + 7 + Offset;
			return Addy;
		}

		return 0;
	}
};

