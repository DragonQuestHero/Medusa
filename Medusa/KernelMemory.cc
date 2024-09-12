#include "KernelMemory.h"


#include "Zydis/include/Zydis/Zydis.h"

KernelMemory::KernelMemory(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	_Model->setColumnCount(4);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Hex");
	_Model->setHeaderData(3, Qt::Horizontal, u8"ASM");

	ui.tableView->setColumnWidth(0, 70);
	ui.tableView->setColumnWidth(1, 200);
	ui.tableView->setColumnWidth(2, 320);
	ui.tableView->setColumnWidth(3, 420);

	connect(ui.pushButton, SIGNAL(clicked()), SLOT(QueryMemory()));
	connect(ui.pushButton_2, SIGNAL(clicked()), SLOT(DumpMemory()));
	connect(ui.pushButton_3, SIGNAL(clicked()), SLOT(DumpASM()));

	connect(ui.textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this,&KernelMemory::TexeBar);
	connect(ui.textEdit_2->verticalScrollBar(), &QScrollBar::valueChanged, this, &KernelMemory::TexeBar);
	connect(ui.textEdit_3->verticalScrollBar(), &QScrollBar::valueChanged, this, &KernelMemory::TexeBar);

	connect(ui.tabWidget, SIGNAL(currentChanged(int)), SLOT(ChangeTab()));//½ø³Ì
}

void KernelMemory::ChangeTab()
{
	if (ui.lineEdit->text() != "" && ui.lineEdit_2->text() != "")
	{
		QueryMemory();
	}
}

void KernelMemory::TexeBar(int value)
{
	if (ui.textEdit->verticalScrollBar()->value() == value)
	{
		ui.textEdit_2->verticalScrollBar()->setValue(value);
		ui.textEdit_3->verticalScrollBar()->setValue(value);
	}
	else if (ui.textEdit_2->verticalScrollBar()->value() == value)
	{
		ui.textEdit->verticalScrollBar()->setValue(value);
		ui.textEdit_3->verticalScrollBar()->setValue(value);
	}
	else if (ui.textEdit_3->verticalScrollBar()->value() == value)
	{
		ui.textEdit_2->verticalScrollBar()->setValue(value);
		ui.textEdit->verticalScrollBar()->setValue(value);
	}
}

void KernelMemory::DumpMemory()
{
	std::string addr_str = ui.lineEdit->text().toStdString();
	if (addr_str.find("0x") != std::string::npos)
	{
		addr_str.erase(0, 2);
	}
	ULONG64 Addr = strtoull(addr_str.data(), 0, 16);

	std::string size_str = ui.lineEdit_2->text().toStdString();
	if (size_str.find("0x") != std::string::npos)
	{
		size_str.erase(0, 2);
	}
	ULONG64 Size = strtoull(size_str.data(), 0, 16);
	if (Size)
	{
		char* temp_buffer = new char[Size];
		RtlZeroMemory(temp_buffer, Size);
		ULONG64 ret = ReadKernelMemory(Addr, Size, temp_buffer);
		if (ret)
		{
			std::fstream temp_file(addr_str, std::ios::out | std::ios::binary);
			if (temp_file.is_open())
			{
				temp_file << std::string(temp_buffer, ret);
				temp_file.close();
				QMessageBox::information(this, "Ret", "susscss");
				return;
			}
		}
	}
	QMessageBox::information(this, "Ret", "error");
}

