#include "Threads.h"

#include "ntdll.h"

#include "Modules.h"


Threads::Threads(QWidget* parent)
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
	_Model->setHeaderData(2, Qt::Horizontal, u8"ETHREAD");
	_Model->setHeaderData(3, Qt::Horizontal, u8"StartAddr");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Module");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 180);
	ui.tableView->setColumnWidth(2, 150);
	ui.tableView->setColumnWidth(3, 150);
	ui.tableView->setColumnWidth(4, 300);

	//this->setWindowFlags(Qt::FramelessWindowHint);
}

std::vector<ThreadList> Threads::GetThreadListR3(ULONG64 PID)
{
	std::vector<ThreadList> temp_vector;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 item;
		item.dwSize = sizeof(item);

		ThreadList temp_list;
		RtlZeroMemory(&temp_list, sizeof(ThreadList));

		if (Thread32First(hSnap, &item))
		{
			do
			{
				temp_list.TID = item.th32ThreadID;
				if (PID == item.th32OwnerProcessID)
				{
					HANDLE thread_handle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, item.th32ThreadID);
					if (!thread_handle)
					{
						temp_vector.push_back(temp_list);
						break;
					}
					void* startaddr = nullptr;
					NTSTATUS status = ZwQueryInformationThread(thread_handle,
						ThreadQuerySetWin32StartAddress, &startaddr, sizeof(startaddr), NULL);
					if (!NT_SUCCESS(status))
					{
						temp_vector.push_back(temp_list);
						CloseHandle(thread_handle);
						break;
					}

					temp_list.StartAddr = (ULONG64)startaddr;

					Modules _Modules;
					std::vector<MODULEENTRY32W> temp_module_vector = _Modules.GetUserMoudleListR3(PID);
					if (temp_module_vector.size() != 0)
					{
						for (auto x : temp_module_vector)
						{
							if ((ULONG64)startaddr >= (ULONG64)x.modBaseAddr &&
								(ULONG64)startaddr <= (ULONG64)x.modBaseAddr + x.modBaseSize)
							{
								ULONG64 offset = temp_list.StartAddr - (ULONG64)x.modBaseAddr;
								std::wstringstream ret;
								ret << std::hex << "0x" << offset;

								std::wstring temp_str = x.szModule + std::wstring(L"+") + ret.str();
								RtlCopyMemory(temp_list.Name, temp_str.data(), temp_str.length()*2);
							}
						}
					}
					temp_vector.push_back(temp_list);
					CloseHandle(thread_handle);
				}
			} while (Thread32Next(hSnap, &item));
		}
		CloseHandle(hSnap);
	}

	return temp_vector;
}