#include "EmunProcess.h"


#include "CRT/NtSysAPI_Func.hpp"





bool EmunProcess::Get_Process_Image(HANDLE Process_Handle, UNICODE_STRING* Process_Path)
{
	NTSTATUS status = 0;
	ULONG Query_Return_Lenght = 0;
	UNICODE_STRING* temp_process_image_name = nullptr;
	FILE_OBJECT* process_image_file_object = nullptr;
	DEVICE_OBJECT* process_image_device_object = nullptr;
	OBJECT_NAME_INFORMATION* process_image_object_name = nullptr;

	//get full image name
	status = ZwQueryInformationProcess(Process_Handle, ProcessImageFileName,
		nullptr, 0, &Query_Return_Lenght);
	temp_process_image_name = (UNICODE_STRING*)new char[Query_Return_Lenght];
	RtlZeroMemory(temp_process_image_name, Query_Return_Lenght);
	//frist call ZwQueryInformationProcess get how long memory for we need
	status = ZwQueryInformationProcess(Process_Handle, ProcessImageFileName,
		temp_process_image_name, Query_Return_Lenght, &Query_Return_Lenght);
	if (!NT_SUCCESS(status))
	{
		goto Clean;
	}

	//conversion the image path
	status = IoGetDeviceObjectPointer(temp_process_image_name, SYNCHRONIZE,
		&process_image_file_object, &process_image_device_object);
	if (!NT_SUCCESS(status))
	{
		goto Clean;
	}
	status = IoQueryFileDosDeviceName(process_image_file_object, &process_image_object_name);
	if (!NT_SUCCESS(status))
	{
		goto Clean;
	}
	Process_Path->Length = process_image_object_name->Name.Length;
	Process_Path->MaximumLength = process_image_object_name->Name.MaximumLength;
	Process_Path->Buffer = (PWCH)new char[Process_Path->MaximumLength];
	RtlCopyMemory(Process_Path->Buffer,
		process_image_object_name->Name.Buffer, Process_Path->MaximumLength);

	ExFreePool(process_image_object_name);
	delete[](char*)temp_process_image_name;
	ObDereferenceObject(process_image_file_object);
	return true;
Clean:
	if (process_image_object_name)
	{
		ExFreePool(process_image_object_name);
	}
	if (temp_process_image_name)
	{
		delete[](char*)temp_process_image_name;
	}
	if (process_image_file_object)
	{
		ObDereferenceObject(process_image_file_object);
	}
	return false;
}











