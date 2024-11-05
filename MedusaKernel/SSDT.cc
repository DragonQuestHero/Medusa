#include "SSDT.h"


typedef struct _SYSTEM_SERVICE_TABLE2
{
	uint32_t*  ServiceTable;  // array of entry points
	ULONG64  CounterTable;  // array of usage counters
	ULONG64  ServiceLimit;    // number of table entries
	ULONG64    ArgumentTable;  // array of byte counts
}
SYSTEM_SERVICE_TABLE2,
* PSYSTEM_SERVICE_TABLE2,
** PPSYSTEM_SERVICE_TABLE2;
//-----------------------------------------------------------------------------------------------------------
typedef struct _SERVICE_DESCRIPTOR_TABLE
{
	SYSTEM_SERVICE_TABLE2 ntoskrnl;  // ntoskrnl.exe ( native api )
	SYSTEM_SERVICE_TABLE2 win32k;    // win32k.sys (gdi/user support)
}
SYSTEM_DESCRIPTOR_TABLE,
* PSYSTEM_DESCRIPTOR_TABLE,
** PPSYSTEM_DESCRIPTOR_TABLE;


PSYSTEM_SERVICE_TABLE2 GetKeServiceDescriptorTableAddress(int SearchType)
{
	PULONG_PTR KiSystemCall64 = NULL;
	PUCHAR CurrentAddr = NULL;

	KiSystemCall64 = (PULONG_PTR)__readmsr(0xC0000082);
	for (CurrentAddr = (PUCHAR)KiSystemCall64; CurrentAddr <= (PUCHAR)KiSystemCall64 + 1024; ++CurrentAddr)
	{
		switch (SearchType)
		{
		case 0:
			if (0x4c == *CurrentAddr && 0x8d == *(PUCHAR)(CurrentAddr + 1) && 0x15 == *(PUCHAR)(CurrentAddr + 2))
			{
				UHALF_PTR Offset = *(PUHALF_PTR)(CurrentAddr + 3);
				ULONG_PTR Base = (ULONG_PTR)(CurrentAddr + 7);
				return PSYSTEM_SERVICE_TABLE2(Base + Offset);
			}
			break;
		case 1:
			if (0x4c == *CurrentAddr && 0x8d == *(PUCHAR)(CurrentAddr + 1) && 0x1d == *(PUCHAR)(CurrentAddr + 2))
			{
				UHALF_PTR Offset = *(PUHALF_PTR)(CurrentAddr + 3);
				ULONG_PTR Base = (ULONG_PTR)(CurrentAddr + 7);
				return PSYSTEM_SERVICE_TABLE2(Base +Offset);
			}
			break;
		}
		// Windows 11, msr[0xC0000082] = nt!KiSystemCall64Shadow  
		// We need search back  
		//  fffff803`154b43c8 4883c408        add     rsp,8  
		//  fffff803`154b43cc 0faee8          lfence  <-- all of these bytes  
		//  fffff803`154b43cf 65c604255308000000 mov   byte ptr gs:[853h],0 <-- last byte  
		//  fffff803`154b43d8 e93d0a97ff      jmp     nt!KiSystemServiceUser (fffff803`14e24e1a)  Branch <-- first 1 byte  
		if (0x00e8ae0f == (UHALF_PTR)((*(PUHALF_PTR)CurrentAddr) & 0x00FFFFFF))
		{
			PUCHAR j = NULL;
			for (j = (PUCHAR)CurrentAddr; j < (PUCHAR)CurrentAddr + 100; ++j)
			{
				if (0xe9 == *j && 0x00 == *(j - 1))
				{
					UHALF_PTR Offset = *(PUHALF_PTR)(j + 1);
					CurrentAddr = PUCHAR((ULONG64)j + (ULONG64)Offset);
					KiSystemCall64 = (PULONG_PTR)CurrentAddr;
					break;
				}
			}
		}
	}
}

bool SSDT::GetAllSSDT()
{
	if (_KeServiceDescriptorTableShadow)
	{

	}
	else
	{
		PSYSTEM_SERVICE_TABLE2 temp_table = GetKeServiceDescriptorTableAddress(0);
		if (!temp_table || !MmIsAddressValid(temp_table) || 
			!MmIsAddressValid(temp_table->ServiceTable) || !MmIsAddressValid((ULONG64*)temp_table->ArgumentTable))
		{
			return false;
		}
		for (int i = 0; i < temp_table->ServiceLimit; i++)
		{
			uint32_t dwOffset = temp_table->ServiceTable[i];
			uint64_t result;
			// 右移 4 位并处理高位
			if (dwOffset & 0x80000000) {
				// 当高位为 1 时，设置高 32 位为 0xFFFFFFFF
				result = (static_cast<uint64_t>(dwOffset) >> 4) | 0xFFFFFFFFF0000000;
			}
			else {
				// 否则直接右移并扩展为 64 位
				result = static_cast<uint64_t>(dwOffset >> 4);
			}
			

			SSDT_STRUCT temp_ssdt = { 0 };
			temp_ssdt.Index = 0;
			temp_ssdt.Addr = (ULONG64)temp_table->ServiceTable + result;

			_SSDTALL.push_back(temp_ssdt);
		}
	}
	return true;
}

bool SSDT::GetAllShadowSSDT()
{
	if (_KeServiceDescriptorTableShadow)
	{

	}
	else
	{

		PSYSTEM_DESCRIPTOR_TABLE temp_descriptor_table = (PSYSTEM_DESCRIPTOR_TABLE)GetKeServiceDescriptorTableAddress(1);
		PSYSTEM_SERVICE_TABLE2 temp_table = &temp_descriptor_table->win32k;
		if (!temp_table || !MmIsAddressValid(temp_table) || !MmIsAddressValid(temp_table->ServiceTable))
		{
			return false;
		}
		for (int i = 0; i < temp_table->ServiceLimit; i++)
		{
			uint32_t dwOffset = temp_table->ServiceTable[i];
			uint64_t result;
			// 右移 4 位并处理高位
			if (dwOffset & 0x80000000) {
				// 当高位为 1 时，设置高 32 位为 0xFFFFFFFF
				result = (static_cast<uint64_t>(dwOffset) >> 4) | 0xFFFFFFFFF0000000;
			}
			else {
				// 否则直接右移并扩展为 64 位
				result = static_cast<uint64_t>(dwOffset >> 4);
			}


			SSDT_STRUCT temp_ssdt = { 0 };
			temp_ssdt.Index = 0;
			temp_ssdt.Addr = (ULONG64)temp_table->ServiceTable + result;

			_SSSDTALL.push_back(temp_ssdt);
		}
	}
	return true;
}