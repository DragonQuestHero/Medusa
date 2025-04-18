#pragma once
#include <ntifs.h>
#include <intrin.h>

#include "EmunProcess.h"
#include "KernelModules.h"
#include "Threads.h"
#include "MedusaPDBInfo.h"
#include "CallBackScanner.h"
#include "SSDT.h"
#include "MemoryRW.h"
#include "PageTable.h"



class IO_Control
{
public:
	IO_Control(PDRIVER_OBJECT drive_object)
	{
		Driver_Object = drive_object;
		_This = this;
	}
	~IO_Control() = default;
public:
	NTSTATUS Create_IO_Control();
	NTSTATUS Delete_IO_Control();
private:
	static IO_Control* _This;
	static NTSTATUS IO_Default(PDEVICE_OBJECT  DeviceObject, PIRP  pIrp);
	static NTSTATUS Code_Control_Center(PDEVICE_OBJECT  DeviceObject, PIRP  pIrp);
	static bool IOCTL_ProcessThread(PIO_STACK_LOCATION irp, PIRP  pIrp);
	static bool IOCTL_DLLInject(PIO_STACK_LOCATION irp, PIRP  pIrp);
	static bool IOCTL_Modules(PIO_STACK_LOCATION irp, PIRP  pIrp);
	static bool IOCTL_Memory(PIO_STACK_LOCATION irp, PIRP  pIrp);
	static bool IOCTL_KernelInfo(PIO_STACK_LOCATION irp, PIRP  pIrp);
private:
	DRIVER_OBJECT *Driver_Object = nullptr;
	DEVICE_OBJECT *Device_Object = nullptr;
	UNICODE_STRING Device_Name;
	UNICODE_STRING Link_Name;
private:
	EmunProcess _EmunProcess;
	KernelModules _KernelModules;
	Threads _Threads;
	MedusaPDBInfo _MedusaPDBInfo;
	CallBackScanner _CallBackScanner;
	SSDT _SSDT;
	PageTable _PageTable;
private:
	std::vector<UserMemoryListStructCR3> _UserMemoryList;
};

