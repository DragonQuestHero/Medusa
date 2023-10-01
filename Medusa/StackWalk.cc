#include "StackWalk.h"

#include "ntdll.h"
#include "Modules.h"
#include "KernelModules.h"



StackWalk::StackWalk(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this); 
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(5);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"TID");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(3, Qt::Horizontal, u8"StartAddr");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Module");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 150);
	ui.tableView->setColumnWidth(2, 200);
	ui.tableView->setColumnWidth(3, 200);
	ui.tableView->setColumnWidth(4, 250);
}

void StackWalk::ShowStackWalkThreadR0(ULONG64 TID)
{
	_Model->removeRows(0, _Model->rowCount());

	ULONG64 PID = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 item;
		item.dwSize = sizeof(item);
		if (Thread32First(hSnap, &item))
		{
			do
			{
				if (TID == item.th32ThreadID)
				{
					PID = item.th32OwnerProcessID;
					break;
				}
			} while (Thread32Next(hSnap, &item));
		}
		CloseHandle(hSnap);
	}
	Modules _Modules;
	std::vector<UserModule> temp_module_vector = _Modules.GetUserMoudleListR0(PID);
	std::vector<ULONG64> temp_vector = GetStackWalkThreadR0(TID);
	int i = 0;
	for (auto y : temp_vector)
	{
		void* startaddr = nullptr;
		std::wstring Module;

		HANDLE thread_handle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, TID);
		if (thread_handle)
		{
			ZwQueryInformationThread(thread_handle,
				ThreadQuerySetWin32StartAddress, &startaddr, sizeof(startaddr), NULL);
			CloseHandle(thread_handle);
		}
		
		bool found = false;
		for (auto x : temp_module_vector)
		{
			if ((ULONG64)y >= (ULONG64)x.Addr &&
				(ULONG64)y <= (ULONG64)x.Addr + x.Size)
			{
				ULONG64 offset = y - (ULONG64)x.Addr;
				std::wstringstream ret;
				ret << std::hex << "0x" << offset;
				Module = x.Name + std::wstring(L"+") + ret.str();
				found = true;
				break;
			}
		}
		if (!found)
		{
			KernelModules _KernelModules;
			_KernelModules.GetKernelModuleListR0();
			for (auto x : _KernelModules._KernelModuleListR0)
			{
				if ((ULONG64)y >= (ULONG64)x.Addr &&
					(ULONG64)y <= (ULONG64)x.Addr + x.Size)
				{
					ULONG64 offset = y - (ULONG64)x.Addr;
					std::wstringstream ret;
					ret << std::hex << "0x" << offset;

					if (x.Check == 0)
					{
						Module = C_TO_W((char*)x.Name);
						Module = Module + std::wstring(L"+") + ret.str();
					}
					else
					{
						Module = x.Name + std::wstring(L"+") + ret.str();
					}
					found = true;
					break;
				}
			}
		}

		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		_Model->setData(_Model->index(i, 1), TID);
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << (ULONG64)y;
		_Model->setData(_Model->index(i, 2), ret2.str().data());
		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)startaddr;
		_Model->setData(_Model->index(i, 3), ret.str().data());
		_Model->setData(_Model->index(i, 4), QString::fromWCharArray(Module.data()));
		i++;
	}

}

#define TEST_GetThreadStackWalk CTL_CODE(FILE_DEVICE_UNKNOWN,0x7113,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetThreadStackWalkNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7114,METHOD_BUFFERED ,FILE_ANY_ACCESS)
std::vector<ULONG64> StackWalk::GetStackWalkThreadR0(ULONG64 TID)
{
	std::vector<ULONG64> temp_vector;

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return temp_vector;
	}
	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetThreadStackWalkNumber, &TID, 8, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		ULONG64* temp_list = (ULONG64*)new char[process_number * sizeof(ULONG64)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetThreadStackWalk, &TID, 8, temp_list, sizeof(ULONG64) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				temp_vector.push_back(temp_list[i]);
			}
		}
		delete temp_list;
	} while (false);
	CloseHandle(m_hDevice);
	return temp_vector;
}