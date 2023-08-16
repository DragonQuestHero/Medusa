#include "HookScanner.h"

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
	_Model->setHeaderData(2, Qt::Horizontal, u8"Offset");
	_Model->setHeaderData(3, Qt::Horizontal, u8"File");
	_Model->setHeaderData(4, Qt::Horizontal, u8"Memory");
	_Model->setHeaderData(5, Qt::Horizontal, u8"Path");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 150);
	ui.tableView->setColumnWidth(2, 150);
	ui.tableView->setColumnWidth(3, 400);
	ui.tableView->setColumnWidth(4, 400);
	ui.tableView->setColumnWidth(5, 400);

	//this->setWindowFlags(Qt::FramelessWindowHint);
}

