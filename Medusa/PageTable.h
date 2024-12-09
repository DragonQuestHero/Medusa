#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QStringListModel>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iomanip>

#include <windows.h>

#include "ui_PageTable.h"
#include "KernelMemory.h"
#include "UserMemoryList.h"


class PageTable : public QMainWindow
{
	Q_OBJECT

public:
	PageTable(QWidget* parent = nullptr);
	~PageTable() = default;
public:
	void SetUI(QTableView* tableView,QStandardItemModel* &_Model)
	{
		_Model = new QStandardItemModel();
		tableView->setModel(_Model);
		tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		tableView->horizontalHeader()->setSectionsClickable(false);
		tableView->verticalHeader()->setDefaultSectionSize(25);
		tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

		_Model->setColumnCount(16);
		//如果为0，表示页面不在内存中（可能在磁盘上），且访问该页面会导致缺页异常。
		_Model->setHeaderData(0, Qt::Horizontal, u8"Valid[0]");//表示该页面是否在物理内存中。如果设置为1，表示该页面在内存中；

		//设置为1表示该页面可读写，设置为0表示该页面为只读。
		_Model->setHeaderData(1, Qt::Horizontal, u8"Wirte[1]");//控制页面的读写权限。

		//设置为1表示用户模式可以访问该页面，设置为0表示只有特权模式（内核）可以访问
		_Model->setHeaderData(2, Qt::Horizontal, u8"User[2]");//指定访问权限。

		//设置为1表示使用写直达（write-through）策略，而设置为0表示使用写回（write-back）策略
		_Model->setHeaderData(3, Qt::Horizontal, u8"WriteThrough[3]");//控制写入缓存的策略。

		//设置为1表示禁用缓存，设置为0表示允许缓存
		_Model->setHeaderData(4, Qt::Horizontal, u8"CacheDisable[4]"); //控制页面是否可以被缓存。

		_Model->setHeaderData(5, Qt::Horizontal, u8"Accessed[5]");//指示该页面是否已经被访问（读取或写入）

		//只有在页面被写入后才会被设置为1。这对于页面换出时是否需要写回到磁盘非常重要
		_Model->setHeaderData(6, Qt::Horizontal, u8"Dirty[6]");//指示该页面是否已经被修改。

		_Model->setHeaderData(7, Qt::Horizontal, u8"LargePage[7]");//如果设置为1，表示该页表项指向一个大页面（如2MB或1GB的页面），而不是常规的4KB页面。

		_Model->setHeaderData(8, Qt::Horizontal, u8"Global[8]");//表示该页面是全局的，适用于所有进程，而不仅仅是当前进程。全局页面不会在上下文切换时被刷新

		//如果该位设置为1，则表示该页面是可进行写时复制的。即，当一个进程尝试写入该页面时，会触发页面保护异常，从而允许操作系统进行页面复制
		_Model->setHeaderData(9, Qt::Horizontal, u8"CopyOnWrite[9]");

		_Model->setHeaderData(10, Qt::Horizontal, u8"Prototype[10]");//设置为1表示该页面是一个原型页

		_Model->setHeaderData(11, Qt::Horizontal, u8"[11]");

		_Model->setHeaderData(12, Qt::Horizontal, u8"PageFrameNumber[12:47]");//表示页面在物理内存中的帧号。这个部分用于指向实际的物理页面

		//_Model->setHeaderData(13, Qt::Horizontal, u8"ResHardware[48:51]");//ReservedForHardware
		//_Model->setHeaderData(14, Qt::Horizontal, u8"ResSoftware[52:55]");//ReservedForSoftware

		//这个字段用于跟踪工作集列表条目的“年龄”，即该页面在工作集中的使用频率
		_Model->setHeaderData(13, Qt::Horizontal, u8"WsleAge[56:59]");

		//这个字段用于指示工作集列表条目的保护属性，例如页面的读写权限
		_Model->setHeaderData(14, Qt::Horizontal, u8"WsleProtection[60:63]");

		_Model->setHeaderData(15, Qt::Horizontal, u8"NoExecute[63]");//用于控制页面是否可执行。它通常位于页表项的高位（在支持该特性的处理器上）。
		//当NX位设置为1时，表示该页面不可执行；当设置为0时，表示该页面是可执行的。




		tableView->setColumnWidth(0, 70);
		tableView->setColumnWidth(1, 70);
		tableView->setColumnWidth(2, 70);
		tableView->setColumnWidth(3, 110);
		tableView->setColumnWidth(4, 110);
		tableView->setColumnWidth(5, 100);
		tableView->setColumnWidth(6, 60);
		tableView->setColumnWidth(7, 90);
		tableView->setColumnWidth(8, 80);
		tableView->setColumnWidth(9, 110);
		tableView->setColumnWidth(11, 50);
		tableView->setColumnWidth(12, 180);
		tableView->setColumnWidth(13, 110);
		tableView->setColumnWidth(14, 140);
	}
	void SetTableViewValue(QStandardItemModel* _Model, HardwarePteX64ForWindows&);
public slots:
	void ReadPage();
	void MemoryView(bool)
	{
		/*_KernelMemory.ui.lineEdit->setText(
			ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString());
		_KernelMemory.QueryMemory();
		_KernelMemory.show();*/
	}
public:
	Ui::Form_PageTable ui;
	QStandardItemModel* _PXE_Model;
	QStandardItemModel* _PPE_Model;
	QStandardItemModel* _PDE_Model;
	QStandardItemModel* _PTE_Model;
	KernelMemory _KernelMemory;
public:
	ULONG64 _PID = 0;
	QAction _TableView_Action_MemoryView;
private:
};