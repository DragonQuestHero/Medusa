#include "UserMemoryListKernel.h"
#include "ntdll.h"

#include "Modules.h"

#include <iostream>
#include <sstream>




UserMemoryListKernel::UserMemoryListKernel(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(12);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Size");
	_Model->setHeaderData(3, Qt::Horizontal, u8"Pte");
	_Model->setHeaderData(4, Qt::Horizontal, u8"PFN");
	_Model->setHeaderData(5, Qt::Horizontal, u8"Wirte");
	_Model->setHeaderData(6, Qt::Horizontal, u8"Execute");
	_Model->setHeaderData(7, Qt::Horizontal, u8"LargePage");
	_Model->setHeaderData(8, Qt::Horizontal, u8"CopyOnWrite");
	_Model->setHeaderData(9, Qt::Horizontal, u8"Prototype");
	_Model->setHeaderData(10, Qt::Horizontal, u8"Valid");
	_Model->setHeaderData(11, Qt::Horizontal, u8"SoftwareWsIndex");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 130);
	ui.tableView->setColumnWidth(2, 90);
	ui.tableView->setColumnWidth(3, 180);
	ui.tableView->setColumnWidth(4, 100);
	ui.tableView->setColumnWidth(5, 50);
	ui.tableView->setColumnWidth(6, 50);
	ui.tableView->setColumnWidth(7, 80);
	ui.tableView->setColumnWidth(8, 100);
	ui.tableView->setColumnWidth(9, 80);
	ui.tableView->setColumnWidth(10, 50);
	ui.tableView->setColumnWidth(11, 120);


	_TableView_Action_Check.setText("MemoryView");
	ui.tableView->addAction(&_TableView_Action_Check);
	connect(&_TableView_Action_Check, SIGNAL(triggered(bool)), SLOT(MemoryView(bool)));
}

