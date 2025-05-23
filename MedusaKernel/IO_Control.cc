﻿#include "IO_Control.h"

#include <functional>

#include "Modules.h"
#include "MemoryRW.h"




#define DEVICE_NAME L"\\Device\\IO_Control"
#define LINK_NAME L"\\??\\IO_Control"
IO_Control* IO_Control::_This;

#define TEST_GetALLProcess CTL_CODE(FILE_DEVICE_UNKNOWN,0x7100,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLProcessNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7101,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetALLKernelModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7102,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLKernelModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7103,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetALLUserModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7104,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLUserModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7105,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_ReadUserMemoryFromKernel CTL_CODE(FILE_DEVICE_UNKNOWN,0x7106,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetALLThreads CTL_CODE(FILE_DEVICE_UNKNOWN,0x7107,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLThreadsNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7108,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define IOCTL_GetPDBInfo CTL_CODE(FILE_DEVICE_UNKNOWN,0x7109,METHOD_BUFFERED ,FILE_ANY_ACCESS)

//#define TEST_GetUnLoadDriver CTL_CODE(FILE_DEVICE_UNKNOWN,0x7110,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetUnLoadKernelModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7111,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetUnLoadKernelModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7112,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetThreadStackWalk CTL_CODE(FILE_DEVICE_UNKNOWN,0x7113,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetThreadStackWalkNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7114,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_DumpDriver CTL_CODE(FILE_DEVICE_UNKNOWN,0x7115,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetIOCTLFunction CTL_CODE(FILE_DEVICE_UNKNOWN,0x7116,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_ReadKernelMemory CTL_CODE(FILE_DEVICE_UNKNOWN,0x7117,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_InjectDLL CTL_CODE(FILE_DEVICE_UNKNOWN,0x7118,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetCallBackList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7119,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetCallBackListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7120,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetCR3 CTL_CODE(FILE_DEVICE_UNKNOWN,0x7121,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_SetCR3 CTL_CODE(FILE_DEVICE_UNKNOWN,0x7122,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetSSDTList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7123,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetSSDTListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7124,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetSSSDTList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7125,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetSSSDTListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7126,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetUserMemoryFromCR3 CTL_CODE(FILE_DEVICE_UNKNOWN,0x7127,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetUserMemoryFromCR3Number CTL_CODE(FILE_DEVICE_UNKNOWN,0x7128,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetPageTables CTL_CODE(FILE_DEVICE_UNKNOWN,0x7129,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_DriverUnload CTL_CODE(FILE_DEVICE_UNKNOWN,0x7130,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define IOCTL_KernelReadPhysicalMemory CTL_CODE(FILE_DEVICE_UNKNOWN,0x7131,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define IOCTL_KernelReadSpecialPhysicalMemory CTL_CODE(FILE_DEVICE_UNKNOWN,0x7132,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define IOCTL_KillProcess CTL_CODE(FILE_DEVICE_UNKNOWN,0x7133,METHOD_BUFFERED ,FILE_ANY_ACCESS)


NTSTATUS IO_Control::Create_IO_Control()
{
	NTSTATUS status = 0;
	//创建设备对象
	RtlInitUnicodeString(&Device_Name, DEVICE_NAME);
	status = IoCreateDevice(Driver_Object, 0, &Device_Name, FILE_DEVICE_UNKNOWN, 0, FALSE, &Device_Object);
	if (!NT_SUCCESS(status))
	{
		return status;
	}

	Device_Object->Flags |= DO_BUFFERED_IO;
	//创建符号连接
	RtlInitUnicodeString(&Link_Name, LINK_NAME);
	status = IoCreateSymbolicLink(&Link_Name, &Device_Name);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(Device_Object);
		return status;
	}


	Driver_Object->MajorFunction[IRP_MJ_CREATE] = IO_Control::IO_Default;
	Driver_Object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IO_Control::Code_Control_Center;


	_Threads.InitWin32StartAddressOffset();
	_KernelModules.Init(Driver_Object);

	if (!_PageTable.Init())
	{
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}

NTSTATUS IO_Control::Delete_IO_Control()
{
	IoDeleteSymbolicLink(&Link_Name);
	IoDeleteDevice(Device_Object);
	DbgPrint("Link_Unload\n");
	return STATUS_SUCCESS;
}

