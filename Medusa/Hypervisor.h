#pragma once
#include <windows.h>


class Hypervisor
{
public:
	Hypervisor() = default;
	~Hypervisor() = default;
public:
	bool Check1();
	bool HypervisorDetection();//https://github.com/void-stack/Hypervisor-Detection
public:
	bool take_time_cpuid_against_fyl2xp1();
	bool check_invalid_leaf();
	bool check_highest_low_function_leaf();
	bool check_for_known_hypervisor();
private:

};

