#include <ntifs.h>
#include <intrin.h>

#include <string>


#include "IO_Control.h"


#ifdef __DEBUG
#include "Test.h"
#endif // DEBUG



IO_Control *_IO_Control;

void DriverUnload(PDRIVER_OBJECT drive_object)
{
	_IO_Control->Delete_IO_Control();
	delete _IO_Control;
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Unload Over!\n");
}


extern "C" NTSTATUS DriverMain(PDRIVER_OBJECT drive_object, PUNICODE_STRING path)
{
	drive_object->DriverUnload = DriverUnload;

	_IO_Control = new IO_Control(drive_object);
	if (!NT_SUCCESS(_IO_Control->Create_IO_Control()))
	{
		_IO_Control->Delete_IO_Control();
		delete _IO_Control;
		return STATUS_IN_PAGE_ERROR;
	}

#ifdef __DEBUG
	TestALL(drive_object);
#endif // DEBUG
	return STATUS_SUCCESS;
}