NTSTATUS IO_Control::IO_Default(PDEVICE_OBJECT  DeviceObject, PIRP  pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

bool IO_Control::IOCTL_ProcessThread(PIO_STACK_LOCATION irp, PIRP  pIrp)
{
	ULONG Io_Control_Code = irp->Parameters.DeviceIoControl.IoControlCode;
	ULONG Input_Lenght = irp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG Output_Lenght = irp->Parameters.DeviceIoControl.OutputBufferLength;
	char* Input_Buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;


	if (Io_Control_Code == TEST_GetALLProcessNumber)
	{
		_This->_EmunProcess.EmunProcessALL();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_EmunProcess._Process_List.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetALLProcess)
	{
		if (Output_Lenght < _This->_EmunProcess._Process_List.size() * sizeof(PROCESS_LIST))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return true;
		}

		int i = 0;
		for (auto x : _This->_EmunProcess._Process_List)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(PROCESS_LIST), &x, sizeof(PROCESS_LIST));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_EmunProcess._Process_List.size() * sizeof(PROCESS_LIST);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetALLThreadsNumber)
	{
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_Threads.GetThreadListR0(*(ULONG64*)Input_Buffer).size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetALLThreads)
	{
		std::vector<ThreadList> temp_vector = _This->_Threads.GetThreadListR0(*(ULONG64*)Input_Buffer);
		if (Output_Lenght < temp_vector.size() * sizeof(ThreadList))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return true;
		}
		int i = 0;
		for (auto x : temp_vector)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(ThreadList), &x, sizeof(ThreadList));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = temp_vector.size() * sizeof(ThreadList);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetThreadStackWalkNumber && Input_Lenght > 0)
	{
		_This->_Threads.StackWalkThread(*(ULONG64*)Input_Buffer);
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_Threads.temp_walk_vector.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetThreadStackWalk)
	{
		int i = 0;
		for (auto x : _This->_Threads.temp_walk_vector)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(ULONG64), &x, sizeof(ULONG64));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_Threads.temp_walk_vector.size() * sizeof(ULONG64);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	return false;
}

bool IO_Control::IOCTL_DLLInject(PIO_STACK_LOCATION irp, PIRP  pIrp)
{
	ULONG Io_Control_Code = irp->Parameters.DeviceIoControl.IoControlCode;
	ULONG Input_Lenght = irp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG Output_Lenght = irp->Parameters.DeviceIoControl.OutputBufferLength;
	char* Input_Buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;

	if (Io_Control_Code == TEST_InjectDLL && Input_Lenght > 0)
	{
		ULONG64 pid = *(ULONG64*)Input_Buffer;
		ULONG64 size = *(ULONG64*)(Input_Buffer + 8);
		void* temp_buffer = (void*)(Input_Buffer + 16);
		Modules _Modules;
		if (_Modules.R0MapInject(pid, size, temp_buffer))
		{
			pIrp->IoStatus.Information = 1;
		}
		else
		{
			pIrp->IoStatus.Information = 0;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}

	return false;
}

bool IO_Control::IOCTL_Modules(PIO_STACK_LOCATION irp, PIRP  pIrp)
{
	ULONG Io_Control_Code = irp->Parameters.DeviceIoControl.IoControlCode;
	ULONG Input_Lenght = irp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG Output_Lenght = irp->Parameters.DeviceIoControl.OutputBufferLength;
	char* Input_Buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;

	if (Io_Control_Code == TEST_GetALLKernelModuleNumber)
	{
		_This->_KernelModules.GetKernelModuleListALL();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules._KernelModuleList.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetALLKernelModule)
	{
		if (Output_Lenght < _This->_KernelModules._KernelModuleList.size() * sizeof(KernelModulesVector))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return true;
		}

		int i = 0;
		for (auto x : _This->_KernelModules._KernelModuleList)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(KernelModulesVector), &x, sizeof(KernelModulesVector));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules._KernelModuleList.size() * sizeof(KernelModulesVector);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetALLUserModuleNumber)
	{
		Modules _Modules;
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _Modules.GetWin32MoudleList(*(ULONG64*)Input_Buffer).size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetALLUserModule)
	{
		Modules _Modules;
		std::vector<UserModule> temp_vector = _Modules.GetWin32MoudleList(*(ULONG64*)Input_Buffer);
		if (Output_Lenght < temp_vector.size() * sizeof(UserModule))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return true;
		}
		int i = 0;
		for (auto x : temp_vector)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(UserModule), &x, sizeof(UserModule));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = temp_vector.size() * sizeof(UserModule);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetUnLoadKernelModuleNumber)
	{
		_This->_KernelModules.GetUnLoadKernelModuleList();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules._UnLoadKernelModuleList.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetUnLoadKernelModule)
	{
		if (Output_Lenght < _This->_KernelModules._UnLoadKernelModuleList.size() * sizeof(KernelUnloadModules))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return true;
		}

		int i = 0;
		for (auto x : _This->_KernelModules._UnLoadKernelModuleList)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(KernelUnloadModules), &x, sizeof(KernelUnloadModules));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules._UnLoadKernelModuleList.size() * sizeof(KernelUnloadModules);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_DumpDriver && Output_Lenght > 0)
	{
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules.DumpDriver(*(ULONG64*)Input_Buffer, Input_Buffer + 8);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}

	return false;
}

