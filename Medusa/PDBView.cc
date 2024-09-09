#include "PDBView.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <regex>


PDBView::PDBView(QWidget* parent)
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
	_Model->setHeaderData(1, Qt::Horizontal, u8"Name");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(3, Qt::Horizontal, u8"Base");
	_Model->setHeaderData(4, Qt::Horizontal, u8"RVA");

	ui.tableView->setColumnWidth(0, 100);
	ui.tableView->setColumnWidth(1, 450);
	ui.tableView->setColumnWidth(2, 200);
	ui.tableView->setColumnWidth(3, 200);
	ui.tableView->setColumnWidth(4, 200);


	connect(ui.pushButton, SIGNAL(clicked()), SLOT(LoadALL()));//进程
	connect(ui.pushButton_2, SIGNAL(clicked()), SLOT(Serch()));//进程
}


void PDBView::LoadALL()
{
	_Model->removeRows(0, _Model->rowCount());
	_PDBInfo.GetALL();
	ui.progressBar->setMaximum(_PDBInfo._Symbol.size());
	ui.progressBar->setValue(0);
	int i = 0;
	int j = 0;
	for (auto x : _PDBInfo._Symbol)
	{
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		_Model->setData(_Model->index(i, 1), x.Name.data());
		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.Addr;
		_Model->setData(_Model->index(i, 2), ret.str().data());
		ret.str("");
		ret << std::hex << "0x" << (ULONG64)_PDBInfo._BaseAddr;
		_Model->setData(_Model->index(i, 3), ret.str().data());
		ret.str("");
		ret << std::hex << "0x" << (ULONG64)x.RVA;
		_Model->setData(_Model->index(i, 4), ret.str().data());
		i++;
		j++;
		ui.progressBar->setValue(ui.progressBar->value() + 1);
		if (j == 1000)
		{
			QCoreApplication::processEvents();
		}
	}
	ui.progressBar->setValue(_PDBInfo._Symbol.size());
}

void PDBView::Serch()
{
	_Model->removeRows(0, _Model->rowCount());
	if (ui.lineEdit->text() != "" && ui.lineEdit_2->text() != "")
	{
		std::string temp_str3 = ui.lineEdit->text().toStdString();
		if (temp_str3.find("0x") != std::string::npos)
		{
			temp_str3.erase(0, 2);
		}
		ULONG64 addr = strtoull(temp_str3.data(), 0, 16);//输入的是数字
		std::vector<SYMBOLSTRUCT> temp_vector = _PDBInfo.PdbGetStruct(&_PDBInfo._Pdb, ui.lineEdit_2->text().toStdString());
		int i = 0;
		for (auto x : temp_vector)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), x.Name.data());
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << addr + x.Offset;
			_Model->setData(_Model->index(i, 2), ret2.str().data());

			std::ostringstream ret;
			_Model->setData(_Model->index(i, 3), x.Type.data());
			ret << std::hex << "Size: 0x" << (ULONG64)x.Size;
			ret << std::hex << "Offset: 0x" << (ULONG64)x.Offset;
			_Model->setData(_Model->index(i, 4), ret.str().data());
			i++;
		}
		return;
	}
	if (ui.lineEdit->text() != "")
	{
		ULONG64 RVA = EzPdbGetRva(&_PDBInfo._Pdb, ui.lineEdit->text().toStdString());//NtTerminateThread
		if (RVA)
		{
			_Model->setVerticalHeaderItem(0, new QStandardItem);
			_Model->setData(_Model->index(0, 0), 0);
			_Model->setData(_Model->index(0, 1), ui.lineEdit->text());
			std::ostringstream ret;
			ret << std::hex << "0x" << (ULONG64)RVA+ _PDBInfo._BaseAddr;
			_Model->setData(_Model->index(0, 2), ret.str().data());
			ret.str("");
			ret << std::hex << "0x" << (ULONG64)_PDBInfo._BaseAddr;
			_Model->setData(_Model->index(0, 3), ret.str().data());
			ret.str("");
			ret << std::hex << "0x" << (ULONG64)RVA;
			_Model->setData(_Model->index(0, 4), ret.str().data());
		}
		return;
	}
	if (ui.lineEdit_2->text() != "")
	{
		std::vector<SYMBOLSTRUCT> temp_vector = _PDBInfo.PdbGetStruct(&_PDBInfo._Pdb, ui.lineEdit_2->text().toStdString());
		int i = 0;
		for (auto x : temp_vector)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), x.Name.data());
			std::ostringstream ret2;
			_Model->setData(_Model->index(i, 2), x.Type.data());
			ret2 << std::hex << "Size: 0x" << (ULONG64)x.Size;
			_Model->setData(_Model->index(i, 3), ret2.str().data());
			std::ostringstream ret;
			ret << "Offset DEC:" << (ULONG64)x.Offset<<"---";
			ret << std::hex << "Hex: 0x" << (ULONG64)x.Offset;
			_Model->setData(_Model->index(i, 4), ret.str().data());
			i++;
		}
		return;
	}
	if (ui.lineEdit_3->text() != "")
	{
		if (!_PDBInfo._Symbol.size())
		{
			_PDBInfo.GetALL();
		}
		int i = 0;

		std::string temp_str3 = ui.lineEdit_3->text().toStdString();
		if (temp_str3.find("0x") != std::string::npos)
		{
			temp_str3.erase(0, 2);
		}
		std::regex pattern("^[0-9]+$");
		if (std::regex_match(temp_str3, pattern))
		{
			ULONG64 addr = strtoull(temp_str3.data(), 0, 16);//输入的是数字
			if (addr != 0)
			{
				for (auto x : _PDBInfo._Symbol)
				{
					if (addr >= x.Addr && addr < x.Addr + x.Size)
					{
						_Model->setVerticalHeaderItem(i, new QStandardItem);
						_Model->setData(_Model->index(i, 0), i);
						_Model->setData(_Model->index(i, 1), x.Name.data());
						std::ostringstream ret;
						ret << std::hex << "0x" << (ULONG64)x.Addr;
						_Model->setData(_Model->index(i, 2), ret.str().data());
						ret.str("");
						ret << std::hex << "0x" << (ULONG64)_PDBInfo._BaseAddr;
						_Model->setData(_Model->index(i, 3), ret.str().data());
						ret.str("");
						ret << std::hex << "0x" << (ULONG64)x.RVA;
						_Model->setData(_Model->index(i, 4), ret.str().data());
						i++;
					}
				}
			}
		}
		else
		{
			for (auto x : _PDBInfo._Symbol)
			{
				std::string temp_str1 = Case_Upper(x.Name);
				std::string temp_str2 = Case_Upper(ui.lineEdit_3->text().toStdString());
				if (temp_str1.find(temp_str2) != std::string::npos)
				{
					_Model->setVerticalHeaderItem(i, new QStandardItem);
					_Model->setData(_Model->index(i, 0), i);
					_Model->setData(_Model->index(i, 1), x.Name.data());
					std::ostringstream ret;
					ret << std::hex << "0x" << (ULONG64)x.Addr;
					_Model->setData(_Model->index(i, 2), ret.str().data());
					ret.str("");
					ret << std::hex << "0x" << (ULONG64)_PDBInfo._BaseAddr;
					_Model->setData(_Model->index(i, 3), ret.str().data());
					ret.str("");
					ret << std::hex << "0x" << (ULONG64)x.RVA;
					_Model->setData(_Model->index(i, 4), ret.str().data());
					i++;
				}
			}
		}
	}
}