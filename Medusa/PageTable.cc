#include "PageTable.h"



PageTable::PageTable(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	SetUI(ui.tableView_PXE, _PXE_Model);
	SetUI(ui.tableView_PPE, _PPE_Model);
	SetUI(ui.tableView_PDE, _PDE_Model);
	SetUI(ui.tableView_PTE, _PTE_Model);

	_TableView_Action_MemoryView.setText("ViewPhysicalMemoryCR3");
	ui.label_CR3->addAction(&_TableView_Action_MemoryView);
	connect(&_TableView_Action_MemoryView, SIGNAL(triggered(bool)), SLOT(MemoryView(bool)));

	connect(ui.pushButton, &QPushButton::clicked, this, &PageTable::ReadPage);
}

void PageTable::SetAddrOffset(ULONG64 Addr)
{
	ADDRESS_STRUCTURE temp_ADDRESS_STRUCTURE = { 0 };
	temp_ADDRESS_STRUCTURE.BitAddress = Addr;

	std::ostringstream ret1;
	ret1 << std::hex << "0x" << temp_ADDRESS_STRUCTURE.Bits.PML4Index;
	ui.lineEdit_pxe_Index->setText(ret1.str().data());
	ret1.str("");
	ret1.clear();

	ret1 << std::hex << "0x" << temp_ADDRESS_STRUCTURE.Bits.PDPTIndex;
	ui.lineEdit_ppe_Index->setText(ret1.str().data());
	ret1.str("");
	ret1.clear();

	ret1 << std::hex << "0x" << temp_ADDRESS_STRUCTURE.Bits.PDIndex;
	ui.lineEdit_pde_Index->setText(ret1.str().data());
	ret1.str("");
	ret1.clear();

	ret1 << std::hex << "0x" << temp_ADDRESS_STRUCTURE.Bits.PEIndex;
	ui.lineEdit_pte_Index->setText(ret1.str().data());
	ret1.str("");
	ret1.clear();

	ret1 << std::hex << "0x" << temp_ADDRESS_STRUCTURE.Bits.Offset;
	ui.lineEdit_offset->setText(ret1.str().data());
	ret1.str("");
	ret1.clear();

	ret1 << std::hex << "0x" << temp_ADDRESS_STRUCTURE.Bits_LargePage.LargeOffset;
	ui.lineEdit_offset_largepage->setText(ret1.str().data());
	ret1.str("");
	ret1.clear();
}

void PageTable::SetTableViewValue(QStandardItemModel* _Model, HardwarePteX64ForWindows &temp_pte)
{
	_Model->removeRows(0, _Model->rowCount());
	_Model->setVerticalHeaderItem(0, new QStandardItem);
	if (temp_pte.Bits.valid)
	{
		_Model->setData(_Model->index(0, 0), "Y");
		_Model->item(0, 0)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 0), "N");
		_Model->item(0, 0)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.write)
	{
		_Model->setData(_Model->index(0, 1), "Y");
		_Model->item(0, 1)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 1), "N");
		_Model->item(0, 1)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.owner)
	{
		_Model->setData(_Model->index(0, 2), "Y");
		_Model->item(0, 2)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 2), "N");
		_Model->item(0, 2)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.write_through)
	{
		_Model->setData(_Model->index(0, 3), "Y");
		_Model->item(0, 3)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 3), "N");
		_Model->item(0, 3)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.cache_disable)
	{
		_Model->setData(_Model->index(0, 4), "Y");
		_Model->item(0, 4)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 4), "N");
		_Model->item(0, 4)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.accessed)
	{
		_Model->setData(_Model->index(0, 5), "Y");
		_Model->item(0, 5)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 5), "N");
		_Model->item(0, 5)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.dirty)
	{
		_Model->setData(_Model->index(0, 6), "Y");
		_Model->item(0, 6)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 6), "N");
		_Model->item(0, 6)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.large_page)
	{
		_Model->setData(_Model->index(0, 7), "Y");
		_Model->item(0, 7)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 7), "N");
		_Model->item(0, 7)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.global)
	{
		_Model->setData(_Model->index(0, 8), "Y");
		_Model->item(0, 8)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 8), "N");
		_Model->item(0, 8)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.copy_on_write)
	{
		_Model->setData(_Model->index(0, 9), "Y");
		_Model->item(0, 9)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 9), "N");
		_Model->item(0, 9)->setBackground(QColor(Qt::red));
	}

	if (temp_pte.Bits.prototype)
	{
		_Model->setData(_Model->index(0, 10), "Y");
		_Model->item(0, 10)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 10), "N");
		_Model->item(0, 10)->setBackground(QColor(Qt::red));
	}

	_Model->setData(_Model->index(0, 11), temp_pte.Bits.reserved0);

	std::ostringstream ret1;
	ret1 << std::hex << "0x" << temp_pte.Bits.page_frame_number;
	_Model->setData(_Model->index(0, 12), ret1.str().data());

	std::ostringstream ret2;
	ret2 << std::hex << "0x" << temp_pte.Bits.WsleAge;
	_Model->setData(_Model->index(0, 13), ret2.str().data());

	std::ostringstream ret3;
	ret3 << std::hex << "0x" << temp_pte.Bits.WsleProtection;
	_Model->setData(_Model->index(0, 14), ret3.str().data());

	if (temp_pte.Bits.no_execute)
	{
		_Model->setData(_Model->index(0, 15), "Y");
		_Model->item(0, 15)->setBackground(QColor(Qt::green));
	}
	else
	{
		_Model->setData(_Model->index(0, 15), "N");
		_Model->item(0, 15)->setBackground(QColor(Qt::red));
	}
}

