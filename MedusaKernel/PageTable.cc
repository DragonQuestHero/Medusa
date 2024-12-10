#include "PageTable.h"


// Base addresses of page structures. Use !pte to obtain them.
static auto kUtilpPxeBase = 0xfffff6fb7dbed000ull;
static auto kUtilpPpeBase = 0xfffff6fb7da00000ull;
static auto kUtilpPdeBase = 0xfffff6fb40000000ull;
static auto kUtilpPteBase = 0xfffff68000000000ull;

// Get the highest 25 bits
static const auto kUtilpPxiShift = 39ull;

// Get the highest 34 bits
static const auto kUtilpPpiShift = 30ull;

// Get the highest 43 bits
static const auto kUtilpPdiShift = 21ull;

// Get the highest 52 bits
static const auto kUtilpPtiShift = 12ull;

// Use  9 bits; 0b0000_0000_0000_0000_0000_0000_0001_1111_1111
static const auto kUtilpPxiMask = 0x1ffull;

// Use 18 bits; 0b0000_0000_0000_0000_0011_1111_1111_1111_1111
static const auto kUtilpPpiMask = 0x3ffffull;

// Use 27 bits; 0b0000_0000_0111_1111_1111_1111_1111_1111_1111
static const auto kUtilpPdiMask = 0x7ffffffull;

// Use 36 bits; 0b1111_1111_1111_1111_1111_1111_1111_1111_1111
static const auto kUtilpPtiMask = 0xfffffffffull;


// Base addresses of page structures. Use !pte to obtain them.
static const auto kUtilpPdeBasePae = 0xc0600000;
static const auto kUtilpPteBasePae = 0xc0000000;

// Get the highest 11 bits
static const auto kUtilpPdiShiftPae = 21;

// Get the highest 20 bits
static const auto kUtilpPtiShiftPae = 12;

// Use 11 bits; 0b0000_0000_0000_0000_0000_0000_0111_1111_1111
static const auto kUtilpPdiMaskPae = 0x7ff;

// Use 20 bits; 0b0000_0000_0000_0000_1111_1111_1111_1111_1111
static const auto kUtilpPtiMaskPae = 0xfffff;

static ULONG_PTR g_utilp_pxe_base = 0;
static ULONG_PTR g_utilp_ppe_base = 0;
static ULONG_PTR g_utilp_pde_base = 0;
static ULONG_PTR g_utilp_pte_base = 0;

static ULONG_PTR g_utilp_pxi_shift = 0;
static ULONG_PTR g_utilp_ppi_shift = 0;
static ULONG_PTR g_utilp_pdi_shift = 0;
static ULONG_PTR g_utilp_pti_shift = 0;

static ULONG_PTR g_utilp_pxi_mask = 0;
static ULONG_PTR g_utilp_ppi_mask = 0;
static ULONG_PTR g_utilp_pdi_mask = 0;
static ULONG_PTR g_utilp_pti_mask = 0;

void* UtilMemMem(const void* search_base,
	SIZE_T search_size, const void* pattern,
	SIZE_T pattern_size) {
	if (pattern_size > search_size) {
		return nullptr;
	}
	auto base = static_cast<const char*>(search_base);
	for (SIZE_T i = 0; i <= search_size - pattern_size; i++) {
		if (RtlCompareMemory(pattern, &base[i], pattern_size) == pattern_size) {
			return const_cast<char*>(&base[i]);
		}
	}
	return nullptr;
}

bool PageTable::Init()
{
	RTL_OSVERSIONINFOW os_version;
	os_version.dwOSVersionInfoSize = sizeof(os_version);
	auto status = RtlGetVersion(&os_version);
	if (!NT_SUCCESS(status)) {
		return false;
	}

	// Win 10 build 14316 is the first version implements randomized page tables
	// Use fixed values if a systems is either: x86, older than Windows 7, or
	// older than build 14316.
	if (os_version.dwMajorVersion < 10 ||
		os_version.dwBuildNumber < 14316) 
	{
		g_utilp_pxe_base = kUtilpPxeBase;
		g_utilp_ppe_base = kUtilpPpeBase;
		g_utilp_pxi_shift = kUtilpPxiShift;
		g_utilp_ppi_shift = kUtilpPpiShift;
		g_utilp_pxi_mask = kUtilpPxiMask;
		g_utilp_ppi_mask = kUtilpPpiMask;
		return false;
	}


	static const UCHAR kPatternWin10x64[] = {
		0x48, 0x8b, 0x04, 0xd0,  // mov     rax, [rax+rdx*8]
		0x48, 0xc1, 0xe0, 0x19,  // shl     rax, 19h
		0x48, 0xba,              // mov     rdx, ????????`????????  ; PTE_BASE
	};
	auto found = reinterpret_cast<ULONG_PTR>(
		UtilMemMem(MmGetVirtualForPhysical, 0x30, kPatternWin10x64,
			sizeof(kPatternWin10x64)));
	if (!found) 
	{
		return false;
	}

	found += sizeof(kPatternWin10x64);

	const auto pte_base = *reinterpret_cast<ULONG_PTR*>(found);
	const auto index = (pte_base >> kUtilpPxiShift) & kUtilpPxiMask;
	const auto pde_base = pte_base | (index << kUtilpPpiShift);
	const auto ppe_base = pde_base | (index << kUtilpPdiShift);
	const auto pxe_base = ppe_base | (index << kUtilpPtiShift);

	g_utilp_pxe_base = static_cast<ULONG_PTR>(pxe_base);
	g_utilp_ppe_base = static_cast<ULONG_PTR>(ppe_base);
	g_utilp_pde_base = static_cast<ULONG_PTR>(pde_base);
	g_utilp_pte_base = static_cast<ULONG_PTR>(pte_base);

	g_utilp_pxi_shift = kUtilpPxiShift;
	g_utilp_ppi_shift = kUtilpPpiShift;
	g_utilp_pdi_shift = kUtilpPdiShift;
	g_utilp_pti_shift = kUtilpPtiShift;

	g_utilp_pxi_mask = kUtilpPxiMask;
	g_utilp_ppi_mask = kUtilpPpiMask;
	g_utilp_pdi_mask = kUtilpPdiMask;
	g_utilp_pti_mask = kUtilpPtiMask;
	return true;
}

