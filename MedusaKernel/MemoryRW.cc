#include "MemoryRW.h"


NTSTATUS NewNtReadWriteVirtualMemory(Message_NtReadWriteVirtualMemory* message)
{
	HANDLE ProcessHandle = message->ProcessHandle;
	PVOID BaseAddress = message->BaseAddress;
	void* Buffer = message->Buffer;
	SIZE_T BufferSize = message->BufferBytes;
	PSIZE_T NumberOfBytesWritten = message->ReturnBytes;

	SIZE_T BytesCopied;
	KPROCESSOR_MODE PreviousMode;
	PEPROCESS Process;
	NTSTATUS Status;
	PETHREAD CurrentThread;

	PAGED_CODE();

	CurrentThread = PsGetCurrentThread();
	PreviousMode = ExGetPreviousMode();
	if (PreviousMode != KernelMode)
	{

		if (((PCHAR)BaseAddress + BufferSize < (PCHAR)BaseAddress) ||
			((PCHAR)Buffer + BufferSize < (PCHAR)Buffer) ||
			((PVOID)((PCHAR)BaseAddress + BufferSize) > MM_HIGHEST_USER_ADDRESS) ||
			((PVOID)((PCHAR)Buffer + BufferSize) > MM_HIGHEST_USER_ADDRESS))
		{
			return STATUS_ACCESS_VIOLATION;
		}

		if (ARGUMENT_PRESENT(NumberOfBytesWritten))
		{
			__try
			{
				ProbeForWrite(NumberOfBytesWritten, sizeof(PSIZE_T), sizeof(ULONG));
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return GetExceptionCode();
			}
		}
	}

	BytesCopied = 0;
	Status = STATUS_SUCCESS;
	if (BufferSize != 0)
	{
		do
		{
			PEPROCESS temp_process;
			Status = PsLookupProcessByProcessId((HANDLE)message->ProcessId, &temp_process);
			if (!NT_SUCCESS(Status))
			{
				break;
			}
			if (message->Read)
			{
				Status = MmCopyVirtualMemory(temp_process, (PVOID)message->BaseAddress, PsGetCurrentProcess(),
					message->Buffer, message->BufferBytes, PreviousMode, &BytesCopied);
			}
			else
			{
				Status = MmCopyVirtualMemory(PsGetCurrentProcess(), message->Buffer, temp_process,
					(PVOID)message->BaseAddress, message->BufferBytes, PreviousMode, &BytesCopied);
			}
			ObDereferenceObject(temp_process);
		} while (false);
	}

	if (ARGUMENT_PRESENT(NumberOfBytesWritten))
	{
		__try
		{
			*NumberOfBytesWritten = BytesCopied;

		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			NOTHING;
		}
	}

	return Status;
}

volatile LONG number_of_processors = 0;
volatile bool _ALLCpuReday = false;
volatile int bbb = 0;
KEVENT _WaitEvent;
KSPIN_LOCK SpinLock;

ULONG_PTR KipiBroadcastWorker(
	ULONG_PTR Argument
)
{
	Message_NtReadWriteVirtualMemory* temp_NtReadWriteVirtualMemory = (Message_NtReadWriteVirtualMemory*)Argument;
	if (0 == (InterlockedDecrement(&number_of_processors)))
	{
		RtlCopyMemory(temp_NtReadWriteVirtualMemory->Buffer,
			temp_NtReadWriteVirtualMemory->BaseAddress, temp_NtReadWriteVirtualMemory->BufferBytes);
		_ALLCpuReday = true;
	}
	while (!_ALLCpuReday)
	{

	}
	return 0;
}


bool KernelSafeReadMemoryIPI(ULONG64 addr, void* Buffer, ULONG64 Size)
{
	Message_NtReadWriteVirtualMemory temp_NtReadWriteVirtualMemory;
	temp_NtReadWriteVirtualMemory.BaseAddress = (void*)addr;
	temp_NtReadWriteVirtualMemory.Buffer = Buffer;
	temp_NtReadWriteVirtualMemory.BufferBytes = Size;

	KeInitializeEvent(&_WaitEvent, NotificationEvent, FALSE);
	KeInitializeSpinLock(&SpinLock);
	number_of_processors = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	if (MmIsAddressValid(temp_NtReadWriteVirtualMemory.BaseAddress) &&
		MmIsAddressValid((void*)
		((ULONG64)temp_NtReadWriteVirtualMemory.BaseAddress + temp_NtReadWriteVirtualMemory.BufferBytes)))
	{
		KeIpiGenericCall(&KipiBroadcastWorker, (ULONG64)&temp_NtReadWriteVirtualMemory);
		while (!_ALLCpuReday)
		{

		}
		return true;
	}
	return false;
}



