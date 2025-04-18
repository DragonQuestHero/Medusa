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

NTSTATUS NewNtReadWriteVirtualMemoryFromKernel(Message_NtReadWriteVirtualMemory* message)
{
	HANDLE ProcessHandle = message->ProcessHandle;
	PVOID BaseAddress = message->BaseAddress;
	void* Buffer = message->Buffer;
	SIZE_T BufferSize = message->BufferBytes;
	PSIZE_T NumberOfBytesWritten = message->ReturnBytes;

	SIZE_T BytesCopied;
	PEPROCESS Process;
	NTSTATUS Status;
	PETHREAD CurrentThread;

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
					message->Buffer, message->BufferBytes, KernelMode, &BytesCopied);
			}
			else
			{
				Status = MmCopyVirtualMemory(PsGetCurrentProcess(), message->Buffer, temp_process,
					(PVOID)message->BaseAddress, message->BufferBytes, KernelMode, &BytesCopied);
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

bool ReadKernelMemory(ULONG64 addr, void* Buffer, ULONG64 Size)
{
	SIZE_T NumberOfBytesTransferred = 0;
	void* temp_buffer = new char[Size];
	if (temp_buffer)
	{
		RtlZeroMemory(temp_buffer, Size);
		MM_COPY_ADDRESS SourceAddress;
		SourceAddress.VirtualAddress = (PVOID)addr;
		NTSTATUS status = MmCopyMemory(temp_buffer, SourceAddress, Size, MM_COPY_MEMORY_VIRTUAL, &NumberOfBytesTransferred);
		if (NumberOfBytesTransferred != 0)
		{
			RtlCopyMemory(Buffer, temp_buffer, NumberOfBytesTransferred);
		}
		else
		{
			if (KernelSafeReadMemoryIPI(addr, temp_buffer, Size))
			{
				NumberOfBytesTransferred = Size;
				RtlCopyMemory(Buffer, temp_buffer, NumberOfBytesTransferred);
			}
		}
		delete temp_buffer;
	}
	if (NumberOfBytesTransferred)
	{
		return true;
	}
	return false;
}


volatile LONG number_of_processors = 0;
volatile bool _ALLCpuReday = false;

ULONG_PTR KipiBroadcastWorker(
	ULONG_PTR Argument
)
{
	Message_NtReadWriteVirtualMemory* temp_NtReadWriteVirtualMemory = (Message_NtReadWriteVirtualMemory*)Argument;
	if (0 == (InterlockedDecrement(&number_of_processors)))
	{
		if (MmIsAddressValid(temp_NtReadWriteVirtualMemory->BaseAddress) && //要求极限的话 应该每一页都检查 不过就算检查了p位 也还是有可能炸
			MmIsAddressValid((void*)
				((ULONG64)temp_NtReadWriteVirtualMemory->BaseAddress + temp_NtReadWriteVirtualMemory->BufferBytes)))
		{
			RtlCopyMemory(temp_NtReadWriteVirtualMemory->Buffer,
				temp_NtReadWriteVirtualMemory->BaseAddress, temp_NtReadWriteVirtualMemory->BufferBytes);
		}
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

	number_of_processors = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	if (MmIsAddressValid(temp_NtReadWriteVirtualMemory.BaseAddress) &&
		MmIsAddressValid((void*)
		((ULONG64)temp_NtReadWriteVirtualMemory.BaseAddress + temp_NtReadWriteVirtualMemory.BufferBytes)))
	{
		KeIpiGenericCall(&KipiBroadcastWorker, (ULONG64)&temp_NtReadWriteVirtualMemory);
		return true;
	}
	return false;
}


// PA -> VA
void* UtilVaFromPa(ULONG64 pa) {
	PHYSICAL_ADDRESS pa2 = {};
	pa2.QuadPart = pa;
	return MmGetVirtualForPhysical(pa2);
}

std::vector<UserMemoryListStructCR3> ScannUserMemoryFromCR3(ULONG64 PID)
{
	std::vector<UserMemoryListStructCR3> temp_list;

	PEPROCESS temp_process;
	NTSTATUS Status = PsLookupProcessByProcessId((HANDLE)PID, &temp_process);
	if (!NT_SUCCESS(Status))
	{
		//加28直接取 给他贯的
		return temp_list;
	}
	ObDereferenceObject(temp_process);


	KAPC_STATE ApcState;
	KeStackAttachProcess(temp_process, &ApcState);

	CR3 temp_cr3;
	temp_cr3.BitAddress = __readcr3();

	//大专巅峰 半步本科!
	//void* page_table = ExAllocatePool(NonPagedPool, PAGE_SIZE);//我乃大专巅峰 谁敢蓝我!?
	//if (!page_table)
	//{
	//	return false;
	//}

	do 
	{
		ULONG64 cr3_va = (ULONG64)UtilVaFromPa(temp_cr3.Bits.PhysicalAddress << 12);
		if (!MmIsAddressValid((void*)cr3_va))
		{
			break;
		}
		for (int i = 0; i < 0x100; i++)
		{
			HardwarePteX64* temp_pxe = (HardwarePteX64*)(cr3_va + i * 8);
			if (temp_pxe->Bits.valid)
			{
				ULONG64 ppe_va = (ULONG64)UtilVaFromPa(temp_pxe->Bits.page_frame_number << 12);
				if (!MmIsAddressValid((void*)ppe_va))
				{
					break;
				}
				for (int x = 0; x < 512; x++)
				{
					HardwarePteX64* temp_ppe = (HardwarePteX64*)(ppe_va + x * 8);
					if (temp_ppe->Bits.large_page)
					{
						ADDRESS_STRUCTURE temp_addr_struct = { 0 };
						temp_addr_struct.Bits.Reserved = 0;
						temp_addr_struct.Bits.Offset = 0;
						temp_addr_struct.Bits.PML4Index = i;
						temp_addr_struct.Bits.PDPTIndex = x;
						temp_addr_struct.Bits.PDIndex = 0;
						temp_addr_struct.Bits.PEIndex = 0;
						UserMemoryListStructCR3 temp_struct = { 0 };
						temp_struct.Addr = temp_addr_struct.BitAddress;
						temp_struct.Size = PAGE_SIZE * 512 * 512;
						temp_struct.PteX64.value = temp_ppe->value;
						temp_list.push_back(temp_struct);
						continue;
					}
					if (temp_ppe->Bits.valid)
					{
						ULONG64 pde_va = (ULONG64)UtilVaFromPa(temp_ppe->Bits.page_frame_number << 12);
						if (!MmIsAddressValid((void*)pde_va))
						{
							break;
						}
						for (int y = 0; y < 512; y++)
						{
							HardwarePteX64* temp_pde = (HardwarePteX64*)(pde_va + y * 8);
							if (temp_pde->Bits.large_page)
							{
								ADDRESS_STRUCTURE temp_addr_struct = { 0 };
								temp_addr_struct.Bits.Reserved = 0;
								temp_addr_struct.Bits.Offset = 0;
								temp_addr_struct.Bits.PML4Index = i;
								temp_addr_struct.Bits.PDPTIndex = x;
								temp_addr_struct.Bits.PDIndex = y;
								temp_addr_struct.Bits.PEIndex = 0;
								UserMemoryListStructCR3 temp_struct = { 0 };
								temp_struct.Addr = temp_addr_struct.BitAddress;
								temp_struct.Size = PAGE_SIZE * 512;
								temp_struct.PteX64.value = temp_pde->value;
								temp_list.push_back(temp_struct);
								continue;
							}
							if (temp_pde->Bits.valid)
							{
								ULONG64 pte_va = (ULONG64)UtilVaFromPa(temp_pde->Bits.page_frame_number << 12);
								if (!MmIsAddressValid((void*)pte_va))
								{
									break;
								}
								for (int z = 0; z < 512; z++)
								{
									HardwarePteX64* temp_pte = (HardwarePteX64*)(pte_va + z * 8);
									if (temp_pte->Bits.valid)
									{
										ADDRESS_STRUCTURE temp_addr_struct = { 0 };
										temp_addr_struct.Bits.Reserved = 0;
										temp_addr_struct.Bits.Offset = 0;
										temp_addr_struct.Bits.PML4Index = i;
										temp_addr_struct.Bits.PDPTIndex = x;
										temp_addr_struct.Bits.PDIndex = y;
										temp_addr_struct.Bits.PEIndex = z;
										UserMemoryListStructCR3 temp_struct = { 0 };
										temp_struct.Addr = temp_addr_struct.BitAddress;
										temp_struct.Size = PAGE_SIZE;
										temp_struct.PteX64.value = temp_pte->value;
										temp_list.push_back(temp_struct);
									}
								}
							}
						}
					}
				}
			}
		}
	} while (false);
	KeUnstackDetachProcess(&ApcState);
	/*auto physical_memory_ranges = MmGetPhysicalMemoryRanges();
	for (int i = 0; i < 0x1000; i++)
	{
		if (physical_memory_ranges[i].BaseAddress.QuadPart == 0 &&
			physical_memory_ranges[i].NumberOfBytes.QuadPart == 0)
		{
			break;
		}
	}*/
	return temp_list;
}

bool CheckPhysicalMemoryIsAddressValid(ULONG64 addr)
{
	auto physical_memory_ranges = MmGetPhysicalMemoryRanges();
	if (!physical_memory_ranges)//特殊情况干脆不检查了
	{
		return false;
	}
	for (int i = 0; i < 0x1000; i++)
	{
		if (physical_memory_ranges[i].BaseAddress.QuadPart == 0 &&
			physical_memory_ranges[i].NumberOfBytes.QuadPart == 0)
		{
			break;
		}
		if (addr >= physical_memory_ranges[i].BaseAddress.QuadPart &&
			addr < (physical_memory_ranges[i].BaseAddress.QuadPart + physical_memory_ranges[i].NumberOfBytes.QuadPart))
		{
			ExFreePoolWithTag(physical_memory_ranges, 'hPmM');
			return true; // 地址在有效范围内
		}
	}
	ExFreePoolWithTag(physical_memory_ranges, 'hPmM');
	return false;
}

bool KernelReadPhysicalMemory(ULONG64 addr, void* Buffer, ULONG64 Size)
{
	if (CheckPhysicalMemoryIsAddressValid(addr))
	{
		void* va = UtilVaFromPa(addr);
		return ReadKernelMemory((ULONG64)va, Buffer, Size);
	}
	return false;
}

bool KernelReadSpecialPhysicalMemory(ULONG64 addr, void* Buffer, ULONG64 Size)
{
	if (Size > PAGE_SIZE)
	{
		return false;
	}
	ULONG64 offset = addr & 0xFFF;
	if (offset + Size > PAGE_SIZE)
	{
		return false;
	}

	PageTable _PageTable;
	ULONG64 temp_ptr = (ULONG64)ExAllocatePool(NonPagedPool, PAGE_SIZE);
	PageTableStruct temp_PageTableStruct = _PageTable.GetPageTable((ULONG64)PsGetCurrentProcessId(), temp_ptr);
	if (!temp_PageTableStruct.pte_addr)
	{
		ExFreePool((void*)temp_ptr);
		return false;
	}
	ULONG64 old_pfn = temp_PageTableStruct.pte.Bits.page_frame_number;
	HardwarePte* temp_pte = (HardwarePte*)temp_PageTableStruct.pte_addr;
	temp_pte->Bits.page_frame_number = addr >> 12;
	__invlpg((void*)temp_ptr);
	ULONG64 addr_offset = temp_ptr + offset;
	RtlCopyMemory(Buffer, (void*)addr_offset, Size);
	temp_pte->Bits.page_frame_number = old_pfn;
	__invlpg((void*)temp_ptr);
	ExFreePool((void*)temp_ptr);
	return true;
}