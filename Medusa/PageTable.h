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


typedef union _DIRECTORY_TABLE_BASE
{
	struct
	{
		UINT64 Ignored0 : 3;            /* 2:0   */
		UINT64 PageWriteThrough : 1;    /* 3     */
		UINT64 PageCacheDisable : 1;    /* 4     */
		UINT64 _Ignored1 : 7;           /* 11:5  */
		UINT64 PhysicalAddress : 36;    /* 47:12 */
		UINT64 _Reserved0 : 16;         /* 63:48 */

	} Bits;

	UINT64 BitAddress;

} DIR_TABLE_BASE;

using PageTableStruct = struct
{
	DIR_TABLE_BASE cr3;
	ULONG64 pxe_addr;
	ULONG64 ppe_addr;
	ULONG64 pde_addr;
	ULONG64 pte_addr;
	HardwarePteX64ForWindows pxe;
	HardwarePteX64ForWindows ppe;
	HardwarePteX64ForWindows pde;
	HardwarePteX64ForWindows pte;
};

typedef union _ADDRESS_STRUCTURE {
	struct {
		ULONG64 Offset : 12;         // 11:0��ҳ��ƫ��
		ULONG64 PEIndex : 9;         // 20:12��ҳ������
		ULONG64 PDIndex : 9;         // 29:21��ҳĿ¼����
		ULONG64 PDPTIndex : 9;       // 38:30��ҳĿ¼ָ�������
		ULONG64 PML4Index : 9;      // 47:39��PML4 ����
		ULONG64 Reserved : 16;      // 63:48������λ

	} Bits;
	uint64_t BitAddress;
} ADDRESS_STRUCTURE;

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
		//���Ϊ0����ʾҳ�治���ڴ��У������ڴ����ϣ����ҷ��ʸ�ҳ��ᵼ��ȱҳ�쳣��
		_Model->setHeaderData(0, Qt::Horizontal, u8"Valid[0]");//��ʾ��ҳ���Ƿ��������ڴ��С��������Ϊ1����ʾ��ҳ�����ڴ��У�

		//����Ϊ1��ʾ��ҳ��ɶ�д������Ϊ0��ʾ��ҳ��Ϊֻ����
		_Model->setHeaderData(1, Qt::Horizontal, u8"Wirte[1]");//����ҳ��Ķ�дȨ�ޡ�

		//����Ϊ1��ʾ�û�ģʽ���Է��ʸ�ҳ�棬����Ϊ0��ʾֻ����Ȩģʽ���ںˣ����Է���
		_Model->setHeaderData(2, Qt::Horizontal, u8"User[2]");//ָ������Ȩ�ޡ�

		//����Ϊ1��ʾʹ��дֱ�write-through�����ԣ�������Ϊ0��ʾʹ��д�أ�write-back������
		_Model->setHeaderData(3, Qt::Horizontal, u8"WriteThrough[3]");//����д�뻺��Ĳ��ԡ�

		//����Ϊ1��ʾ���û��棬����Ϊ0��ʾ������
		_Model->setHeaderData(4, Qt::Horizontal, u8"CacheDisable[4]"); //����ҳ���Ƿ���Ա����档

		_Model->setHeaderData(5, Qt::Horizontal, u8"Accessed[5]");//ָʾ��ҳ���Ƿ��Ѿ������ʣ���ȡ��д�룩

		//ֻ����ҳ�汻д���Żᱻ����Ϊ1�������ҳ�滻��ʱ�Ƿ���Ҫд�ص����̷ǳ���Ҫ
		_Model->setHeaderData(6, Qt::Horizontal, u8"Dirty[6]");//ָʾ��ҳ���Ƿ��Ѿ����޸ġ�

		_Model->setHeaderData(7, Qt::Horizontal, u8"LargePage[7]");//�������Ϊ1����ʾ��ҳ����ָ��һ����ҳ�棨��2MB��1GB��ҳ�棩�������ǳ����4KBҳ�档

		_Model->setHeaderData(8, Qt::Horizontal, u8"Global[8]");//��ʾ��ҳ����ȫ�ֵģ����������н��̣����������ǵ�ǰ���̡�ȫ��ҳ�治�����������л�ʱ��ˢ��

		//�����λ����Ϊ1�����ʾ��ҳ���ǿɽ���дʱ���Ƶġ�������һ�����̳���д���ҳ��ʱ���ᴥ��ҳ�汣���쳣���Ӷ��������ϵͳ����ҳ�渴��
		_Model->setHeaderData(9, Qt::Horizontal, u8"CopyOnWrite[9]");

		_Model->setHeaderData(10, Qt::Horizontal, u8"Prototype[10]");//����Ϊ1��ʾ��ҳ����һ��ԭ��ҳ

		_Model->setHeaderData(11, Qt::Horizontal, u8"[11]");

		_Model->setHeaderData(12, Qt::Horizontal, u8"PageFrameNumber[12:47]");//��ʾҳ���������ڴ��е�֡�š������������ָ��ʵ�ʵ�����ҳ��

		//_Model->setHeaderData(13, Qt::Horizontal, u8"ResHardware[48:51]");//ReservedForHardware
		//_Model->setHeaderData(14, Qt::Horizontal, u8"ResSoftware[52:55]");//ReservedForSoftware

		//����ֶ����ڸ��ٹ������б���Ŀ�ġ����䡱������ҳ���ڹ������е�ʹ��Ƶ��
		_Model->setHeaderData(13, Qt::Horizontal, u8"WsleAge[56:59]");

		//����ֶ�����ָʾ�������б���Ŀ�ı������ԣ�����ҳ��Ķ�дȨ��
		_Model->setHeaderData(14, Qt::Horizontal, u8"WsleProtection[60:63]");

		_Model->setHeaderData(15, Qt::Horizontal, u8"NoExecute[63]");//���ڿ���ҳ���Ƿ��ִ�С���ͨ��λ��ҳ����ĸ�λ����֧�ָ����ԵĴ������ϣ���
		//��NXλ����Ϊ1ʱ����ʾ��ҳ�治��ִ�У�������Ϊ0ʱ����ʾ��ҳ���ǿ�ִ�еġ�




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
	void SetAddrOffset(ULONG64 Addr);
	PageTableStruct GetPageTableFromKernel(ULONG64 PID, ULONG64 addr);
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
	std::string ReplaceStr2(std::string& str,
		const std::string& old_value, const std::string& new_value)
	{
		for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
			if ((pos = str.find(old_value, pos)) != std::string::npos)
			{
				str.replace(pos, old_value.length(), new_value);
			}
			else
			{
				break;
			}
		}
		return str;
	}
};