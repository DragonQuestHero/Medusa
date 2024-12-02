#include "HookScanner.h"

#include "FileCheck.h"

HookScanner::HookScanner(QWidget* parent)
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
	_Model->setHeaderData(3, Qt::Horizontal, u8"File");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Memory");
	_Model->setHeaderData(5, Qt::Horizontal, u8"Path");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 150);
	ui.tableView->setColumnWidth(2, 150);
	ui.tableView->setColumnWidth(3, 400);
	ui.tableView->setColumnWidth(4, 400);
	ui.tableView->setColumnWidth(5, 400);

	_TableView_Action_UserMemory.setText("MemoryView");
	ui.tableView->addAction(&_TableView_Action_UserMemory);
	connect(&_TableView_Action_UserMemory, SIGNAL(triggered(bool)), SLOT(MemoryView(bool)));
}

void HookScanner::MemoryView(bool)
{
	_UserMemory.setWindowTitle("ProcessID:" + QString::number(_PID));
	_UserMemory.ui.tabWidget->setCurrentIndex(0);
	_UserMemory.PID = _PID;
	_UserMemory.ui.label->setText("ProcessID:" + QString::number(_PID) + "    CR3:0x0");
	_UserMemory.ui.lineEdit->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString());
	_UserMemory.ui.lineEdit_2->setText("0x1000");
	_UserMemory.QueryMemory();
	_UserMemory.show();
}


bool HookScanner::ProcessHookScanner(ULONG64 PID, bool kernel_mode)
{
	_PID = PID;
	_Model->removeRows(0, _Model->rowCount());
	FileCheck temp_check(kernel_mode);
	std::vector<_CheckDifferent> temp_vector = temp_check.CheckPlain(PID);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		_Model->setData(_Model->index(i, 1), QString::fromWCharArray(x.Name));
		std::ostringstream ret;
		ret << std::hex << "0x" << x.Addr;
		_Model->setData(_Model->index(i, 2), ret.str().data());
		_Model->setData(_Model->index(i, 3), String_TO_HEX(std::string(x.FileHex, 20)).data());
		_Model->setData(_Model->index(i, 4), String_TO_HEX(std::string(x.MemoryHex, 20)).data());
		_Model->setData(_Model->index(i, 5), QString::fromWCharArray(x.Path));
		if (x.Fail)
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
	return true;
}

bool HookScanner::ProcessHookScannerQuick(ULONG64 PID, bool kernel_mode)
{
	_PID = PID;
	_Model->removeRows(0, _Model->rowCount());
	FileCheck temp_check(kernel_mode);
	std::vector<_CheckDifferent> temp_vector = temp_check.CheckPlainQuick(PID);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		_Model->setData(_Model->index(i, 1), QString::fromWCharArray(x.Name));
		std::ostringstream ret;
		ret << std::hex << "0x" << x.Addr;
		_Model->setData(_Model->index(i, 2), ret.str().data());
		_Model->setData(_Model->index(i, 3), String_TO_HEX(std::string(x.FileHex, 20)).data());
		_Model->setData(_Model->index(i, 4), String_TO_HEX(std::string(x.MemoryHex, 20)).data());
		_Model->setData(_Model->index(i, 5), QString::fromWCharArray(x.Path));
		if (x.Fail)
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
	return true;
}