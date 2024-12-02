#include "UserMemoryList.h"
#include "ntdll.h"

#include "Modules.h"

#include <iostream>
#include <sstream>




UserMemoryList::UserMemoryList(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(7);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Size");
	_Model->setHeaderData(3, Qt::Horizontal, u8"Type");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Protection");
	_Model->setHeaderData(5, Qt::Horizontal, u8"State");
	_Model->setHeaderData(6, Qt::Horizontal, u8"Name");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 150);
	ui.tableView->setColumnWidth(2, 100);
	ui.tableView->setColumnWidth(3, 100);
	ui.tableView->setColumnWidth(4, 300);
	ui.tableView->setColumnWidth(5, 150);
	ui.tableView->setColumnWidth(6, 150);


	_TableView_Action_Check.setText("MemoryView");
	ui.tableView->addAction(&_TableView_Action_Check);
	connect(&_TableView_Action_Check, SIGNAL(triggered(bool)), SLOT(MemoryView(bool)));
}

std::string ProtectToString(DWORD protect) 
{
	std::string result;

	if (protect & PAGE_NOACCESS) {
		result += "PAGE_NOACCESS|";
	}
	if (protect & PAGE_READONLY) {
		result += "PAGE_READONLY|";
	}
	if (protect & PAGE_READWRITE) {
		result += "PAGE_READWRITE|";
	}
	if (protect & PAGE_WRITECOPY) {
		result += "PAGE_WRITECOPY|";
	}
	if (protect & PAGE_EXECUTE) {
		result += "PAGE_EXECUTE|";
	}
	if (protect & PAGE_EXECUTE_READ) {
		result += "PAGE_EXECUTE_READ|";
	}
	if (protect & PAGE_EXECUTE_READWRITE) {
		result += "PAGE_EXECUTE_READWRITE|";
	}
	if (protect & PAGE_EXECUTE_WRITECOPY) {
		result += "PAGE_EXECUTE_WRITECOPY|";
	}
	if (protect & PAGE_GUARD) {
		result += "PAGE_GUARD|";
	}
	if (protect & PAGE_NOCACHE) {
		result += "PAGE_NOCACHE|";
	}
	if (protect & PAGE_WRITECOMBINE) {
		result += "PAGE_WRITECOMBINE|";
	}
	if (protect & PAGE_GRAPHICS_NOACCESS) {
		result += "PAGE_GRAPHICS_NOACCESS|";
	}
	if (protect & PAGE_GRAPHICS_READONLY) {
		result += "PAGE_GRAPHICS_READONLY|";
	}
	if (protect & PAGE_GRAPHICS_READWRITE) {
		result += "PAGE_GRAPHICS_READWRITE|";
	}
	if (protect & PAGE_GRAPHICS_EXECUTE) {
		result += "PAGE_GRAPHICS_EXECUTE|";
	}
	if (protect & PAGE_GRAPHICS_EXECUTE_READ) {
		result += "PAGE_GRAPHICS_EXECUTE_READ|";
	}
	if (protect & PAGE_GRAPHICS_EXECUTE_READWRITE) {
		result += "PAGE_GRAPHICS_EXECUTE_READWRITE|";
	}
	if (protect & PAGE_GRAPHICS_COHERENT) {
		result += "PAGE_GRAPHICS_COHERENT|";
	}
	if (protect & PAGE_GRAPHICS_NOCACHE) {
		result += "PAGE_GRAPHICS_NOCACHE|";
	}
	if (protect & PAGE_ENCLAVE_THREAD_CONTROL) {
		result += "PAGE_ENCLAVE_THREAD_CONTROL|";
	}
	if (protect & PAGE_REVERT_TO_FILE_MAP) {
		result += "PAGE_REVERT_TO_FILE_MAP|";
	}
	if (protect & PAGE_TARGETS_NO_UPDATE) {
		result += "PAGE_TARGETS_NO_UPDATE|";
	}
	if (protect & PAGE_TARGETS_INVALID) {
		result += "PAGE_TARGETS_INVALID|";
	}
	if (protect & PAGE_ENCLAVE_UNVALIDATED) {
		result += "PAGE_ENCLAVE_UNVALIDATED|";
	}
	if (protect & PAGE_ENCLAVE_MASK) {
		result += "PAGE_ENCLAVE_MASK|";
	}
	if (protect & PAGE_ENCLAVE_DECOMMIT) {
		result += "PAGE_ENCLAVE_DECOMMIT|";
	}
	if (protect & PAGE_ENCLAVE_SS_FIRST) {
		result += "PAGE_ENCLAVE_SS_FIRST|";
	}
	if (protect & PAGE_ENCLAVE_SS_REST) {
		result += "PAGE_ENCLAVE_SS_REST|";
	}

	// 去掉最后一个 '|'
	if (!result.empty()) {
		result.pop_back();
	}

	return result;
}
std::string StateToString(DWORD state)
{
	std::string result;

	if (state & MEM_COMMIT) {
		result += "MEM_COMMIT|";
	}
	if (state & MEM_RESERVE) {
		result += "MEM_RESERVE|";
	}
	if (state & MEM_REPLACE_PLACEHOLDER) {
		result += "MEM_REPLACE_PLACEHOLDER|";
	}
	if (state & MEM_RESERVE_PLACEHOLDER) {
		result += "MEM_RESERVE_PLACEHOLDER|";
	}
	if (state & MEM_RESET) {
		result += "MEM_RESET|";
	}
	if (state & MEM_TOP_DOWN) {
		result += "MEM_TOP_DOWN|";
	}
	if (state & MEM_WRITE_WATCH) {
		result += "MEM_WRITE_WATCH|";
	}
	if (state & MEM_PHYSICAL) {
		result += "MEM_PHYSICAL|";
	}
	if (state & MEM_ROTATE) {
		result += "MEM_ROTATE|";
	}
	if (state & MEM_DIFFERENT_IMAGE_BASE_OK) {
		result += "MEM_DIFFERENT_IMAGE_BASE_OK|";
	}
	if (state & MEM_RESET_UNDO) {
		result += "MEM_RESET_UNDO|";
	}
	if (state & MEM_LARGE_PAGES) {
		result += "MEM_LARGE_PAGES|";
	}
	if (state & MEM_4MB_PAGES) {
		result += "MEM_4MB_PAGES|";
	}
	if (state & MEM_64K_PAGES) {
		result += "MEM_64K_PAGES|";
	}
	if (state & MEM_UNMAP_WITH_TRANSIENT_BOOST) {
		result += "MEM_UNMAP_WITH_TRANSIENT_BOOST|";
	}
	if (state & MEM_COALESCE_PLACEHOLDERS) {
		result += "MEM_COALESCE_PLACEHOLDERS|";
	}
	if (state & MEM_PRESERVE_PLACEHOLDER) {
		result += "MEM_PRESERVE_PLACEHOLDER|";
	}
	if (state & MEM_DECOMMIT) {
		result += "MEM_DECOMMIT|";
	}
	if (state & MEM_RELEASE) {
		result += "MEM_RELEASE|";
	}
	if (state & MEM_FREE) {
		result += "MEM_FREE|";
	}

	// 去掉最后一个 '|'
	if (!result.empty()) {
		result.pop_back();
	}

	return result;
}
bool UserMemoryList::ShowUserMemoryListR3(ULONG64 PID,bool CheckHide)
{
	_Model->removeRows(0, _Model->rowCount());
	std::vector<UserMemoryListStruct> temp_vector;
	if (CheckHide)
	{
		temp_vector = GetUserMemoryListR3CheckHide(PID);
	}
	else
	{
		temp_vector = GetUserMemoryListR3(PID);
	}
	Modules _Modules;
	std::vector<MODULEENTRY32W> temp_vector_modules = _Modules.GetUserMoudleListR3(PID);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		std::ostringstream ret1;
		ret1 << std::hex << "0x" << x.Addr;
		_Model->setData(_Model->index(i, 1), ret1.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << x.Size;
		_Model->setData(_Model->index(i, 2), ret2.str().data());
		if (x.Type == 0x1000000)
		{
			_Model->setData(_Model->index(i, 3), "MEM_IMAGE");
		}
		else if (x.Type == 0x40000)
		{
			_Model->setData(_Model->index(i, 3), "MEM_MAPPED");
		}
		else if (x.Type == 0x20000)
		{
			_Model->setData(_Model->index(i, 3), "MEM_PRIVATE");
		}
		else
		{
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << (ULONG64)x.Type;
			_Model->setData(_Model->index(i, 3), ret2.str().data());
		}
		_Model->setData(_Model->index(i, 4), ProtectToString(x.Protect).data());
		_Model->setData(_Model->index(i, 5), StateToString(x.State).data());
		bool found = false;
		for (auto y : temp_vector_modules)
		{
			if ((ULONG64)y.modBaseAddr == x.Addr || ((x.Addr >= (ULONG64)y.modBaseAddr) &&
				(x.Addr < (ULONG64)(y.modBaseAddr) + y.modBaseSize)))
			{
				found = true;
				_Model->setData(_Model->index(i, 6), QString::fromWCharArray(y.szModule));
				break;
			}
		}
		if (x.Color == 1)
		{
			for (int z = 0; z < 6; z++)
			{
				_Model->item(i, z)->setBackground(QColor(Qt::red));
			}
			if (found)
			{
				_Model->item(i, 6)->setBackground(QColor(Qt::red));
			}
		}
		if (x.Color == 2)
		{
			for (int z = 0; z < 6; z++)
			{
				_Model->item(i, z)->setBackground(QColor(Qt::yellow));
			}
			if (found)
			{
				_Model->item(i, 6)->setBackground(QColor(Qt::yellow));
			}
		}
		i++;
	}
	return true;
}