void PageTable::ReadPage()
{
	_PXE_Model->removeRows(0, _PXE_Model->rowCount());
	_PPE_Model->removeRows(0, _PPE_Model->rowCount());
	_PDE_Model->removeRows(0, _PDE_Model->rowCount());
	_PTE_Model->removeRows(0, _PTE_Model->rowCount());
	ui.lineEdit_pxe->setText("");
	ui.lineEdit_ppe->setText("");
	ui.lineEdit_pde->setText("");
	ui.lineEdit_pte->setText("");
	ui.lineEdit_pxe_2->setText("");
	ui.lineEdit_ppe_2->setText("");
	ui.lineEdit_pde_2->setText("");
	ui.lineEdit_pte_2->setText("");


	if (ui.lineEdit_4->text() != "" && ui.lineEdit->text() != "")
	{
		ui.lineEdit_2->setText("");
		ui.lineEdit_3->setText("");

		std::string addr_str = ui.lineEdit->text().toStdString();
		addr_str = ReplaceStr2(addr_str, "`", "");
		if (addr_str.find("0x") != std::string::npos)
		{
			addr_str.erase(0, 2);
		}
		ULONG64 Addr = strtoull(addr_str.data(), 0, 16);
		SetAddrOffset(Addr);
	}
	else if (ui.lineEdit_4->text() == "" && ui.lineEdit->text() != "")
	{
		ui.lineEdit_2->setText("");
		ui.lineEdit_3->setText("");

		std::string addr_str = ui.lineEdit->text().toStdString();
		addr_str = ReplaceStr2(addr_str, "`", "");
		if (addr_str.find("0x") != std::string::npos)
		{
			addr_str.erase(0, 2);
		}
		ULONG64 Addr = strtoull(addr_str.data(), 0, 16);
		SetAddrOffset(Addr);

		PageTableStruct temp_PageTableStruct = GetPageTableFromKernel(GetCurrentProcessId(), Addr);

		std::ostringstream ret1;
		ret1 << std::hex << "0x" << temp_PageTableStruct.pxe_addr;
		ui.lineEdit_pxe->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();

		ret1 << std::hex << "0x" << temp_PageTableStruct.ppe_addr;
		ui.lineEdit_ppe->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();

		ret1 << std::hex << "0x" << temp_PageTableStruct.pde_addr;
		ui.lineEdit_pde->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();

		ret1 << std::hex << "0x" << temp_PageTableStruct.pte_addr;
		ui.lineEdit_pte->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();


		ret1 << std::hex << "0x" << temp_PageTableStruct.pxe.value;
		ui.lineEdit_pxe_2->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();

		ret1 << std::hex << "0x" << temp_PageTableStruct.ppe.value;
		ui.lineEdit_ppe_2->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();

		ret1 << std::hex << "0x" << temp_PageTableStruct.pde.value;
		ui.lineEdit_pde_2->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();

		ret1 << std::hex << "0x" << temp_PageTableStruct.pte.value;
		ui.lineEdit_pte_2->setText(ret1.str().data());
		ret1.str("");
		ret1.clear();

		SetTableViewValue(_PXE_Model, temp_PageTableStruct.pxe);
		SetTableViewValue(_PPE_Model, temp_PageTableStruct.ppe);
		if (temp_PageTableStruct.ppe.Bits.large_page)
		{
			return;
		}
		SetTableViewValue(_PDE_Model, temp_PageTableStruct.pde);
		if (temp_PageTableStruct.pde.Bits.large_page)
		{
			return;
		}
		SetTableViewValue(_PTE_Model, temp_PageTableStruct.pte);
	}
	else if (ui.lineEdit_2->text() != "")
	{
		ui.lineEdit->setText("");
		ui.lineEdit_3->setText("");

		std::string addr_str = ui.lineEdit_2->text().toStdString();
		if (addr_str.find("0x") != std::string::npos)
		{
			addr_str.erase(0, 2);
		}
		ULONG64 Addr = strtoull(addr_str.data(), 0, 16);
		HardwarePteX64ForWindows temp_pte = { 0 };
		temp_pte.value = Addr;
		SetTableViewValue(_PXE_Model, temp_pte);
	}
	else if (ui.lineEdit_3->text() != "")
	{
		ui.lineEdit->setText("");
		ui.lineEdit_2->setText("");

		std::string addr_str = ui.lineEdit_3->text().toStdString();
		if (addr_str.find("0x") != std::string::npos)
		{
			addr_str.erase(0, 2);
		}
		ULONG64 Addr = strtoull(addr_str.data(), 0, 16);
		HardwarePteX64ForWindows temp_pte = { 0 };
		_KernelMemory.ReadKernelMemory(Addr, 8, &temp_pte.value);
		SetTableViewValue(_PTE_Model, temp_pte);
	}
}

#define TEST_GetPageTables CTL_CODE(FILE_DEVICE_UNKNOWN,0x7129,METHOD_BUFFERED ,FILE_ANY_ACCESS)
PageTableStruct PageTable::GetPageTableFromKernel(ULONG64 PID, ULONG64 addr)
{
	PageTableStruct temp_PageTableStruct = { 0 };

	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return temp_PageTableStruct;
	}

	ULONG64 temp_buffer[2] = { 0 };
	temp_buffer[0] = PID;
	temp_buffer[1] = addr;

	DWORD dwRet = 0;
	DeviceIoControl(m_hDevice, TEST_GetPageTables, temp_buffer,sizeof(ULONG64)*2, &temp_PageTableStruct, sizeof(PageTableStruct), &dwRet, NULL);
	if (dwRet)
	{
		CloseHandle(m_hDevice);
		return temp_PageTableStruct;
	}
	CloseHandle(m_hDevice);
	return temp_PageTableStruct;
}