void KernelMemory::DumpASM()
{
	std::string addr_str = ui.lineEdit->text().toStdString();
	if (addr_str.find("0x") != std::string::npos)
	{
		addr_str.erase(0, 2);
	}
	ULONG64 Addr = strtoull(addr_str.data(), 0, 16);

	std::string size_str = ui.lineEdit_2->text().toStdString();
	if (size_str.find("0x") != std::string::npos)
	{
		size_str.erase(0, 2);
	}
	ULONG64 Size = strtoull(size_str.data(), 0, 16);
	if (Size)
	{
		char* temp_buffer = new char[Size];
		RtlZeroMemory(temp_buffer, Size);
		ULONG64 ret = ReadKernelMemory(Addr, Size, temp_buffer);
		if (ret)
		{
			std::fstream temp_file(addr_str+".txt", std::ios::out | std::ios::binary);
			if (temp_file.is_open())
			{
				ZyanU64 runtime_address = Addr;

				int i = 0;
				// Loop over the instructions in our buffer. 
				ZyanUSize offset = 0;
				ZydisDisassembledInstruction instruction;
				while (ZYAN_SUCCESS(ZydisDisassembleIntel(
					/* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
					/* runtime_address: */ runtime_address,
					/* buffer:          */ temp_buffer + offset,
					/* length:          */ Size - offset,
					/* instruction:     */ &instruction
				)))
				{
					temp_file << i << "    ";
					temp_file << std::hex << runtime_address << "          ";
					std::string str_line2;
					for (int j = 0; j < instruction.info.length; j++)
					{
						USHORT temp_short = 0;
						RtlCopyMemory(&temp_short, temp_buffer + offset + j, 1);
						std::ostringstream ret;
						if (temp_short < 0x10)
						{
							ret << std::hex << "0x0" << temp_short;
						}
						else
						{
							ret << std::hex << "0x" << temp_short;
						}
						str_line2 = str_line2 + ret.str() + " ";
					}
					temp_file << str_line2 << "\r\n";
					temp_file << "                                         " << instruction.text << "\r\n";
					i++;

					offset += instruction.info.length;
					runtime_address += instruction.info.length;
				}
				temp_file.close();
				QMessageBox::information(this, "Ret", "susscss");
				return;
			}
		}
	}
	QMessageBox::information(this, "Ret", "error");
}

void KernelMemory::QueryMemoryTable1(char* temp_buffer, ULONG64 ret, ULONG64 Addr)
{
	std::string str_line1;
	std::string str_line2;
	std::string str_line3;
	for (int i = 0; i < ret / 8; i++)
	{
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << Addr + i * 8;
		str_line1 = str_line1 + ret2.str() + "\r\n";

		if (ui.radioButton->isChecked())
		{
			for (int j = 0; j < 8; j++)
			{
				USHORT temp_short = 0;
				RtlCopyMemory(&temp_short, temp_buffer + i * 8 + j, 1);
				std::ostringstream ret;
				if (temp_short < 0x10)
				{
					ret << std::hex << "0x0" << temp_short;
				}
				else
				{
					ret << std::hex << "0x" << temp_short;
				}
				str_line2 = str_line2 + ret.str() + " ";
				str_line3 = str_line3 + std::string(temp_buffer + i * 8 + j, 1) + " ";
			}
		}
		else if (ui.radioButton_2->isChecked())
		{
			ULONG64 temp_short = 0;
			RtlCopyMemory(&temp_short, temp_buffer + i * 8, 8);
			std::ostringstream ret;
			ret << std::hex << "0x0" << temp_short;
			str_line2 = str_line2 + ret.str();
		}
		str_line2 = str_line2 + "\r\n";
		str_line3 = str_line3 + "\r\n";
	}
	ui.textEdit_3->setPlainText(str_line1.data());
	ui.textEdit->setPlainText(str_line2.data());
	ui.textEdit_2->setPlainText(QString::fromStdString(str_line3));
}

