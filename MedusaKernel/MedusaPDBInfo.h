#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "CRT/NtSysAPI_Func.hpp"


struct PDBInfo
{
	ULONG64 MiProcessLoaderEntry;
	ULONG64 PiDDBLock;
	ULONG64 PiDDBCacheTable;
	ULONG64 MmUnloadedDrivers;
	ULONG64 MmLastUnloadedDriver;
	ULONG64 KernelHashBucketList;
	ULONG64 HashCacheLock;
	ULONG64 CiEaCacheLookasideList;
};



class MedusaPDBInfo
{
public:
	MedusaPDBInfo() = default;
	~MedusaPDBInfo() = default;
public:
public:
	//static MedusaPDBInfo *_This;
public:
	static PDBInfo _PDBInfo;
private:

};