std::vector<UserMemoryListStruct> UserMemoryList::GetUserMemoryListR3(ULONG64 PID)
{
	_PID = PID;
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
	std::vector<UserMemoryListStruct> temp_vector;
	MEMORY_BASIC_INFORMATION base_info;
	SIZE_T ReturnLength;
	void* base_addr = 0;
	while (NT_SUCCESS(
		NtQueryVirtualMemory(processHandle, base_addr, MemoryBasicInformation,
			&base_info, sizeof(MEMORY_BASIC_INFORMATION), &ReturnLength)))
	{
		if (base_info.RegionSize != 0)
		{
			UserMemoryListStruct temp_list;
			RtlZeroMemory(&temp_list, sizeof(UserMemoryListStruct));
			temp_list.Addr = (ULONG64)base_info.BaseAddress;
			temp_list.Size = base_info.RegionSize;
			temp_list.Type = base_info.Type;
			temp_list.Protect = base_info.Protect;
			temp_list.State = base_info.State;
			temp_list.Color = 0;//0 正常 1 红色无模块可执行 2 黄色可执行
			if (base_info.Protect & 0xf0)
			{
				temp_list.Color = 2;
			}
			temp_vector.push_back(temp_list);
		}
		base_addr = (void*)((ULONG64)base_addr + base_info.RegionSize);
	}
	return temp_vector;
}