bool IO_Control::IOCTL_Memory(PIO_STACK_LOCATION irp, PIRP  pIrp)
{
	ULONG Io_Control_Code = irp->Parameters.DeviceIoControl.IoControlCode;
	ULONG Input_Lenght = irp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG Output_Lenght = irp->Parameters.DeviceIoControl.OutputBufferLength;
	char* Input_Buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;

	if (Io_Control_Code == TEST_ReadUserMemoryFromKernel && Input_Lenght > 0)
	{
		NewNtReadWriteVirtualMemory((Message_NtReadWriteVirtualMemory*)Input_Buffer);
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_ReadKernelMemory && Input_Lenght > 0 && Output_Lenght > 0)
	{
		if (MmIsAddressValid(Input_Buffer))
		{
			ULONG64 addr = *(ULONG64*)Input_Buffer;
			ULONG64 size = *(ULONG64*)(Input_Buffer + 8);
			if (ReadKernelMemory(addr, Input_Buffer, size))
			{
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = size;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return true;
			}
		}
		else
		{
			return false;
		}
	}
	else if (Io_Control_Code == IOCTL_KernelReadSpecialPhysicalMemory && Input_Lenght > 0 && Output_Lenght > 0)
	{
		if (MmIsAddressValid(Input_Buffer))
		{
			ULONG64 addr = *(ULONG64*)Input_Buffer;
			ULONG64 size = *(ULONG64*)(Input_Buffer + 8);
			if (KernelReadSpecialPhysicalMemory(addr, Input_Buffer, size))
			{
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = size;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return true;
			}
		}
		else
		{
			return false;
		}
	}
	else if (Io_Control_Code == IOCTL_KernelReadPhysicalMemory && Input_Lenght > 0 && Output_Lenght > 0)
	{
		if (MmIsAddressValid(Input_Buffer))
		{
			ULONG64 addr = *(ULONG64*)Input_Buffer;
			ULONG64 size = *(ULONG64*)(Input_Buffer + 8);
			if (KernelReadPhysicalMemory(addr, Input_Buffer, size))
			{
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = size;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return true;
			}
		}
		else
		{
			return false;
		}
	}
	else if (Io_Control_Code == TEST_GetCR3 && Output_Lenght > 0)
	{
		*(ULONG64*)Input_Buffer = __readcr3();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = 8;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetUserMemoryFromCR3Number)
	{
		ULONG64 pid = *(ULONG64*)Input_Buffer;
		_This->_UserMemoryList = ScannUserMemoryFromCR3(pid);
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_UserMemoryList.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetUserMemoryFromCR3)
	{
		int i = 0;
		for (auto x : _This->_UserMemoryList)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(UserMemoryListStructCR3), &x, sizeof(UserMemoryListStructCR3));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_UserMemoryList.size() * sizeof(UserMemoryListStructCR3);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}

	if (Io_Control_Code == TEST_GetPageTables)
	{
		if (MmIsAddressValid(Input_Buffer))
		{
			ULONG64 pid = *(ULONG64*)Input_Buffer;
			ULONG64 addr = *(ULONG64*)(Input_Buffer + 8);
			RtlCopyMemory(Input_Buffer, &_This->_PageTable.GetPageTable(pid, addr), sizeof(PageTableStruct));
			pIrp->IoStatus.Status = STATUS_SUCCESS;
			pIrp->IoStatus.Information = sizeof(PageTableStruct);
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return true;
		}
	}


	return false;
}