bool UserMemoryListKernel::ShowUserMemoryListR0(ULONG64 PID)
{
	_Model->removeRows(0, _Model->rowCount());
	std::vector<UserMemoryListStructCR3> temp_vector = GetUserMemoryListR0(PID);
	std::vector<UserMemoryListStructCR3> temp_vector2;

	if (temp_vector.empty()) {
		return true; // 如果原始向量为空，直接返回
	}

	UserMemoryListStructCR3 current = temp_vector[0];

	for (size_t i = 1; i < temp_vector.size(); ) {
		UserMemoryListStructCR3 next = temp_vector[i];

		// 检查条件：Size 相同，Addr 连续，PteX64 除了 page_frame_number 外相同
		if (current.Addr + current.Size == next.Addr) {

			// 满足条件，合并
			current.Size += next.Size; // 更新大小
			i++; // 移动到下一个元素
		}
		else {
			// 将合并后的结果添加到新向量中
			temp_vector2.push_back(current);
			current = next; // 更新 current 为下一个未合并的元素
			i++; // 也要移动到下一个元素
		}
	}
	// 添加最后一个元素
	temp_vector2.push_back(current);

	// 更新模型
	for (int i = 0; i < temp_vector2.size(); i++)
	{
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		std::ostringstream ret1;
		ret1 << std::hex << "0x" << temp_vector2[i].Addr;
		_Model->setData(_Model->index(i, 1), ret1.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << temp_vector2[i].Size;
		_Model->setData(_Model->index(i, 2), ret2.str().data());
		std::ostringstream ret3;
		ret3 << std::hex << "0x" << temp_vector2[i].PteX64.value;
		_Model->setData(_Model->index(i, 3), ret3.str().data());
		std::ostringstream ret4;
		ret4 << std::hex << "0x" << temp_vector2[i].PteX64.Bits.page_frame_number;
		_Model->setData(_Model->index(i, 4), ret4.str().data());
		_Model->setData(_Model->index(i, 5), temp_vector2[i].PteX64.Bits.write);
		_Model->setData(_Model->index(i, 6), temp_vector2[i].PteX64.Bits.no_execute);
		_Model->setData(_Model->index(i, 7), temp_vector2[i].PteX64.Bits.large_page);
		_Model->setData(_Model->index(i, 8), temp_vector2[i].PteX64.Bits.copy_on_write);
		_Model->setData(_Model->index(i, 9), temp_vector2[i].PteX64.Bits.prototype);
		_Model->setData(_Model->index(i, 10), temp_vector2[i].PteX64.Bits.valid);
		std::ostringstream ret5;
		ret5 << std::hex << "0x" << temp_vector2[i].PteX64.Bits.software_ws_index;
		_Model->setData(_Model->index(i, 11), ret5.str().data());
	}

	return true;
}

bool UserMemoryListKernel::ShowUserMemoryListR0_2(ULONG64 PID)
{
	_Model->removeRows(0, _Model->rowCount());
	std::vector<UserMemoryListStructCR3> temp_vector = GetUserMemoryListR0(PID);
	std::vector<UserMemoryListStructCR3> temp_vector2;

	if (temp_vector.empty()) {
		return true; // 如果原始向量为空，直接返回
	}

	UserMemoryListStructCR3 current = temp_vector[0];

	for (size_t i = 1; i < temp_vector.size(); ) {
		UserMemoryListStructCR3 next = temp_vector[i];

		// 检查条件：Size 相同，Addr 连续，PteX64 除了 page_frame_number 外相同
		if (
			current.Addr + current.Size == next.Addr && // 地址连续
			current.PteX64.Bits.valid == next.PteX64.Bits.valid &&
			current.PteX64.Bits.write == next.PteX64.Bits.write &&
			current.PteX64.Bits.owner == next.PteX64.Bits.owner &&
			current.PteX64.Bits.write_through == next.PteX64.Bits.write_through &&
			current.PteX64.Bits.cache_disable == next.PteX64.Bits.cache_disable &&
			//current.PteX64.Bits.accessed == next.PteX64.Bits.accessed &&
			//current.PteX64.Bits.dirty == next.PteX64.Bits.dirty &&
			current.PteX64.Bits.large_page == next.PteX64.Bits.large_page &&
			current.PteX64.Bits.global == next.PteX64.Bits.global &&
			current.PteX64.Bits.copy_on_write == next.PteX64.Bits.copy_on_write &&
			current.PteX64.Bits.prototype == next.PteX64.Bits.prototype &&
			current.PteX64.Bits.reserved0 == next.PteX64.Bits.reserved0 &&
			current.PteX64.Bits.reserved1 == next.PteX64.Bits.reserved1 &&
			current.PteX64.Bits.software_ws_index == next.PteX64.Bits.software_ws_index &&
			current.PteX64.Bits.no_execute == next.PteX64.Bits.no_execute) {

			// 满足条件，合并
			current.Size += next.Size; // 更新大小
			i++; // 移动到下一个元素
		}
		else {
			// 将合并后的结果添加到新向量中
			temp_vector2.push_back(current);
			current = next; // 更新 current 为下一个未合并的元素
			i++; // 也要移动到下一个元素
		}
	}
	// 添加最后一个元素
	temp_vector2.push_back(current);

	// 更新模型
	for (int i = 0; i < temp_vector2.size(); i++)
	{
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		std::ostringstream ret1;
		ret1 << std::hex << "0x" << temp_vector2[i].Addr;
		_Model->setData(_Model->index(i, 1), ret1.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << temp_vector2[i].Size;
		_Model->setData(_Model->index(i, 2), ret2.str().data());
		std::ostringstream ret3;
		ret3 << std::hex << "0x" << temp_vector2[i].PteX64.value;
		_Model->setData(_Model->index(i, 3), ret3.str().data());
		std::ostringstream ret4;
		ret4 << std::hex << "0x" << temp_vector2[i].PteX64.Bits.page_frame_number;
		_Model->setData(_Model->index(i, 4), ret4.str().data());
		_Model->setData(_Model->index(i, 5), temp_vector2[i].PteX64.Bits.write);
		_Model->setData(_Model->index(i, 6), temp_vector2[i].PteX64.Bits.no_execute);
		_Model->setData(_Model->index(i, 7), temp_vector2[i].PteX64.Bits.large_page);
		_Model->setData(_Model->index(i, 8), temp_vector2[i].PteX64.Bits.copy_on_write);
		_Model->setData(_Model->index(i, 9), temp_vector2[i].PteX64.Bits.prototype);
		_Model->setData(_Model->index(i, 10), temp_vector2[i].PteX64.Bits.valid);
		std::ostringstream ret5;
		ret5 << std::hex << "0x" << temp_vector2[i].PteX64.Bits.software_ws_index;
		_Model->setData(_Model->index(i, 11), ret5.str().data());
	}

	return true;
}

bool UserMemoryListKernel::ShowUserMemoryListR0ALL(ULONG64 PID)
{
	_Model->removeRows(0, _Model->rowCount());
	std::vector<UserMemoryListStructCR3> temp_vector = GetUserMemoryListR0(PID);
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
		std::ostringstream ret3;
		ret3 << std::hex << "0x" << x.PteX64.value;
		_Model->setData(_Model->index(i, 3), ret3.str().data());
		std::ostringstream ret4;
		ret4 << std::hex << "0x" << x.PteX64.Bits.page_frame_number;
		_Model->setData(_Model->index(i, 4), ret4.str().data());
		_Model->setData(_Model->index(i, 5), x.PteX64.Bits.write);
		_Model->setData(_Model->index(i, 6), x.PteX64.Bits.no_execute);
		_Model->setData(_Model->index(i, 7), x.PteX64.Bits.large_page);
		_Model->setData(_Model->index(i, 8), x.PteX64.Bits.copy_on_write);
		_Model->setData(_Model->index(i, 9), x.PteX64.Bits.prototype);
		_Model->setData(_Model->index(i, 10), x.PteX64.Bits.valid);
		std::ostringstream ret5;
		ret5 << std::hex << "0x" << x.PteX64.Bits.software_ws_index;
		_Model->setData(_Model->index(i, 11), ret5.str().data());
		i++;
	}
	return true;
}

#define TEST_GetUserMemoryFromCR3 CTL_CODE(FILE_DEVICE_UNKNOWN,0x7127,METHOD_BUFFERED ,FILE_ANY_ACCESS)
#define TEST_GetUserMemoryFromCR3Number CTL_CODE(FILE_DEVICE_UNKNOWN,0x7128,METHOD_BUFFERED ,FILE_ANY_ACCESS)
std::vector<UserMemoryListStructCR3> UserMemoryListKernel::GetUserMemoryListR0(ULONG64 PID)
{
	_PID = PID;
	std::vector<UserMemoryListStructCR3> temp_vector;

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return temp_vector;
	}

	do
	{
		DWORD process_number = 0;
		DeviceIoControl(m_hDevice, TEST_GetUserMemoryFromCR3Number, &PID, 8, 0, 0, &process_number, NULL);
		if (!process_number)
		{
			break;
		}

		DWORD dwRet = 0;
		UserMemoryListStructCR3* temp_list = (UserMemoryListStructCR3*)new char[process_number * sizeof(UserMemoryListStructCR3)];
		if (!temp_list)
		{
			break;
		}

		DeviceIoControl(m_hDevice, TEST_GetUserMemoryFromCR3, 0, 0, temp_list, sizeof(UserMemoryListStructCR3) * process_number, &dwRet, NULL);
		if (dwRet)
		{
			for (int i = 0; i < process_number; i++)
			{
				temp_vector.push_back(temp_list[i]);
			}
		}
		delete temp_list;

		CloseHandle(m_hDevice);
		return temp_vector;
	} while (false);





	CloseHandle(m_hDevice);
	return temp_vector;
}

void UserMemoryListKernel::MemoryView(bool)
{
	_UserMemory.setWindowTitle("ProcessID:" + QString::number(_PID));
	_UserMemory.ui.tabWidget->setCurrentIndex(1);
	_UserMemory.PID = _PID;
	_UserMemory.ui.label->setText("ProcessID:" + QString::number(_PID) + "    CR3:0x0");
	_UserMemory.ui.lineEdit->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 1).data().toString());
	//_UserMemory->ui.lineEdit_2->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString());
	_UserMemory.ui.lineEdit_2->setText("0x1000");
	if (ui.tableView->model()->index(ui.tableView->currentIndex().row(), 5).data().toString().toStdString().find("MEM_FREE") != std::string::npos)
	{
		_UserMemory.ui.lineEdit_2->setText("0x0");
	}
	_UserMemory.QueryMemory();
	_UserMemory.show();
}