bool EmunProcess::EmunProcessALL()
{
	bool first_check = false;
	_Process_List.clear();

	void* pBuffer = NULL;

	do 
	{
		ULONG cbBuffer = 0;
		NTSTATUS status = 0;

		if (NT_SUCCESS(ZwQuerySystemInformation(SystemProcessInformation, pBuffer, 0, &cbBuffer)))
		{
			first_check = false;//要不就是有人hook了 不然咋可能
			break;
		}
		pBuffer = ExAllocatePool(NonPagedPool, cbBuffer);
		if (!pBuffer)
		{
			first_check = false;
			break;
		}
		status = ZwQuerySystemInformation(SystemProcessInformation, pBuffer, cbBuffer, &cbBuffer);
		if (!NT_SUCCESS(status))
		{
			first_check = false;
			break;
		}
		PSYSTEM_PROCESS_INFORMATION pInfo = (PSYSTEM_PROCESS_INFORMATION)pBuffer;
		while (true)
		{
			PROCESS_LIST temp_list;
			RtlZeroMemory(&temp_list, sizeof(PROCESS_LIST));
			temp_list.PID = (ULONG64)pInfo->ProcessId;
			if (pInfo->ImageName.Length != 0 && MmIsAddressValid(pInfo->ImageName.Buffer))
			{
				RtlCopyMemory(temp_list.Name, pInfo->ImageName.Buffer, pInfo->ImageName.MaximumLength);
			}
			temp_list.Check = false;
			if (temp_list.PID == 0 || temp_list.PID == 4)
			{
				temp_list.Check = true;
			}
			PEPROCESS tempep;
			status = PsLookupProcessByProcessId((HANDLE)temp_list.PID, &tempep);
			if (NT_SUCCESS(status))
			{
				temp_list.EPROCESS = (ULONG64)tempep;
				ObDereferenceObject(tempep);
			}


			HANDLE handle;
			OBJECT_ATTRIBUTES ObjectAttributes;
			CLIENT_ID clientid;
			InitializeObjectAttributes(&ObjectAttributes, 0, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
			clientid.UniqueProcess = (HANDLE)temp_list.PID;
			clientid.UniqueThread = 0;
			if (NT_SUCCESS(ZwOpenProcess(&handle, PROCESS_ALL_ACCESS, &ObjectAttributes, &clientid)))
			{
				UNICODE_STRING temp_str;
				if (Get_Process_Image(handle, &temp_str))
				{
					RtlCopyMemory(temp_list.Path, temp_str.Buffer, temp_str.MaximumLength);
					delete temp_str.Buffer;
				}
			}

			_Process_List.push_back(temp_list);
			
			if (pInfo->NextEntryOffset == 0)
				break;
			pInfo = (PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)pInfo) + pInfo->NextEntryOffset);
		}
	} while (false);

	if (pBuffer)
	{
		ExFreePool(pBuffer);
	}

	NTSTATUS                       status = STATUS_SUCCESS;
	HANDLE                         hret = NULL;
	PEPROCESS                      tempep = NULL;
	LPSTR                         imagefilename = NULL;

	for (int i = 8; i < 65535; i = i + 4)
	{
		status = PsLookupProcessByProcessId((HANDLE)i, &tempep);
		if (NT_SUCCESS(status))
		{
			ObDereferenceObject(tempep);
			bool found = false;
			for (auto x : _Process_List)
			{
				if (x.PID == i)
				{
					found = true;
					x.Check = true;
					break;
				}
			}

			if (found == false)
			{
				UNICODE_STRING apiname;
				RtlInitUnicodeString(&apiname, L"PsGetProcessDebugPort");
				char *func = (char*)MmGetSystemRoutineAddress(&apiname);
				ULONG handletableoffset = *(PULONG)(func + 3) - 8;
				if (MmIsAddressValid((char*)tempep + handletableoffset) && *(ULONG64*)((ULONG64)tempep + handletableoffset) != 0)
				{
					PROCESS_LIST temp_list;
					temp_list.Check = false;
					RtlZeroMemory(&temp_list, sizeof(PROCESS_LIST));
					temp_list.PID = i;
					temp_list.EPROCESS = (ULONG64)tempep;
					RtlCopyMemory(temp_list.Name, PsGetProcessImageFileName(tempep), 15);

					HANDLE handle;
					OBJECT_ATTRIBUTES ObjectAttributes;
					CLIENT_ID clientid;
					InitializeObjectAttributes(&ObjectAttributes, 0, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
					clientid.UniqueProcess = (HANDLE)i;
					clientid.UniqueThread = 0;
					if (NT_SUCCESS(ZwOpenProcess(&handle, PROCESS_ALL_ACCESS, &ObjectAttributes, &clientid)))
					{
						UNICODE_STRING temp_str;
						if (Get_Process_Image(handle, &temp_str))
						{
							RtlCopyMemory(temp_list.Path, temp_str.Buffer, temp_str.MaximumLength);
							delete temp_str.Buffer;
						}
					}
					_Process_List.push_back(temp_list);
				}



				
			}
		}
	}

	EmunProcessSecondCheck();
	if (_ProcessSecondCheckList.size() == _Process_List.size())
	{
		return hret;
	}
	else
	{
		for (auto x : _ProcessSecondCheckList)
		{
			bool found = false;
			for (auto y : _Process_List)
			{
				if (y.PID == x)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				PROCESS_LIST temp_list;
				temp_list.Check = false;
				RtlZeroMemory(&temp_list, sizeof(PROCESS_LIST));
				temp_list.PID = x;
				temp_list.EPROCESS = (ULONG64)tempep;
				status = PsLookupProcessByProcessId((HANDLE)x, &tempep);
				if (NT_SUCCESS(status))
				{
					ObDereferenceObject(tempep);
					RtlCopyMemory(temp_list.Name, PsGetProcessImageFileName(tempep), 15);

					HANDLE handle;
					OBJECT_ATTRIBUTES ObjectAttributes;
					CLIENT_ID clientid;
					InitializeObjectAttributes(&ObjectAttributes, 0, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, 0);
					clientid.UniqueProcess = (HANDLE)x;
					clientid.UniqueThread = 0;
					if (NT_SUCCESS(ZwOpenProcess(&handle, PROCESS_ALL_ACCESS, &ObjectAttributes, &clientid)))
					{
						UNICODE_STRING temp_str;
						if (Get_Process_Image(handle, &temp_str))
						{
							RtlCopyMemory(temp_list.Path, temp_str.Buffer, temp_str.MaximumLength);
							delete temp_str.Buffer;
						}
					}
				}
				_Process_List.push_back(temp_list);
			}
		}
	}
	

	return hret;
}

void EmunProcess::EmunProcessSecondCheck()
{
	_ProcessSecondCheckList.clear();

	PETHREAD tempthd;
	NTSTATUS status;
	for (int i = 8; i < 65535; i = i + 4)
	{
		status = PsLookupThreadByThreadId((HANDLE)i, &tempthd);
		if (NT_SUCCESS(status))
		{
			ObDereferenceObject(tempthd);
			ULONG64 PID = (ULONG64)PsGetThreadProcessId(tempthd);
			_ProcessSecondCheckList.insert(PID);
		}
	}
}