bool IO_Control::IOCTL_KernelInfo(PIO_STACK_LOCATION irp, PIRP  pIrp)
{
	ULONG Io_Control_Code = irp->Parameters.DeviceIoControl.IoControlCode;
	ULONG Input_Lenght = irp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG Output_Lenght = irp->Parameters.DeviceIoControl.OutputBufferLength;
	char* Input_Buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;

	if (Io_Control_Code == TEST_GetIOCTLFunction && Input_Lenght > 0 && Output_Lenght > 0)
	{
		std::vector<IOCTLS> temp_vector = _This->_KernelModules.GetIOCTLFunctionR0(*(ULONG64*)Input_Buffer);
		int i = 0;
		for (auto x : temp_vector)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(IOCTLS), &x, sizeof(IOCTLS));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = temp_vector.size() * sizeof(IOCTLS);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetCallBackListNumber)
	{
		_This->_CallBackScanner.GetALLCallBackList();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_CallBackScanner._ObjectCallBackList.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetCallBackList)
	{
		int i = 0;
		for (auto x : _This->_CallBackScanner._ObjectCallBackList)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(ObjectCallBackList), &x, sizeof(ObjectCallBackList));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_CallBackScanner._ObjectCallBackList.size() * sizeof(ObjectCallBackList);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetSSDTListNumber)
	{
		if (_This->_SSDT._SSDTALL.size() == 0)
		{
			_This->_SSDT.GetAllSSDT();
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_SSDT._SSDTALL.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetSSDTList)
	{
		int i = 0;
		for (auto x : _This->_SSDT._SSDTALL)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(SSDT_STRUCT), &x, sizeof(SSDT_STRUCT));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_SSDT._SSDTALL.size() * sizeof(SSDT_STRUCT);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetSSSDTListNumber)
	{
		if (_This->_SSDT._SSSDTALL.size() == 0)
		{
			_This->_SSDT.GetAllShadowSSDT();
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_SSDT._SSSDTALL.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}
	else if (Io_Control_Code == TEST_GetSSSDTList)
	{
		int i = 0;
		for (auto x : _This->_SSDT._SSSDTALL)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(SSDT_STRUCT), &x, sizeof(SSDT_STRUCT));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_SSDT._SSSDTALL.size() * sizeof(SSDT_STRUCT);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return true;
	}

	return false;
}

NTSTATUS IO_Control::Code_Control_Center(PDEVICE_OBJECT  DeviceObject, PIRP  pIrp)
{
	PIO_STACK_LOCATION irp = IoGetCurrentIrpStackLocation(pIrp);
	ULONG Io_Control_Code = irp->Parameters.DeviceIoControl.IoControlCode;
	ULONG Input_Lenght = irp->Parameters.DeviceIoControl.InputBufferLength;
	ULONG Output_Lenght = irp->Parameters.DeviceIoControl.OutputBufferLength;
	char* Input_Buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;


	if (IOCTL_ProcessThread(irp, pIrp))
	{
		return STATUS_SUCCESS;
	}
	if (IOCTL_Modules(irp, pIrp))
	{
		return STATUS_SUCCESS;
	}
	if (IOCTL_Memory(irp, pIrp))
	{
		return STATUS_SUCCESS;
	}
	if (IOCTL_DLLInject(irp, pIrp))
	{
		return STATUS_SUCCESS;
	}
	if (IOCTL_KernelInfo(irp, pIrp))
	{
		return STATUS_SUCCESS;
	}

	if (Io_Control_Code == IOCTL_KillProcess)
	{
		if (MmIsAddressValid(Input_Buffer))
		{
			ULONG64 mode = *(ULONG64*)Input_Buffer;
			ULONG64 pid = *(ULONG64*)(Input_Buffer + 8);
			bool ret = false;
			if (mode == 0)
			{
				ret = _This->_EmunProcess.KillProcess(pid);
			}
			else if (mode == 1)
			{
				ret = _This->_EmunProcess.KillProcess1(pid);
			}
			else if (mode == 2)
			{
				ret = _This->_EmunProcess.KillProcess2(pid);
			}
			else if (mode == 3)
			{
				ret = _This->_EmunProcess.KillProcess3(pid);
			}

			if (ret)
			{
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = 0;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return STATUS_SUCCESS;
			}
			else
			{
				pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				pIrp->IoStatus.Information = 0;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				return STATUS_UNSUCCESSFUL;
			}
		}
	}
	

	
	
	if (Io_Control_Code == IOCTL_GetPDBInfo)
	{
		RtlCopyMemory(&_This->_MedusaPDBInfo._PDBInfo, Input_Buffer, sizeof(PDBInfo));
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	
	
	
	


	if (Io_Control_Code == TEST_DriverUnload)
	{
		if (MmIsAddressValid(Input_Buffer))
		{
			PDRIVER_OBJECT temp_object = *(PDRIVER_OBJECT*)Input_Buffer;
			if (MmIsAddressValid(temp_object))
			{
				PDRIVER_UNLOAD temp_func = temp_object->DriverUnload;
				if (MmIsAddressValid(temp_object) && temp_func)
				{
					temp_func(temp_object);
					pIrp->IoStatus.Status = STATUS_SUCCESS;
					pIrp->IoStatus.Information = 0;
					IoCompleteRequest(pIrp, IO_NO_INCREMENT);
					return STATUS_SUCCESS;
				}
			}
		}
	}

	

	pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_UNSUCCESSFUL;

}