#include "Hypervisor.h"

#include <limits.h>
#include <intrin.h>

#include <stdint.h>

//https://secret.club/2020/01/12/battleye-hypervisor-detection.html
//ÆúÓÃ
bool Hypervisor::Check1()
{
	// SET THREAD PRIORITY TO THE HIGHEST
	const auto old_priority = SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	// CALCULATE CYCLES FOR 1000MS
	const auto timestamp_calibrator = __rdtsc();
	Sleep(1000);
	const auto timestamp_calibration = __rdtsc() - timestamp_calibrator;

	// TIME CPUID
	auto total_time = 0;
	for (size_t count = 0; count < 0x6694; count++)
	{
		// SAVE PRE CPUID TIME
		const auto timestamp_pre = __rdtsc();

		int cpuid_data[4] = {};
		__cpuid(cpuid_data, 0);

		// SAVE THE DELTA
		total_time += __rdtsc() - timestamp_pre;
	}

	// SAVE THE RESULT IN THE GLOBAL REPORT TABLE
	//battleye::report_table[0x1A8] = 10000000 * total_time / timestamp_calibration / 0x65;

	// RESTORE THREAD PRIORITY
	SetThreadPriority(GetCurrentThread(), old_priority);

	return true;
}



extern "C" void __fastcall _asm_fyl2xp1(void);

struct _cpuid_buffer_t
{
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
};

// resources [check #Improvement Part https://secret.club/2020/01/12/battleye-hypervisor-detection.html] 
bool take_time()
{
	// If the CPUID instruction execution time is longer than the arithmetic
	// instruction it’s a reliable indication that the system is virtualized
	// because under no circumstances should the arithmetic instruction take
	// longer than the CPUID execution to grab vendor, or version information.
	// This detection will also catch those using TSC offsetting/scaling.

	constexpr auto measure_time = 5;

	long long __cpuid_time = 0;
	long long __fyl2xp1_time = 0;

	LARGE_INTEGER frequency = {};
	LARGE_INTEGER start = {};
	LARGE_INTEGER end = {};

	QueryPerformanceFrequency(&frequency);

	// count the average time it takes to execute a CPUID instruction
	for (size_t i = 0; i < measure_time; ++i)
	{
		QueryPerformanceCounter(&start);
		_cpuid_buffer_t cpuid_data;
		__cpuid(reinterpret_cast<int*>(&cpuid_data), 1);
		QueryPerformanceCounter(&end);

		auto delta = end.QuadPart - start.QuadPart;

		delta *= 1000000000;
		delta /= frequency.QuadPart;

		__cpuid_time += delta;
	}

	// count the average time it takes to execute a FYL2XP1 instruction
	for (size_t i = 0; i < measure_time; ++i)
	{
		QueryPerformanceCounter(&start);
		_asm_fyl2xp1();
		QueryPerformanceCounter(&end);

		auto delta = end.QuadPart - start.QuadPart;

		delta *= 1000000000;
		delta /= frequency.QuadPart;

		__fyl2xp1_time += delta;
	}

	return __fyl2xp1_time <= __cpuid_time;
}

// resources [check https://secret.club/2020/01/12/battleye-hypervisor-detection.html] #Improvement Part
bool Hypervisor::take_time_cpuid_against_fyl2xp1()
{
	constexpr auto measure_times = 5;
	auto positives = 0;
	auto negatives = 0;

	// run the internal VM check multiple times to get an average result
	for (auto i = measure_times; i != 0; --i)
		take_time() ? ++positives : ++negatives;

	// if there are more positive results than negative results, the
	// process is likely running inside a VM
	const bool decision = (positives >= negatives);

	return decision;
}

// resources https://secret.club/2020/04/13/how-anti-cheats-detect-system-emulation.html
bool Hypervisor::check_invalid_leaf()
{
	constexpr unsigned int invalid_leaf = 0x04201337;
	constexpr unsigned int valid_leaf = 0x40000000;

	_cpuid_buffer_t InvalidLeafResponse = {};
	_cpuid_buffer_t ValidLeafResponse = {};

	__cpuid(reinterpret_cast<int32_t*>(&InvalidLeafResponse), invalid_leaf);
	__cpuid(reinterpret_cast<int32_t*>(&ValidLeafResponse), valid_leaf);

	if ((InvalidLeafResponse.EAX != ValidLeafResponse.EAX) ||
		(InvalidLeafResponse.EBX != ValidLeafResponse.EBX) ||
		(InvalidLeafResponse.ECX != ValidLeafResponse.ECX) ||
		(InvalidLeafResponse.EDX != ValidLeafResponse.EDX))
		return true;

	return false;
}

// resources https://secret.club/2020/04/13/how-anti-cheats-detect-system-emulation.html
bool Hypervisor::check_highest_low_function_leaf()
{
	constexpr auto queryVendorIdMagic = 0x40000000;

	_cpuid_buffer_t regs = {};
	__cpuid(reinterpret_cast<int32_t*>(&regs), queryVendorIdMagic);

	_cpuid_buffer_t reserved_regs = {};
	__cpuid(reinterpret_cast<int32_t*>(&reserved_regs), 1);

	__cpuid(reinterpret_cast<int32_t*>(&reserved_regs), reserved_regs.EAX);

	if (reserved_regs.EAX != regs.EAX ||
		reserved_regs.EBX != regs.EBX ||
		reserved_regs.ECX != regs.ECX ||
		reserved_regs.EDX != regs.EDX)
		return true;

	return false;
}

// resouces https://kb.vmware.com/s/article/1009458
bool Hypervisor::check_for_known_hypervisor()
{
	_cpuid_buffer_t cpuInfo = {};
	__cpuid(reinterpret_cast<int32_t*>(&cpuInfo), 1);

	if (!(cpuInfo.ECX & (1 << 31))) // check bit 31 of register ECX, which is “hypervisor present bit?
		return false;               // if not present return

	// we know hypervisor is present we can query the vendor id.
	constexpr auto queryVendorIdMagic = 0x40000000;
	__cpuid(reinterpret_cast<int32_t*>(&cpuInfo), queryVendorIdMagic);

	// construct string for our vendor name
	constexpr auto size = 13;
	const auto presentVendor = new char[size];
	memcpy(presentVendor + 0, &cpuInfo.EBX, 4);
	memcpy(presentVendor + 4, &cpuInfo.ECX, 4);
	memcpy(presentVendor + 8, &cpuInfo.EDX, 4);
	presentVendor[12] = '\0';

	// check against known vendor names
	const char* vendors[]{
		"KVMKVMKVM\0\0\0", // KVM 
		"Microsoft Hv",    // Microsoft Hyper-V or Windows Virtual PC */
		"VMwareVMware",    // VMware 
		"XenVMMXenVMM",    // Xen 
		"prl hyperv  ",    // Parallels
		"VBoxVBoxVBox"     // VirtualBox 
	};

	for (const auto& vendor : vendors)
	{
		if (!memcmp(vendor, presentVendor, size))
		{
			return true;
		}
	}
	return false;
}


bool Hypervisor::HypervisorDetection()
{
	if (take_time_cpuid_against_fyl2xp1() && 
		check_invalid_leaf() &&
		check_highest_low_function_leaf() &&
		check_for_known_hypervisor())
	{
		return true;
	}
	return false;
}