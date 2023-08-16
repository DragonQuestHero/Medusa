#include "IO_Control.h"

#include <functional>

#define DEVICE_NAME L"\\Device\\IO_Control"
#define LINK_NAME L"\\??\\IO_Control"
IO_Control* IO_Control::_This;

#define TEST_GetALLProcess CTL_CODE(FILE_DEVICE_UNKNOWN,0x7100,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLProcessNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7101,METHOD_BUFFERED ,FILE_ANY_ACCESS)

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

	DbgPrint("Create Device and Link SUCCESS!\n");

	Driver_Object->MajorFunction[IRP_MJ_CREATE] = IO_Control::IO_Default;
	Driver_Object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IO_Control::Code_Control_Center;

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
	char *Input_Buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;

	DbgBreakPoint();
	if (Io_Control_Code == TEST_GetALLProcessNumber)
	{
		_This->_EmunProcess.EmunProcessALL();
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_EmunProcess._Process_List.size();
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

	if (Io_Control_Code == TEST_GetALLProcess)
	{
		if (Output_Lenght < _This->_EmunProcess._Process_List.size() * sizeof(PROCESS_LIST))
		{
			pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			pIrp->IoStatus.Information = 0;
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
		}

		int i = 0;
		for (auto x: _This->_EmunProcess._Process_List)
		{
			RtlCopyMemory(Input_Buffer + i * sizeof(PROCESS_LIST), &x, sizeof(PROCESS_LIST));
			i++;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = _This->_EmunProcess._Process_List.size() * sizeof(PROCESS_LIST);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

	pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}