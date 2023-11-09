#include "KernelCallBackScanner.h"



#define TEST_GetCallBackList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7119,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetCallBackListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7120,METHOD_BUFFERED ,FILE_ANY_ACCESS)
std::vector<ObjectCallBackList> KernelCallBackScanner::GetALLCallBackList()
{
	__KernelCallBackListR0.clear();

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return __KernelCallBackListR0;
	}
	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetCallBackListNumber, 0, 0, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		ObjectCallBackList* temp_list = (ObjectCallBackList*)new char[process_number * sizeof(ObjectCallBackList)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetCallBackList, 0, 0, temp_list, sizeof(ObjectCallBackList) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				__KernelCallBackListR0.push_back(temp_list[i]);
			}
		}
		delete temp_list;
	} while (false);
	CloseHandle(m_hDevice);
	return __KernelCallBackListR0;
}