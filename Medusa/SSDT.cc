#include "SSDT.h"




#define TEST_GetSSDTList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7123,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetSSDTListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7124,METHOD_BUFFERED ,FILE_ANY_ACCESS)

#define TEST_GetSSSDTList CTL_CODE(FILE_DEVICE_UNKNOWN,0x7125,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetSSSDTListNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7126,METHOD_BUFFERED ,FILE_ANY_ACCESS)




void SSDT::GetALLSSDT()
{
	/*__KernelCallBackListR0.clear();

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
	return __KernelCallBackListR0;*/
}

void SSDT::GetALLShadowSSDT()
{
	
}


