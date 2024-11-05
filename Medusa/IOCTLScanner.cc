#include "IOCTLScanner.h"

IOCTLScanner::IOCTLScanner(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(4);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"MJFunction");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(3, Qt::Horizontal, u8"Module");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 250);
	ui.tableView->setColumnWidth(2, 200);
	ui.tableView->setColumnWidth(3, 200);

	_TableView_Action_Inject.setText("MemoryView");
	ui.tableView->addAction(&_TableView_Action_Inject);
	ui.tableView->setContextMenuPolicy(Qt::ActionsContextMenu);
	connect(&_TableView_Action_Inject, SIGNAL(triggered(bool)), SLOT(MemoryView(bool)));
}

std::string IOCTLScanner::GetFunctionName(ULONG64 Index)
{
	std::string funciton_name;
	switch (Index)
	{
	case 0:
		funciton_name = "IRP_MJ_CREATE";
		break;
	case 1:
		funciton_name = "IRP_MJ_CREATE_NAMED_PIPE";
		break;
	case 2:
		funciton_name = "IRP_MJ_CLOSE";
		break;
	case 3:
		funciton_name = "IRP_MJ_READ";
		break;
	case 4:
		funciton_name = "IRP_MJ_WRITE";
		break;
	case 5:
		funciton_name = "IRP_MJ_QUERY_INFORMATION";
		break;
	case 6:
		funciton_name = "IRP_MJ_SET_INFORMATION";
		break;
	case 7:
		funciton_name = "IRP_MJ_QUERY_EA";
		break;
	case 8:
		funciton_name = "IRP_MJ_SET_EA";
		break;
	case 9:
		funciton_name = "IRP_MJ_FLUSH_BUFFERS";
		break;
	case 10:
		funciton_name = "IRP_MJ_QUERY_VOLUME_INFORMATION";
		break;
	case 11:
		funciton_name = "IRP_MJ_SET_VOLUME_INFORMATION";
		break;
	case 12:
		funciton_name = "IRP_MJ_DIRECTORY_CONTROL";
		break;
	case 13:
		funciton_name = "IRP_MJ_FILE_SYSTEM_CONTROL";
		break;
	case 14:
		funciton_name = "IRP_MJ_DEVICE_CONTROL";
		break;
	case 15:
		funciton_name = "IRP_MJ_INTERNAL_DEVICE_CONTROL";
		break;
	case 16:
		funciton_name = "IRP_MJ_SHUTDOWN";
		break;
	case 17:
		funciton_name = "IRP_MJ_LOCK_CONTROL";
		break;
	case 18:
		funciton_name = "IRP_MJ_CLEANUP";
		break;
	case 19:
		funciton_name = "IRP_MJ_CREATE_MAILSLOT";
		break;
	case 20:
		funciton_name = "IRP_MJ_QUERY_SECURITY";
		break;
	case 21:
		funciton_name = "IRP_MJ_SET_SECURITY";
		break;
	case 22:
		funciton_name = "IRP_MJ_POWER";
		break;
	case 23:
		funciton_name = "IRP_MJ_SYSTEM_CONTROL";
		break;
	case 24:
		funciton_name = "IRP_MJ_DEVICE_CHANGE";
		break;
	case 25:
		funciton_name = "IRP_MJ_QUERY_QUOTA";
		break;
	case 26:
		funciton_name = "IRP_MJ_SET_QUOTA";
		break;
	case 27:
		funciton_name = "IRP_MJ_PNP";
		break;
	case 28:
		funciton_name = "IRP_MJ_PNP_POWER";
		break;
	}
	return funciton_name;
}