std::vector<UserMemoryListStruct> UserMemoryList::GetUserMemoryListR3CheckHide(ULONG64 PID)
{
	_PID = PID;
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID);
	std::vector<UserMemoryListStruct> temp_vector;
	MEMORY_BASIC_INFORMATION base_info;
	SIZE_T ReturnLength;
	void* base_addr = nullptr;

	while (NT_SUCCESS(
		NtQueryVirtualMemory(processHandle, base_addr, MemoryBasicInformation,
			&base_info, sizeof(MEMORY_BASIC_INFORMATION), &ReturnLength)))
	{
		if (base_info.RegionSize != 0)
		{
			UserMemoryListStruct temp_list;
			RtlZeroMemory(&temp_list, sizeof(UserMemoryListStruct));
			temp_list.Addr = (ULONG64)base_info.BaseAddress;
			temp_list.Size = base_info.RegionSize;
			temp_list.Type = base_info.Type;
			temp_list.Protect = base_info.Protect;
			temp_list.State = base_info.State;
			temp_list.Color = 0;//0 正常 1 红色无模块可执行 2 黄色可执行
			if (base_info.Protect & 0xf0)
			{
				temp_list.Color = 2;
			}
			temp_vector.push_back(temp_list);
		}

		// 检查下一个内存区域
		void* next_addr = (void*)((ULONG64)base_addr + base_info.RegionSize);
		ULONG64 gap = (ULONG64)next_addr - ((ULONG64)base_info.BaseAddress + base_info.RegionSize);

		// 如果间隔大于一页，进行逐页验证
		if (gap >= PAGE_SIZE) // 4096字节 = 1页
		{
			for (SIZE_T offset = 0; offset < gap; offset += PAGE_SIZE) // 每次检查一页
			{
				void* check_addr = (void*)((ULONG64)base_info.BaseAddress + base_info.RegionSize + offset);
				if (!IsBadReadPtr(check_addr, 1)) // 检查地址是否可读
				{
					UserMemoryListStruct temp_list;
					RtlZeroMemory(&temp_list, sizeof(UserMemoryListStruct));
					temp_list.Addr = (ULONG64)base_info.BaseAddress;
					temp_list.Size = base_info.RegionSize;
					temp_list.Type = base_info.Type;
					temp_list.Protect = base_info.Protect;
					temp_list.State = base_info.State;
					temp_list.Color = 1;//0 正常 1 红色无模块可执行 2 黄色可执行
					temp_vector.push_back(temp_list);
				}
			}
		}

		base_addr = next_addr; // 移动到下一个区域
	}

	CloseHandle(processHandle); // 关闭进程句柄
	return temp_vector;
}

void UserMemoryList::MemoryView(bool)
{
	_UserMemory->setWindowTitle("ProcessID:" + QString::number(_PID));
	_UserMemory->ui.tabWidget->setCurrentIndex(1);
	_UserMemory->PID = _PID;
	_UserMemory->ui.label->setText("ProcessID:" + QString::number(_PID) + "    CR3:0x0");
	_UserMemory->ui.lineEdit->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 1).data().toString());
	//_UserMemory->ui.lineEdit_2->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString());
	_UserMemory->ui.lineEdit_2->setText("0x1000");
	if (ui.tableView->model()->index(ui.tableView->currentIndex().row(), 5).data().toString().toStdString().find("MEM_FREE") != std::string::npos)
	{
		_UserMemory->ui.lineEdit_2->setText("0x0");
	}
	_UserMemory->QueryMemory();
	_UserMemory->show();
}


