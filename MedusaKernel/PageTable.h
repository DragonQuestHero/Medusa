#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"

#include "MemoryRW.h"


using PageTableStruct = struct 
{
	DIR_TABLE_BASE cr3;
	ULONG64 pxe_addr;
	ULONG64 ppe_addr;
	ULONG64 pde_addr;
	ULONG64 pte_addr;
	HardwarePte pxe;
	HardwarePte ppe;
	HardwarePte pde;
	HardwarePte pte;
};

class PageTable
{
public:
	PageTable() = default;
	~PageTable() = default;
public:
	bool Init();
	PageTableStruct GetPageTable(ULONG64 PID, ULONG64 addr);
public:
	// Return an address of PXE
	HardwarePte* PageTable::UtilpAddressToPxe(const void* address);
	// Return an address of PPE
	HardwarePte* PageTable::UtilpAddressToPpe(const void* address);
	// Return an address of PDE
	HardwarePte* PageTable::UtilpAddressToPde(const void* address);
	// Return an address of PTE
	HardwarePte* PageTable::UtilpAddressToPte(const void* address);
	// VA -> PA
	ULONG64 PageTable::UtilPaFromVa(void* va);
	// VA -> PFN
	PFN_NUMBER PageTable::UtilPfnFromVa(void* va);
	// PA -> PFN
	PFN_NUMBER PageTable::UtilPfnFromPa(ULONG64 pa);
	// PA -> VA
	void* PageTable::UtilVaFromPa(ULONG64 pa);
	// PNF -> PA
	ULONG64 PageTable::UtilPaFromPfn(PFN_NUMBER pfn);
	// PFN -> VA
	void* PageTable::UtilVaFromPfn(PFN_NUMBER pfn);
private:
	bool _Init = false;
};