HardwarePte* PageTable::UtilpAddressToPxe(
	const void* address) {
	const auto addr = reinterpret_cast<ULONG_PTR>(address);
	const auto pxe_index = (addr >> g_utilp_pxi_shift) & g_utilp_pxi_mask;
	const auto offset = pxe_index * sizeof(HardwarePte);
	return reinterpret_cast<HardwarePte*>(g_utilp_pxe_base + offset);
}

// Return an address of PPE
HardwarePte* PageTable::UtilpAddressToPpe(
	const void* address) {
	const auto addr = reinterpret_cast<ULONG_PTR>(address);
	const auto ppe_index = (addr >> g_utilp_ppi_shift) & g_utilp_ppi_mask;
	const auto offset = ppe_index * sizeof(HardwarePte);
	return reinterpret_cast<HardwarePte*>(g_utilp_ppe_base + offset);
}

// Return an address of PDE
HardwarePte* PageTable::UtilpAddressToPde(
	const void* address) {
	const auto addr = reinterpret_cast<ULONG_PTR>(address);
	const auto pde_index = (addr >> g_utilp_pdi_shift) & g_utilp_pdi_mask;
	const auto offset = pde_index * sizeof(HardwarePte);
	return reinterpret_cast<HardwarePte*>(g_utilp_pde_base + offset);
}

// Return an address of PTE
HardwarePte* PageTable::UtilpAddressToPte(
	const void* address) {
	const auto addr = reinterpret_cast<ULONG_PTR>(address);
	const auto pte_index = (addr >> g_utilp_pti_shift) & g_utilp_pti_mask;
	const auto offset = pte_index * sizeof(HardwarePte);
	return reinterpret_cast<HardwarePte*>(g_utilp_pte_base + offset);
}

// VA -> PA
ULONG64 PageTable::UtilPaFromVa(void* va) {
	const auto pa = MmGetPhysicalAddress(va);
	return pa.QuadPart;
}

// VA -> PFN
PFN_NUMBER PageTable::UtilPfnFromVa(void* va) {
	return UtilPfnFromPa(UtilPaFromVa(va));
}

// PA -> PFN
PFN_NUMBER PageTable::UtilPfnFromPa(ULONG64 pa) {
	return static_cast<PFN_NUMBER>(pa >> PAGE_SHIFT);
}

// PA -> VA
void* PageTable::UtilVaFromPa(ULONG64 pa) {
	PHYSICAL_ADDRESS pa2 = {};
	pa2.QuadPart = pa;
	return MmGetVirtualForPhysical(pa2);
}

// PNF -> PA
ULONG64 PageTable::UtilPaFromPfn(PFN_NUMBER pfn) {
	return static_cast<ULONG64>(pfn) << PAGE_SHIFT;
}

// PFN -> VA
void* PageTable::UtilVaFromPfn(PFN_NUMBER pfn) {
	return UtilVaFromPa(UtilPaFromPfn(pfn));
}


PageTableStruct PageTable::GetPageTable(ULONG64 PID, ULONG64 addr)
{
	PageTableStruct temp_PageTableStruct = { 0 };
	PEPROCESS   tempep = NULL;
	KAPC_STATE   kapc = { 0 };
	KIRQL        tempirql = 0;
	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)PID, &tempep);
	if (!NT_SUCCESS(status))
	{
		return temp_PageTableStruct;
	}
	ObDereferenceObject(tempep);
	KeStackAttachProcess(tempep, &kapc);

	temp_PageTableStruct.cr3.BitAddress = __readcr3();
	temp_PageTableStruct.pxe_addr = (ULONG64)UtilpAddressToPxe((void*)addr);
	temp_PageTableStruct.ppe_addr = (ULONG64)UtilpAddressToPpe((void*)addr);
	temp_PageTableStruct.pde_addr = (ULONG64)UtilpAddressToPde((void*)addr);
	temp_PageTableStruct.pte_addr = (ULONG64)UtilpAddressToPte((void*)addr);

	if (MmIsAddressValid((void*)temp_PageTableStruct.pxe_addr))
	{
		temp_PageTableStruct.pxe.value = UtilpAddressToPxe((void*)addr)->value;
	}
	if (MmIsAddressValid((void*)temp_PageTableStruct.ppe_addr))
	{
		temp_PageTableStruct.ppe.value = UtilpAddressToPpe((void*)addr)->value;
	}
	if (MmIsAddressValid((void*)temp_PageTableStruct.pde_addr) && !temp_PageTableStruct.ppe.Bits.large_page)
	{
		temp_PageTableStruct.pde.value = UtilpAddressToPde((void*)addr)->value;
		if (MmIsAddressValid((void*)temp_PageTableStruct.pte_addr) && !temp_PageTableStruct.pde.Bits.large_page)
		{
			temp_PageTableStruct.pte.value = UtilpAddressToPte((void*)addr)->value;
		}
	}


	KeUnstackDetachProcess(&kapc);
	return temp_PageTableStruct;
}