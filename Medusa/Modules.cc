#include "Modules.h"
#include "ntdll.h"


#include "libpeconv/peconv.h"

#include "Process.h"


Modules::Modules(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(6);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"Name");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(3, Qt::Horizontal, u8"Size");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Path");
	_Model->setHeaderData(5, Qt::Horizontal, u8"Desciption");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 150);
	ui.tableView->setColumnWidth(2, 150);
	ui.tableView->setColumnWidth(3, 150);
	ui.tableView->setColumnWidth(4, 150);
	ui.tableView->setColumnWidth(5, 400);

	setWindowTitle("Modules");

	_TableView_Action_Dump.setText("Dump");
	_TableView_Action_DumpToFile.setText("DumpToFile");
	_TableView_Action_ViewExportFunc.setText("ViewExportFunc");
	ui.tableView->addAction(&_TableView_Action_Dump);
	ui.tableView->addAction(&_TableView_Action_DumpToFile);
	ui.tableView->addAction(&_TableView_Action_ViewExportFunc);
	connect(&_TableView_Action_Dump, SIGNAL(triggered(bool)), SLOT(Dump(bool)));
	connect(&_TableView_Action_DumpToFile, SIGNAL(triggered(bool)), SLOT(DumpToFile(bool)));
	connect(&_TableView_Action_ViewExportFunc, SIGNAL(triggered(bool)), SLOT(ViewExportFunc(bool)));

	_TableView_Action_UserMemory.setText("MemoryView");
	ui.tableView->addAction(&_TableView_Action_UserMemory);
	connect(&_TableView_Action_UserMemory, SIGNAL(triggered(bool)), SLOT(MemoryView(bool)));
}

void Modules::Dump(bool)
{
	auto proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, _PID);
	if (!proc)
	{
		return;
	}
	std::string addr_str = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString().toStdString();
	ULONG64 addr = strtoull(addr_str.data(), 0, 16);
	std::string size_str = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 3).data().toString().toStdString();
	ULONG64 size = strtoull(size_str.data(), 0, 16);

	char* temp_memory = new char[size];
	if (temp_memory)
	{
		SIZE_T temp_size = 0;
		if (ReadProcessMemory(proc, (char*)addr, temp_memory, size, &temp_size))
		{
			std::fstream temp_file(addr_str, std::ios::out | std::ios::binary);
			if (temp_file.is_open())
			{
				temp_file << std::string(temp_memory, size);
				temp_file.close();
				QMessageBox::information(this, "Ret", "susscss");
			}
		}
		else
		{
		}
	}
	else
	{
		QMessageBox::information(this, "Ret", "alloc memory error");
	}
	CloseHandle(proc);
}

void Modules::DumpToFile(bool)
{
	auto proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, _PID);
	if (!proc)
	{
		return;
	}
	std::string addr_str = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString().toStdString();
	ULONG64 addr = strtoull(addr_str.data(), 0, 16);
	std::string size_str = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 3).data().toString().toStdString();
	ULONG64 size = strtoull(size_str.data(), 0, 16);

	char* temp_memory = new char[size];
	if (temp_memory)
	{
		SIZE_T temp_size = 0;
		if (ReadProcessMemory(proc, (char*)addr, temp_memory, size, &temp_size))
		{
			peconv::t_pe_dump_mode dump_mode = peconv::PE_DUMP_UNMAP;
			if (peconv::dump_pe(C_TO_W(addr_str + ".DLL").data(), (BYTE*)temp_memory, size, 0, dump_mode))
			{
				QMessageBox::information(this, "Ret", "susscss");
			}
		}
		else
		{
		}
	}
	else
	{
		QMessageBox::information(this, "Ret", "alloc memory error");
	}
	CloseHandle(proc);
}

void Modules::ViewExportFunc(bool)
{
	std::string file_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 4).data().toString().toStdString();
	std::string addr_str = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString().toStdString();
	addr_str.erase(0, 2);
	ULONG64 addr = strtoull(addr_str.data(), 0, 16);

	_ModuleExportFunc._PID = _PID;
	_ModuleExportFunc._Previous = false;
	_ModuleExportFunc.GetExportFunc(addr, file_name);
	_ModuleExportFunc.show();
}

void Modules::MemoryView(bool)
{
	_UserMemory.setWindowTitle("ProcessID:" + QString::number(_PID));
	_UserMemory.ui.tabWidget->setCurrentIndex(1);
	_UserMemory.PID = _PID;
	_UserMemory.ui.label->setText("ProcessID:" + QString::number(_PID) + "    CR3:0x0");
	_UserMemory.ui.lineEdit->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString());
	_UserMemory.ui.lineEdit_2->setText("0x1000");
	_UserMemory.QueryMemory();
	_UserMemory.show();
}