void KernelMemory::QueryMemoryTable2(char* temp_buffer, ULONG64 ret, ULONG64 Addr, ULONG64 Size)
{
	ZyanU64 runtime_address = Addr;

	int i = 0;
	// Loop over the instructions in our buffer. 
	ZyanUSize offset = 0;
	ZydisDisassembledInstruction instruction;
	while (ZYAN_SUCCESS(ZydisDisassembleIntel(
		/* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
		/* runtime_address: */ runtime_address,
		/* buffer:          */ temp_buffer + offset,
		/* length:          */ Size - offset,
		/* instruction:     */ &instruction
	))) 
	{
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		std::ostringstream ret;
		ret << std::hex << "0x" << runtime_address;
		_Model->setData(_Model->index(i, 1), ret.str().data());
		
		std::string str_line2;
		for (int j = 0; j < instruction.info.length; j++)
		{
			USHORT temp_short = 0;
			RtlCopyMemory(&temp_short, temp_buffer + offset + j, 1);
			std::ostringstream ret;
			if (temp_short < 0x10)
			{
				ret << std::hex << "0x0" << temp_short;
			}
			else
			{
				ret << std::hex << "0x" << temp_short;
			}
			str_line2 = str_line2 + ret.str() + " ";
		}
		_Model->setData(_Model->index(i, 2), str_line2.data());
		_Model->setData(_Model->index(i, 3), instruction.text);
		i++;

		offset += instruction.info.length;
		runtime_address += instruction.info.length;
	}
}

void KernelMemory::QueryMemory()
{
	_Model->removeRows(0, _Model->rowCount());
	ui.textEdit_3->setPlainText("");
	ui.textEdit->setPlainText("");
	ui.textEdit_2->setPlainText("");


	std::string addr_str = ui.lineEdit->text().toStdString();
	if (addr_str.find("0x") != std::string::npos)
	{
		addr_str.erase(0, 2);
	}
	ULONG64 Addr = strtoull(addr_str.data(), 0, 16);

	std::string size_str = ui.lineEdit_2->text().toStdString();
	if (size_str.find("0x") != std::string::npos)
	{
		size_str.erase(0, 2);
	}
	ULONG64 Size = strtoull(size_str.data(), 0, 16);
	if (Size)
	{
		char* temp_buffer = new char[Size];
		RtlZeroMemory(temp_buffer, Size);
		ULONG64 ret = ReadKernelMemory(Addr, Size, temp_buffer);
		if (ret)
		{
			if (ui.tabWidget->currentIndex() == 0)
			{
				QueryMemoryTable2(temp_buffer, ret, Addr,Size);
			}
			if (ui.tabWidget->currentIndex() == 1)
			{
				QueryMemoryTable1(temp_buffer, ret, Addr);
			}
		}
		delete temp_buffer;
	}
	std::ostringstream ret;
	ret << "Process:Medusa    CR3:" << std::hex << "0x" << GetKernelCR3();
	ui.label->setText(ret.str().data());
}

#define TEST_ReadKernelMemory CTL_CODE(FILE_DEVICE_UNKNOWN,0x7117,METHOD_BUFFERED ,FILE_ANY_ACCESS)
ULONG64 KernelMemory::ReadKernelMemory(ULONG64 Addr, ULONG64 Size, void* Buffer)
{
	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return 0;
	}

	ULONG64* temp_ulong64 = new ULONG64[2];
	temp_ulong64[0] = Addr;
	temp_ulong64[1] = Size;

	DWORD dwRet = 0;
	DeviceIoControl(m_hDevice, TEST_ReadKernelMemory, temp_ulong64, 0x10, Buffer, Size, &dwRet, NULL);

	delete temp_ulong64;
	CloseHandle(m_hDevice);
	return dwRet;
}

#define TEST_GetCR3 CTL_CODE(FILE_DEVICE_UNKNOWN,0x7121,METHOD_BUFFERED ,FILE_ANY_ACCESS)
ULONG64 KernelMemory::GetKernelCR3()
{
	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == m_hDevice)
	{
		return 0;
	}

	ULONG64 CR3 = 0;

	DWORD dwRet = 0;
	DeviceIoControl(m_hDevice, TEST_GetCR3, &CR3, 8, &CR3, 8, &dwRet, NULL);

	CloseHandle(m_hDevice);
	return CR3;
}