#include "CallBackScanner.h"



std::vector<ObjectCallBackList> CallBackScanner::GetObjectCallBackList(POBJECT_TYPE pObject)
{
	std::vector<ObjectCallBackList> _temp_vector;

	POBJECT_CALLBACK_ENTRY pCallbackEntry = *(POBJECT_CALLBACK_ENTRY*)((UCHAR*)pObject + _CallbackListOffset);
	POBJECT_CALLBACK_ENTRY Head = pCallbackEntry;
	while (TRUE)
	{
		if (MmIsAddressValid(pCallbackEntry) && pCallbackEntry->ObjectType != nullptr && MmIsAddressValid(pCallbackEntry->CallbackEntry))
		{
			ObjectCallBackList temp_list;
			RtlZeroMemory(&temp_list, sizeof(ObjectCallBackList));
			temp_list.PreOperation = (UINT64)pCallbackEntry->PreOperation;
			temp_list.PostOperation = (UINT64)pCallbackEntry->PostOperation;
			temp_list.CallbackEntry = (UINT64)pCallbackEntry;
			if (pObject == *PsProcessType)
			{
				temp_list.Type = 1;
			}
			if (pObject == *PsThreadType)
			{
				temp_list.Type = 2;
			}
			RtlCopyMemory(temp_list.Name, pCallbackEntry->CallbackEntry->Altitude.Buffer, pCallbackEntry->CallbackEntry->Altitude.MaximumLength);
			_temp_vector.push_back(temp_list);
		}
		pCallbackEntry = (POBJECT_CALLBACK_ENTRY)pCallbackEntry->CallbackList.Flink;
		if (Head == pCallbackEntry)
			break;
	}
	return _temp_vector;
}


std::vector<ObjectCallBackList> CallBackScanner::GetNotifyRoutineList(ULONG64 Type)
{
	std::vector<ObjectCallBackList> _temp_vector;
	ULONG64* Psp = 0;

	if (Type == 3)
	{
		Psp = (ULONG64*)_LoadImageNotifyRoutine;
	}
	if (Type == 4)
	{
		Psp = (ULONG64*)_ProcessNotifyRoutine;
	}
	if (Type == 5)
	{
		Psp = (ULONG64*)_ThreadNotifyRoutine;
	}

	typedef struct _PS_CALLBACK_ENTRY
	{
		PVOID* Callback;
		LARGE_INTEGER* Fillz;
	} PS_CALLBACK_ENTRY, * PPS_CALLBACK_ENTRY;

	for (int i = 0; i < 0x10; ++i)
	{
		if (!MmIsAddressValid((PVOID)Psp[i]))
			continue;

#define Mask3Bits(addr)	 (((ULONG_PTR) (addr)) & ~7)
		PPS_CALLBACK_ENTRY monCallBack = (PPS_CALLBACK_ENTRY)Mask3Bits(Psp[i]);

		if (MmIsAddressValid(monCallBack->Callback))
		{
			ObjectCallBackList temp_list;
			RtlZeroMemory(&temp_list, sizeof(ObjectCallBackList));
			temp_list.PreOperation = (UINT64)monCallBack->Callback;
			temp_list.CallbackEntry = (UINT64)monCallBack;
			temp_list.Type = Type;
			_temp_vector.push_back(temp_list);
		}
	}
	return _temp_vector;
}


std::vector<ObjectCallBackList> CallBackScanner::GetALLCallBackList()
{
	_ObjectCallBackList.clear();
	std::vector<ObjectCallBackList> _temp_vector;

	_temp_vector = GetObjectCallBackList(*PsProcessType);
	for (auto x : _temp_vector)
	{
		_ObjectCallBackList.push_back(x);
	}
	_temp_vector = GetObjectCallBackList(*PsThreadType);
	for (auto x : _temp_vector)
	{
		_ObjectCallBackList.push_back(x);
	}
	_temp_vector = GetNotifyRoutineList(3);
	for (auto x : _temp_vector)
	{
		_ObjectCallBackList.push_back(x);
	}
	_temp_vector = GetNotifyRoutineList(4);
	for (auto x : _temp_vector)
	{
		_ObjectCallBackList.push_back(x);
	}
	_temp_vector = GetNotifyRoutineList(5);
	for (auto x : _temp_vector)
	{
		_ObjectCallBackList.push_back(x);
	}



	return _ObjectCallBackList;
}