bool Modules::ShowUserMoudleList(ULONG64 PID, bool kernel_mode)
{
	_PID = PID;
	if (kernel_mode)
	{
		_Model->removeRows(0, _Model->rowCount());
		std::vector<MODULEENTRY32W> temp_vector = GetUserMoudleListR3(PID);
		std::vector<UserModule> temp_vector2 = GetUserMoudleListR0(PID);
		int i = 0;
		for (auto x : temp_vector2)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), QString::fromWCharArray(x.Name));
			std::ostringstream ret;
			ret << std::hex << "0x" << (ULONG64)x.Addr;
			_Model->setData(_Model->index(i, 2), ret.str().data());
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << x.Size;
			_Model->setData(_Model->index(i, 3), ret2.str().data());
			_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.Path));
			std::wstring retStr;
			Process _Process;
			if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
			{
				_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
			}
			else
			{
				_Model->setData(_Model->index(i, 5), "");
			}
			bool found = false;
			for (auto y : temp_vector)
			{
				if (x.Addr == (ULONG64)y.modBaseAddr)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				_Model->item(i, 0)->setBackground(QColor(Qt::red));
				_Model->item(i, 1)->setBackground(QColor(Qt::red));
				_Model->item(i, 2)->setBackground(QColor(Qt::red));
				_Model->item(i, 3)->setBackground(QColor(Qt::red));
				_Model->item(i, 4)->setBackground(QColor(Qt::red));
				_Model->item(i, 5)->setBackground(QColor(Qt::red));
			}
			i++;
		}
	}
	else
	{
		_Model->removeRows(0, _Model->rowCount());
		std::vector<MODULEENTRY32W> temp_vector =
			GetUserMoudleListR3(PID);
		int i = 0;
		for (auto x : temp_vector)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), QString::fromWCharArray(x.szModule));
			std::ostringstream ret;
			ret << std::hex << "0x" << (ULONG64)x.modBaseAddr;
			_Model->setData(_Model->index(i, 2), ret.str().data());
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << x.modBaseSize;
			_Model->setData(_Model->index(i, 3), ret2.str().data());
			_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.szExePath));
			std::wstring retStr;
			Process _Process;
			if (_Process.QueryValue(L"FileDescription", x.szExePath, retStr))
			{
				_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
			}
			i++;
		}
	}
}

std::vector<MODULEENTRY32W> Modules::GetUserMoudleListR3(ULONG64 PID)
{
	_PID = PID;
	std::vector<MODULEENTRY32W> temp_vector;

	ULONG Flag = TH32CS_SNAPMODULE;
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
	if (processHandle != NULL) {
		BOOL isWow64 = FALSE;
		if (IsWow64Process(processHandle, &isWow64)) {
			if (isWow64) {
				Flag = TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32;
			}
			else {
				Flag = TH32CS_SNAPMODULE;
			}
		}
	}

	
	

	HANDLE        hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };

	hModuleSnap = CreateToolhelp32Snapshot(Flag, PID);
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		int a = GetLastError();
		return temp_vector;
	}

	if (!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);
		return temp_vector;
	}

	do {
		temp_vector.push_back(me32);

	} while (Module32Next(hModuleSnap, &me32));
	CloseHandle(hModuleSnap);
	return temp_vector;
}

#define TEST_GetALLUserModule CTL_CODE(FILE_DEVICE_UNKNOWN,0x7104,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetALLUserModuleNumber CTL_CODE(FILE_DEVICE_UNKNOWN,0x7105,METHOD_BUFFERED ,FILE_ANY_ACCESS)
std::vector<UserModule> Modules::GetUserMoudleListR0(ULONG64 PID)
{
	_PID = PID;
	std::vector<UserModule> temp_vector;

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return temp_vector;
	}
	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetALLUserModuleNumber, &PID, 8, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		UserModule* temp_list = (UserModule*)new char[process_number * sizeof(UserModule)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetALLUserModule, &PID, 8, temp_list, sizeof(UserModule) * process_number, &dwRet, NULL);
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

std::vector<UserModule> Modules::R3ModuleScanner(ULONG64 PID,HANDLE handle)
{
	_PID = PID;
	std::vector<UserModule> temp_vector;
	MEMORY_BASIC_INFORMATION base_info;
	SIZE_T ReturnLength;
	void* base_addr = 0;
	while (NT_SUCCESS(
		NtQueryVirtualMemory(handle, base_addr, MemoryBasicInformation,
			&base_info, sizeof(MEMORY_BASIC_INFORMATION), &ReturnLength)))
	{
		bool found = false;
		if (base_info.Protect & 0xf0 && base_info.Type != MEM_IMAGE && base_info.RegionSize > PAGE_SIZE)
		{
			for (int i = 0; i < base_info.RegionSize; i++)
			{
				USHORT magic = 0;
				SIZE_T size = 0;
				if (ReadProcessMemory(handle, (char*)base_addr+i, &magic, 2, &size))
				{
					if (magic == 0x5A4D && base_info.RegionSize - i > PAGE_SIZE)//除非情况很极端
					{
						void* memory_p = new char[PAGE_SIZE];
						if (ReadProcessMemory(handle, (char*)base_addr+i, memory_p, PAGE_SIZE, &size))
						{
							PIMAGE_NT_HEADERS nt_header = RtlImageNtHeader((void*)memory_p);
							if (nt_header)
							{
								UserModule temp_list;
								RtlZeroMemory(&temp_list, sizeof(UserModule));
								temp_list.Addr = (ULONG64)((char*)base_addr + i);
								temp_list.Size = nt_header->OptionalHeader.SizeOfImage;
								temp_vector.push_back(temp_list);
								found = true;
								delete memory_p;
								break;
							}
						}
						delete memory_p;
					}
				}
			}
		}
		if (base_info.Protect & 0xf0 && base_info.Type != MEM_IMAGE && !found)
		{
			UserModule temp_list;
			RtlZeroMemory(&temp_list, sizeof(UserModule));
			temp_list.Addr = (ULONG64)((char*)base_addr);
			temp_list.Size = base_info.RegionSize;
			RtlCopyMemory(temp_list.Name, L"shellcode", 20);
			temp_vector.push_back(temp_list);
		}
		base_addr = (void*)((ULONG64)base_addr + base_info.RegionSize);
	}
	return temp_vector;
}