#define TEST_GetIOCTLFunction CTL_CODE(FILE_DEVICE_UNKNOWN,0x7116,METHOD_BUFFERED ,FILE_ANY_ACCESS)
bool IOCTLScanner::GetIOCTLFunction(ULONG64 Addr, KernelModules& _KernelModules, std::string name)
{
	_Model->removeRows(0, _Model->rowCount());

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	DWORD dwRet = 0;
	IOCTLS* temp_list = (IOCTLS*)new char[(0x1b + 1) * sizeof(IOCTLS)];
	if (!temp_list)
	{
		CloseHandle(m_hDevice);
		return false;
	}
	DeviceIoControl(m_hDevice, TEST_GetIOCTLFunction, &Addr, 8, temp_list, sizeof(IOCTLS) * (0x1b + 1), &dwRet, NULL);
	if (dwRet)
	{
		for (int i = 0; i < 0x1b + 1; i++)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), GetFunctionName(temp_list[i].Index).data());
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << temp_list[i].Addr;
			_Model->setData(_Model->index(i, 2), ret2.str().data());
			_Model->setData(_Model->index(i, 3), "");

			std::string module_name;
			bool found = false;
			for (auto x : _KernelModules._KernelModuleListR0)
			{
				if (x.Check == 1 || x.Check == 2)
				{
					module_name = W_TO_C((WCHAR*)x.Name);
				}
				else
				{
					module_name = (char*)x.Name;
				}
				if (temp_list[i].Addr >= (ULONG64)x.Addr &&
					temp_list[i].Addr < (ULONG64)x.Addr + (ULONG64)x.Size)
				{
					if (module_name == "ntoskrnl.exe" && !temp_list[i].Check)
					{
						found = true;
					}
					else if (name == module_name)
					{
						found = true;
					}
					module_name = module_name + "+";
					std::ostringstream ret;
					ret << std::hex << "0x" << temp_list[i].Addr - x.Addr;
					module_name = module_name + ret.str();
					_Model->setData(_Model->index(i, 3), module_name.data());
					break;
				}
			}
			if (!found)
			{
				_Model->item(i, 0)->setBackground(QColor(Qt::red));
				_Model->item(i, 1)->setBackground(QColor(Qt::red));
				_Model->item(i, 2)->setBackground(QColor(Qt::red));
				_Model->item(i, 3)->setBackground(QColor(Qt::red));
			}

			/*if (temp_list[i].Check)
			{
				_Model->item(i, 0)->setBackground(QColor(Qt::red));
				_Model->item(i, 1)->setBackground(QColor(Qt::red));
				_Model->item(i, 2)->setBackground(QColor(Qt::red));
				_Model->item(i, 3)->setBackground(QColor(Qt::red));
			}*/
		}
	}


	delete temp_list;
	CloseHandle(m_hDevice);
	return dwRet;
}

bool IOCTLScanner::QueryIOCTLHook(ULONG64 Addr, KernelModules& _KernelModules, std::string name)
{
	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return false;
	}

	bool ret_found = false;
	DWORD dwRet = 0;
	IOCTLS* temp_list = (IOCTLS*)new char[(0x1b + 1) * sizeof(IOCTLS)];
	if (!temp_list)
	{
		CloseHandle(m_hDevice);
		return false;
	}
	DeviceIoControl(m_hDevice, TEST_GetIOCTLFunction, &Addr, 8, temp_list, sizeof(IOCTLS) * (0x1b + 1), &dwRet, NULL);
	if (dwRet)
	{
		for (int i = 0; i < 0x1b + 1; i++)
		{
			bool found = false;
			std::string module_name;
			for (auto x : _KernelModules._KernelModuleListR0)
			{
				if (x.Check == 1 || x.Check == 2)
				{
					module_name = W_TO_C((WCHAR*)x.Name);
				}
				else
				{
					module_name = (char*)x.Name;
				}
				if (temp_list[i].Addr >= (ULONG64)x.Addr &&
					temp_list[i].Addr < (ULONG64)x.Addr + (ULONG64)x.Size)
				{
					if (module_name == "ntoskrnl.exe" && !temp_list[i].Check)
					{
						found = true;
					}
					else if (name == module_name)
					{
						found = true;
					}
					break;
				}
			}
			if (!found)
			{
				ret_found = true;
				break;
			}
		}
	}


	delete temp_list;
	CloseHandle(m_hDevice);
	return ret_found;
}