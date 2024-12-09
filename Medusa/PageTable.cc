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
	if (ui.lineEdit->text() != "")
	{

	}
	if (ui.lineEdit_2->text() != "")
	{
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
	if (ui.lineEdit_3->text() != "")
	{

	}
}