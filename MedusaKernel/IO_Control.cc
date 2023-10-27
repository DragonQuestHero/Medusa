#include "IO_Control.h"

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

#define TEST_GetRWMemory CTL_CODE(FILE_DEVICE_UNKNOWN,0x7106,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetALLThreads CTL_CODE(FILE_DEVICE_UNKNOWN,0x7107,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLThreadsNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7108,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetPDBInfo CTL_CODE(FILE_DEVICE_UNKNOWN,0x7109,METHOD_BUFFERED ,FILE_ANY_ACCESS)

//#define TEST_GetUnLoadDriver CTL_CODE(FILE_DEVICE_UNKNOWN,0x7110,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetUnLoadKernelModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7111,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetUnLoadKernelModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7112,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetThreadStackWalk CTL_CODE(FILE_DEVICE_UNKNOWN,0x7113,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetThreadStackWalkNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7114,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_DumpDriver CTL_CODE(FILE_DEVICE_UNKNOWN,0x7115,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetIOCTLFunction CTL_CODE(FILE_DEVICE_UNKNOWN,0x7116,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_ReadKernelMemory CTL_CODE(FILE_DEVICE_UNKNOWN,0x7117,METHOD_BUFFERED ,FILE_ANY_ACCESS)

NTSTATUS IO_Control::Create_IO_Control()
{
	NTSTATUS status = 0;
	//创建设备对象
	RtlInitUnicodeString(&Device_Name, DEVICE_NAME);
	status = IoCreateDevice(Driver_Object, 0, &Device_Name, FILE_DEVICE_UNKNOWN, 0, FALSE, &Device_Object);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Create Device error!\n");
		return status;
	}

	Device_Object->Flags |= DO_BUFFERED_IO;
	//创建符号连接
	RtlInitUnicodeString(&Link_Name, LINK_NAME);
	status = IoCreateSymbolicLink(&Link_Name, &Device_Name);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(Device_Object);
		DbgPrint("Create Link error!\n");
		return status;
	}


	Driver_Object->MajorFunction[IRP_MJ_CREATE] = IO_Control::IO_Default;
	Driver_Object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IO_Control::Code_Control_Center;


	_Threads.InitWin32StartAddressOffset();
	_KernelModules.Init(Driver_Object);


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

NTSTATUS IO_Control::Code_Control_Center(PDEVICE_OBJECT  DeviceObject, PIRP  pIrp)
{
	PIO_STACK_LOCATION irp = IoGetCurrentIrpStackLocation(pIrp);
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
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetALLProcess)
	{
		if (Output_Lenght < _This->_EmunProcess._Process_List.size() * sizeof(PROCESS_LIST))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
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
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetALLKernelModuleNumber)
	{
		_This->_KernelModules.GetKernelModuleListALL();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules._KernelModuleList.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetALLKernelModule)
	{
		if (Output_Lenght < _This->_KernelModules._KernelModuleList.size() * sizeof(KernelModulesVector))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
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
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetALLUserModuleNumber)
	{
		Modules _Modules;
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _Modules.GetWin32MoudleList(*(ULONG64*)Input_Buffer).size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
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
			return STATUS_SUCCESS;
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
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetRWMemory && Input_Lenght > 0)
	{
		NewNtReadWriteVirtualMemory((Message_NtReadWriteVirtualMemory*)Input_Buffer);
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = 0;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetALLThreadsNumber)
	{
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_Threads.GetThreadListR0(*(ULONG64*)Input_Buffer).size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetALLThreads)
	{
		std::vector<ThreadList> temp_vector = _This->_Threads.GetThreadListR0(*(ULONG64*)Input_Buffer);
		if (Output_Lenght < temp_vector.size() * sizeof(ThreadList))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
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
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetPDBInfo)
	{
		RtlCopyMemory(&_This->_MedusaPDBInfo._PDBInfo, Input_Buffer, sizeof(PDBInfo));
	}
	else if (Io_Control_Code == TEST_GetUnLoadKernelModuleNumber)
	{
		_This->_KernelModules.GetUnLoadKernelModuleList();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules._UnLoadKernelModuleList.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_GetUnLoadKernelModule)
	{
		if (Output_Lenght < _This->_KernelModules._UnLoadKernelModuleList.size() * sizeof(KernelUnloadModules))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
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
		return STATUS_SUCCESS;
	}
	
	if (Io_Control_Code == TEST_GetThreadStackWalkNumber)
	{
		_This->_Threads.StackWalkThread(*(ULONG64*)Input_Buffer);
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_Threads.temp_walk_vector.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
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
		return STATUS_SUCCESS;
	}
	else if (Io_Control_Code == TEST_DumpDriver && Output_Lenght > 0)
	{
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_KernelModules.DumpDriver(*(ULONG64*)Input_Buffer, Input_Buffer + 8);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	else if(Io_Control_Code == TEST_GetIOCTLFunction && Input_Lenght > 0 && Output_Lenght > 0)
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
		return STATUS_SUCCESS;
	}
	else if(Io_Control_Code == TEST_ReadKernelMemory && Input_Lenght > 0 && Output_Lenght > 0)
	{
		SIZE_T NumberOfBytesTransferred = 0;

		ULONG64 addr = *(ULONG64*)Input_Buffer;
		ULONG64 size = *(ULONG64*)(Input_Buffer + 8);
		void* temp_buffer = new char[size];
		if (temp_buffer)
		{
			RtlZeroMemory(temp_buffer, size);
			MM_COPY_ADDRESS SourceAddress;
			SourceAddress.VirtualAddress = (PVOID)addr;
			NTSTATUS status = MmCopyMemory(temp_buffer, SourceAddress, size, MM_COPY_MEMORY_VIRTUAL, &NumberOfBytesTransferred);
			if (NumberOfBytesTransferred != 0)
			{
				RtlCopyMemory(Input_Buffer, temp_buffer, NumberOfBytesTransferred);
			}
			delete temp_buffer;

			pIrp->IoStatus.Status = STATUS_SUCCESS;
			pIrp->IoStatus.Information = NumberOfBytesTransferred;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
		}
	}


	pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;

}