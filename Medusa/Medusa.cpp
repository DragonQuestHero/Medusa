#include "Medusa.h"

#include "FileCheck.h"
#include "Hypervisor.h"
#include "DLLInject.h"
#include "KernelCallBackScanner.h"



#include "libpeconv/peconv.h"




Medusa::Medusa(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	Enable_Debug();

	ProcessUI();
	DriverUI();
	UnloadDriverUI();
	CallBackListUI();
	SSDTListUI();
	ShadowSSDTListUI();
	SSDTRightMenuUI();


	ProcessRightMenuUI();
	DriverRightMenuUI();
	
	QTextCodec* codec = QTextCodec::codecForName("UTF-8");
	QTextCodec::setCodecForLocale(codec);



	Set_SLOTS();



	HANDLE m_hDevice = CreateFileA("\\\\.\\IO_Control", GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != m_hDevice)
	{
		_Driver_Loaded = true;
	}
	CloseHandle(m_hDevice);

	/*_QTimer = new QTimer(this);
	QObject::connect(_QTimer, &QTimer::timeout, [&](){
			ChangeTab();
		});
	_QTimer->start(10000);*/
	ChangeTab();
}


void Medusa::Set_SLOTS()
{
	connect(ui.tabWidget, SIGNAL(currentChanged(int)), SLOT(ChangeTab()));//进程
	connect(ui.tabWidget, SIGNAL(tabBarClicked(int)), SLOT(ChangeTab()));//进程


	connect(&_TableView_Menu_Inject, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_HookCheck, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_Modules, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_Threads, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_KillProcess, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_PDBViewProcess, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Menu_Memory, SIGNAL(triggered(QAction*)), SLOT(ProcessRightMenu(QAction*)));//进程鼠标右键菜单
	connect(&_TableView_Action_HideProcess, SIGNAL(triggered(bool)), SLOT(HideProcess(bool)));//进程鼠标右键菜单


	connect(&_TableView_Menu_DriverClear, SIGNAL(triggered(QAction*)), SLOT(DriverRightMenu(QAction*)));
	connect(&_TableView_Action_DriverDumpFILE, SIGNAL(triggered(bool)), SLOT(DriverRightMenuDumpToFILE(bool)));
	connect(&_TableView_Action_DriverDumpMemory, SIGNAL(triggered(bool)), SLOT(DriverRightMenuDumpToMemory(bool)));
	connect(&_TableView_Menu_IOCTLScanner, SIGNAL(triggered(QAction*)), SLOT(DriverRightMenuIOCTLScanner(QAction*)));
	connect(&_TableView_Menu_PDBViewDriver, SIGNAL(triggered(QAction*)), SLOT(DriverRightMenu(QAction*)));
	connect(&_TableView_Action_ViewExportFunc, SIGNAL(triggered(bool)), SLOT(DriverRightMenuViewExportFunc(bool)));
	connect(&_TableView_Menu_Driver_Unload, SIGNAL(triggered(QAction*)), SLOT(DriverRightMenuUnload(QAction*)));

	connect(&_TableView_Action_SSDT_MemoryView, SIGNAL(triggered(bool)), SLOT(SSDTMemoryView(bool)));
	connect(&_TableView_Action_SSSDT_MemoryView, SIGNAL(triggered(bool)), SLOT(SSSDTMemoryView(bool)));

	connect(ui.menuMenu, SIGNAL(triggered(QAction*)), SLOT(DriverLoadMenu(QAction*)));
	connect(ui.menuHypervisor, SIGNAL(triggered(QAction*)), SLOT(HypervisorMenu(QAction*)));
	connect(ui.menuPDB, SIGNAL(triggered(QAction*)), SLOT(PdbMenu(QAction*)));
	connect(ui.menuViewKernelMemory, SIGNAL(triggered(QAction*)), SLOT(ViewKernelMemory(QAction*)));
}

void Medusa::DriverLoadMenu(QAction* action)
{
	if (action->text() == "NtLoad" ||
		action->text() == "NormalLoad" ||
		action->text() == "UnLoadMedusaDriver" ||
		action->text() == "Nt" ||
		action->text() == "Normal" ||
		action->text() == "Unload")
	{
		DriverLoad(action);
		return;
	}
}

void Medusa::HypervisorMenu(QAction* action)
{
	if (action->text() == "R3Check")
	{
		Hypervisor _Hypervisor;
		std::string str = u8"×=detected virtua environment\r\n√=not\r\n";
		str = str + "[Checking for known hypervisor vendors]:  ";
		if (_Hypervisor.check_for_known_hypervisor())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		str = str + "[Checking highest low function leaf]:  ";
		if (_Hypervisor.check_highest_low_function_leaf())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		str = str + "[Checking invalid leaf]:  ";
		if (_Hypervisor.check_invalid_leaf())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		str = str + "[Profiling CPUID against FYL2XP1]:  ";
		if (_Hypervisor.take_time_cpuid_against_fyl2xp1())
		{
			str = str + u8"×" + "\r\n";
		}
		else
		{
			str = str + u8"√" + "\r\n";
		}
		QMessageBox::information(this, "Ret", str.data());
	}
}

void Medusa::PdbMenu(QAction* action)
{
	if (action->text() == "Down&Load ntos")
	{
		_PDBView._PDBInfo.UnLoad();
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (!_PDBView._PDBInfo.DownLoadNtos())
		{
			ui.label->setText("");
			QMessageBox::information(this, "error", "error download pdb");
			return;
		}
		ui.progressBar->setValue(100);
		ui.label->setText("downlode pdb susscess");
		std::string pe_file_path = std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe";
		_PDBView.setWindowTitle(pe_file_path.data());
		return;
	}
	if (action->text() == "Down&Load file")
	{
		_PDBView._PDBInfo.UnLoad();
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (!_PDBView._PDBInfo.DownLoad(temp_str.toStdString(),false))
		{
			ui.label->setText("");
			QMessageBox::information(this, "error", "error download pdb");
			return;
		}
		ui.progressBar->setValue(100);
		ui.label->setText("downlode pdb susscess");
		return;
	}
	if (action->text() == "Down&Load file with baseaddr")
	{
		_PDBView._PDBInfo.UnLoad();
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		if (!_PDBView._PDBInfo.DownLoad(temp_str.toStdString(), true))
		{
			QMessageBox::information(this, "error", "error download pdb");
		}
		return;
	}
	if (action->text() == "Unload Symbol")
	{
		_PDBView._PDBInfo.UnLoad();
		return;
	}
	if (action->text() == "PdbView")
	{
		_PDBView._Model->removeRows(0, _PDBView._Model->rowCount());
		_PDBView.show();
		return;
	}
	if (action->text().toStdString().find("SSDT& SSSDT Use PDB") != std::string::npos)
	{
		_Setting_SSDT_SSSDT_PDB = !_Setting_SSDT_SSSDT_PDB;
		if (_Setting_SSDT_SSSDT_PDB)
		{
			action->setText(u8"√ SSDT& SSSDT Use PDB");
		}
		else
		{
			action->setText(u8"× SSDT& SSSDT Use PDB");
		}
	}
	if (action->text().toStdString().find("Use microsoft server") != std::string::npos)
	{
		action->setText(u8"√ Use microsoft server");
		ui.actionUse_order_server->setText(u8"× Use Order Server");
		_PDBView._PDBInfo._SymbolServer = "https://msdl.microsoft.com/download/symbols/";
	}
	if (action->text().toStdString().find("Use Order Server") != std::string::npos)
	{
		action->setText(u8"√ Use Order Server");
		ui.actionUse_microsoft_server->setText(u8"× Use microsoft server");
		_PDBView._PDBInfo._SymbolServer = "https://msdl.szdyg.cn/download/symbols/";
	}
	if (action->text() == "SendPDBInfo")
	{
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (_PDBView._PDBInfo.SendMedusaPDBInfo())
		{
			ui.progressBar->setValue(100);
			ui.label->setText("success load pdb");
			QMessageBox::information(this, "success", "pdb send");
		}
		else
		{
			ui.label->setText("error load pdb");
			QMessageBox::information(this, "error", "error");
		}
	}
}

void Medusa::ViewKernelMemory(QAction* action)
{
	if (action->text() == "View")
	{
		_KernelMemory.show();
	}
	if (action->text() == "PageTable")
	{
		_PageTable.show();
	}
}

void Medusa::DriverLoad(QAction* action)
{
	if (!_Driver_Loaded)
	{
		if (action->text() == "NtLoad")
		{
			std::fstream check_file("MedusaKernel.sys", std::ios::in);
			if (check_file.is_open())
			{
				check_file.close();

				char temp_str[MAX_PATH];
				if (GetCurrentDirectoryA(MAX_PATH, temp_str) > 0)
				{
					_Driver_Load.Init(temp_str + std::string("\\MedusaKernel.sys"));
					if (_Driver_Load.Nt_Register_Driver())
					{
						if (_Driver_Load.Nt_Start_Driver())
						{
							_Driver_Loaded = true;
							ChangeTab();
							QMessageBox::information(this, "success", "driver loaded");
							return;
						}
					}
				}
			}
			QMessageBox::information(this, "error", "cannot load driver");
		}
		if (action->text() == "NormalLoad")
		{
			std::fstream check_file("MedusaKernel.sys", std::ios::in);
			if (check_file.is_open())
			{
				check_file.close();

				char temp_str[MAX_PATH];
				if (GetCurrentDirectoryA(MAX_PATH, temp_str) > 0)
				{
					_Driver_Load.Init(temp_str + std::string("\\MedusaKernel.sys"));
					if (_Driver_Load.Register_Driver())
					{
						if (_Driver_Load.Start_Driver())
						{
							_Driver_Loaded = true;
							ChangeTab();
							QMessageBox::information(this, "success", "driver loaded");
							return;
						}
					}
				}
			}
			QMessageBox::information(this, "error", "cannot load driver");
		}
	}
	
	if (action->text() == "UnLoadMedusaDriver")
	{
		if (_Driver_Load._Driver_Name == "")
		{
			char szDownloadDir[MAX_PATH] = { 0 };
			GetCurrentDirectoryA(sizeof(szDownloadDir), szDownloadDir);
			_Driver_Load.Init(szDownloadDir + std::string("\\MedusaKernel.sys"));
		}
		if (_Driver_Load._NtModule)
		{
			if (_Driver_Load.Nt_Stop_Driver() && _Driver_Load.Nt_UnRegister_Driver())
			{
				_Driver_Loaded = false;
				QMessageBox::information(this, "success", "driver unload");
				exit(0);
			}
		}
		else
		{
			if (_Driver_Load.Stop_Driver() && _Driver_Load.UnRegister_Driver())
			{
				_Driver_Loaded = false;
				QMessageBox::information(this, "success", "driver unload");
				exit(0);
			}
			else
			{
				if (_Driver_Load.Nt_Stop_Driver() && _Driver_Load.Nt_UnRegister_Driver())
				{
					_Driver_Loaded = false;
					QMessageBox::information(this, "success", "driver unload");
					exit(0);
				}
			}
		}
		
	}



	//加载其他驱动
	if (action->text() == "Nt")
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		if (_Driver_Load_Other.Init(temp_str.toStdString()))
		{
			if (_Driver_Load_Other.Nt_Register_Driver())
			{
				if (_Driver_Load_Other.Nt_Start_Driver())
				{
					QMessageBox::information(this, "success", "driver loaded");
					return;
				}
			}
		}
	}
	if (action->text() == "Normal")
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);
		_Driver_Load_Other.Init(temp_str.toStdString());
		if (_Driver_Load_Other.Register_Driver())
		{
			if (_Driver_Load_Other.Start_Driver())
			{
				QMessageBox::information(this, "success", "driver loaded");
				return;
			}
		}
	}
	if (action->text() == "Unload")
	{
		if (_Driver_Load_Other._NtModule)
		{
			if (_Driver_Load_Other.Nt_Stop_Driver() && _Driver_Load_Other.Nt_UnRegister_Driver())
			{
				QMessageBox::information(this, "success", "driver unload");
			}
		}
		else
		{
			if (_Driver_Load_Other.Stop_Driver() && _Driver_Load_Other.UnRegister_Driver())
			{
				QMessageBox::information(this, "success", "driver unload");
			}
		}
	}
}








void Medusa::ProcessRightMenu(QAction* action)
{
	ULONG64 PID = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 1).data().toULongLong();
	if (action->text() == "R3CreateRemoteThread+LoadLibraryA" ||
		action->text() == "R3APCInject" ||
		action->text() == "R3MapInject" || 
		action->text() == "R3SetThreadContext+LoadLibrary" ||
		action->text() == "R0MapInject")
	{
		RightMenuDLLInject(action);
		return;
	}
	else if (action->text() == "QuickCheckALLProcess" || 
		action->text() == "HookScanner" || 
		action->text() == "HookScannerQuick" ||
		action->text() == "HookScannerSimple(Y/N)")
	{
		RightMenuHookScanner(action);
		return;
	}
	else if (action->text() == "R3ModulesView")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_Modules.setWindowTitle(process_name);
		_Modules.ShowUserMoudleList(PID, false);
		_Modules.show();
	}
	else if (action->text() == "R0ModulesView(second check)")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_Modules.setWindowTitle(process_name);
		if (_Driver_Loaded)
		{
			_Modules.ShowUserMoudleList(PID, true);
		}
		_Modules.show();
	}
	else if (action->text() == "R3ModuleScanner")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_Modules.setWindowTitle(process_name);
		_Modules.ShowUserMoudleScanner(PID, false);
		_Modules.show();
	}
	else if (action->text() == "R3ThreadView")
	{
		RightMenuR3ThreadsView(PID);
	}
	else if (action->text() == "R0ThreadView(second check)")
	{
		RightMenuR0ThreadsView(PID);
	}
	else if (action->text() == "R3KillProcess")
	{
		HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, PID);
		TerminateProcess(handle, 0);
	}
	else if (action->text() == "R0KillProcess")
	{
		
	}
	else if (action->text() == "_EPROCESS")
	{
		_PDBView._Model->removeRows(0, _PDBView._Model->rowCount());
		_PDBView._PDBInfo.UnLoad();
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (!_PDBView._PDBInfo.DownLoadNtos())
		{
			ui.label->setText("");
			QMessageBox::information(this, "error", "error download pdb");
			return;
		}
		ui.progressBar->setValue(100);
		ui.label->setText("downlode pdb susscess");
		std::string pe_file_path = std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe";
		_PDBView.setWindowTitle(pe_file_path.data());
		_PDBView.ui.lineEdit->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 3).data().toString());
		_PDBView.ui.lineEdit_2->setText("_EPROCESS");
		_PDBView.Serch();
		_PDBView.show();
		return;
	}
	else if (action->text() == "_KPROCESS")
	{
		_PDBView._Model->removeRows(0, _PDBView._Model->rowCount());
		_PDBView._PDBInfo.UnLoad();
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (!_PDBView._PDBInfo.DownLoadNtos())
		{
			ui.label->setText("");
			QMessageBox::information(this, "error", "error download pdb");
			return;
		}
		ui.progressBar->setValue(100);
		ui.label->setText("downlode pdb susscess");
		std::string pe_file_path = std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe";
		_PDBView.setWindowTitle(pe_file_path.data());
		_PDBView.ui.lineEdit->setText(ui.tableView->model()->index(ui.tableView->currentIndex().row(), 3).data().toString());
		_PDBView.ui.lineEdit_2->setText("_KPROCESS");
		_PDBView.Serch();
		_PDBView.show();
		return;
	}
	else if (action->text() == "ViewMemory")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_UserMemory.setWindowTitle(process_name);
		_UserMemory.PID = PID;
		_UserMemory.ui.label->setText("Process:"+
			ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString() +"    CR3:0x0" );
		_UserMemory.show();
	}
	
	if (action->text() == "MemoryListR3")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_UserMemoryList.setWindowTitle(process_name);
		_UserMemoryList.ShowUserMemoryListR3(PID, false);
		_UserMemoryList.show();
	}
	else if (action->text() == "MemoryListR3(second check)")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_UserMemoryList.setWindowTitle(process_name);
		_UserMemoryList.ShowUserMemoryListR3(PID, true);
		_UserMemoryList.show();
	}
	else if (action->text() == "MemoryListR0")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_UserMemoryListKernel.setWindowTitle(process_name);
		_UserMemoryListKernel.ShowUserMemoryListR0(PID);
		_UserMemoryListKernel.show();
	}
	else if (action->text() == "MemoryListR0_2")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_UserMemoryListKernel.setWindowTitle(process_name);
		_UserMemoryListKernel.ShowUserMemoryListR0_2(PID);
		_UserMemoryListKernel.show();
	}
	else if (action->text() == "MemoryListR0ALL")
	{
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_UserMemoryListKernel.setWindowTitle(process_name);
		_UserMemoryListKernel.ShowUserMemoryListR0ALL(PID);
		_UserMemoryListKernel.show();
	}
}

void Medusa::HideProcess(bool)
{
}

void Medusa::DriverRightMenu(QAction* action)
{
	if (action->text() == "_DRIVER_OBJECT")
	{
		_PDBView._Model->removeRows(0, _PDBView._Model->rowCount());
		_PDBView._PDBInfo.UnLoad();
		ui.label->setText("downloding pdb files..........................");
		ui.progressBar->setMaximum(100);
		ui.progressBar->setValue(5);
		QCoreApplication::processEvents();
		if (!_PDBView._PDBInfo.DownLoadNtos())
		{
			ui.label->setText("");
			QMessageBox::information(this, "error", "error download pdb");
			return;
		}
		ui.progressBar->setValue(100);
		ui.label->setText("downlode pdb susscess");
		std::string pe_file_path = std::string(std::getenv("systemroot")) + "\\System32\\ntoskrnl.exe";
		_PDBView.setWindowTitle(pe_file_path.data());
		_PDBView.ui.lineEdit->setText(ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 6).data().toString());
		_PDBView.ui.lineEdit_2->setText("_DRIVER_OBJECT");
		_PDBView.Serch();
		_PDBView.show();
		return;
	}
}

void Medusa::DriverRightMenuDumpToFILE(bool)
{
	std::string file_name = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 1).data().toString().toStdString();
	std::string addr_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 2).data().toString().toStdString();
	addr_str.erase(0, 2);
	ULONG64 addr = strtoull(addr_str.data(), 0, 16);
	std::string size_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 3).data().toString().toStdString();
	size_str.erase(0, 2);
	ULONG64 size = strtoull(size_str.data(), 0, 16);
	char* temp_buffer = new char[size];
	if (_KernelModules.DumpDriver(addr, size, temp_buffer))
	{
		peconv::t_pe_dump_mode dump_mode = peconv::PE_DUMP_UNMAP;
		if (peconv::dump_pe(C_TO_W(file_name).data(), (BYTE*)temp_buffer, size, 0, dump_mode))
		{
			QMessageBox::information(this, "Ret", "susscss");
		}
	}
	else
	{
		QMessageBox::information(this, "Ret", "error");
	}
	delete temp_buffer;
}

void Medusa::DriverRightMenuDumpToMemory(bool)
{
	std::string addr_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 2).data().toString().toStdString();
	addr_str.erase(0, 2);
	ULONG64 addr = strtoull(addr_str.data(), 0, 16);
	std::string size_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 3).data().toString().toStdString();
	size_str.erase(0, 2);
	ULONG64 size = strtoull(size_str.data(), 0, 16);
	char* temp_buffer = new char[size];
	if (_KernelModules.DumpDriver(addr, size, temp_buffer))
	{
		std::fstream temp_file(addr_str, std::ios::out | std::ios::binary);
		if (temp_file.is_open())
		{
			temp_file << std::string(temp_buffer, size);
			temp_file.close();
			QMessageBox::information(this, "Ret", "susscss");
		}
	}
	else
	{
		QMessageBox::information(this, "Ret", "error");
	}
	delete temp_buffer;
}\

void Medusa::DriverRightMenuViewExportFunc(bool)
{
	std::string file_name = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 4).data().toString().toStdString();
	std::string addr_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 2).data().toString().toStdString();
	addr_str.erase(0, 2);
	ULONG64 addr = strtoull(addr_str.data(), 0, 16);

	_ModuleExportFunc._PID = 0;
	_ModuleExportFunc._Previous = true;
	_ModuleExportFunc.GetExportFunc(addr, file_name);
	_ModuleExportFunc.show();
}

void Medusa::DriverRightMenuIOCTLScanner(QAction* action)
{
	if (action->text() == "ViewIOCTL-Functions")
	{
		std::string addr_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 6).data().toString().toStdString();
		addr_str.erase(0, 2);
		ULONG64 addr = strtoull(addr_str.data(), 0, 16);

		std::string module_name = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 1).data().toString().toStdString();

		_IOCTLScanner.GetIOCTLFunction(addr, _KernelModules, module_name);
		_IOCTLScanner.show();
	}
	if (action->text() == "ScanAllDriverIOCTLHook")
	{
		std::string ret = "detected hook:\r\n";
		for (auto x : _KernelModules._KernelModuleListR0)
		{
			if (x.DriverObject)
			{
				if (_IOCTLScanner.QueryIOCTLHook(x.DriverObject, _KernelModules, x.Name))
				{
					ret = ret + x.Name + "\r\n";
				}
			}
		}
		QMessageBox::information(this, "Ret", ret.data());
	}
	if (action->text() == "ScanIOCTLHook")
	{
		std::string addr_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 6).data().toString().toStdString();
		addr_str.erase(0, 2);
		ULONG64 addr = strtoull(addr_str.data(), 0, 16);

		std::string module_name = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 1).data().toString().toStdString();
		bool ret = _IOCTLScanner.QueryIOCTLHook(addr, _KernelModules, module_name);
		if (ret)
		{
			QMessageBox::information(this, "Ret", "detected hook");
		}
		else
		{
			QMessageBox::information(this, "Ret", "normal");
		}
	}
}

void Medusa::DriverRightMenuUnload(QAction* action)
{
	if (action->text() == "Unload(R3)")
	{
		Driver_Load _Driver_Load2;
		std::string path_str = 
			ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 4).data().toString().toStdString();
		_Driver_Load2.Init(path_str);
		if (_Driver_Load2.Stop_Driver())
		{
			QMessageBox::information(this, "success", "driver unload");
			return;
		}
		else
		{
			if (_Driver_Load2.Nt_Stop_Driver())
			{
				QMessageBox::information(this, "success", "driver unload");
				return;
			}
		}
		QMessageBox::information(this, "error", std::to_string(_Driver_Load2._Last_Error).data());
	}
	if (action->text() == "Unload(R0)")
	{
		std::string addr_str = ui.tableView_Driver->model()->index(ui.tableView_Driver->currentIndex().row(), 6).data().toString().toStdString();
		addr_str.erase(0, 2);
		ULONG64 addr = strtoull(addr_str.data(), 0, 16);
		if (_KernelModules.DriverUnload(addr))
		{
			QMessageBox::information(this, "success", "driver unload");
			return;
		}
		QMessageBox::information(this, "error", "error");
	}
}

void Medusa::ChangeTab()
{
	if (ui.tabWidget->currentIndex() == 0)
	{
		GetProcessList();
	}
	else if (ui.tabWidget->currentIndex() == 1)
	{
		GetKernelModuleList();
	}
	else if (ui.tabWidget->currentIndex() == 2)
	{
		GetUnLoadKernelModuleList();
	}
	else if (ui.tabWidget->currentIndex() == 3)
	{
		GetALLCallBackList();
	}
	else if (ui.tabWidget->currentIndex() == 4)
	{
		GetSSDT();
	}
	else if (ui.tabWidget->currentIndex() == 5)
	{
		GetShadowSSDT();
	}
}





void Medusa::RightMenuDLLInject(QAction* action)
{
	ULONG64 PID = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 1).data().toULongLong();
	bool ret = false;
	if (action->text() == "R3CreateRemoteThread+LoadLibraryA")
	{
		DLLInject _DLLInject;
		ret = _DLLInject.R3CreateThread(PID);
		if (ret)
		{
			QMessageBox::information(this, "Ret", "susscss");
		}
		else
		{
			QMessageBox::information(this, "Ret", "error");
		}
	}
	if (action->text() == "R3APCInject")
	{
		DLLInject _DLLInject;
		ret = _DLLInject.R3APCInject(PID);
		if (ret)
		{
			QMessageBox::information(this, "Ret", "susscss");
		}
		else
		{
			QMessageBox::information(this, "Ret", "error");
		}
	}
	if (action->text() == "R3MapInject")
	{
		DLLInject _DLLInject;
		ret = _DLLInject.R3MapInject(PID);
		if (ret)
		{
			QMessageBox::information(this, "Ret", "susscss");
		}
		else
		{
			QMessageBox::information(this, "Ret", "error");
		}
	}
	if (action->text() == "R3SetThreadContext+LoadLibrary")
	{
		QFileDialog file_path;
		QString temp_str = file_path.getOpenFileName();
		if (temp_str.size() == 0)
		{
			return;
		}
		temp_str = QDir::toNativeSeparators(temp_str);

		DLLInject _DLLInject;
		ret = _DLLInject.injectdll(PID,C_TO_W(temp_str.toStdString()));
		if (ret)
		{
			QMessageBox::information(this, "Ret", "susscss");
		}
		else
		{
			QMessageBox::information(this, "Ret", "error");
		}
	}
	if (action->text() == "R0MapInject")
	{
		DLLInject _DLLInject;
		ret = _DLLInject.R0MapInject(PID);
		if (ret)
		{
			QMessageBox::information(this, "Ret", "susscss");
		}
		else
		{
			QMessageBox::information(this, "Ret", "error");
		}
	}
}

void Medusa::RightMenuHookScanner(QAction* action)
{
	ULONG64 PID = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 1).data().toULongLong();
	if (action->text() == "QuickCheckALLProcess")
	{
		FileCheck temp_check(_Driver_Loaded);
		std::string temp_str;
		ui.progressBar->setMaximum(_Process._Process_List_R3.size());
		ui.progressBar->setValue(0);
		for (auto x : _Process._Process_List_R3)
		{
			if (x.PID == 0 || x.PID == 4)
			{
				continue;
			}
			int temp_ret = temp_check.CheckSimple(x.PID);
			if (temp_ret == -1)
			{
				temp_str = temp_str + QString::fromWCharArray(x.Name).toStdString() + "-----Insufficient permissions or file miss\r\n";
			}
			if (temp_ret == 0)
			{
				temp_str = temp_str + QString::fromWCharArray(x.Name).toStdString() + "-----detected hook\r\n";
			}
			ui.progressBar->setValue(ui.progressBar->value() + 1);
		}
		ui.progressBar->setValue(_Process._Process_List_R3.size());
		QMessageBox::information(this, "Ret", temp_str.data());
	}
	if (action->text() == "HookScannerSimple(Y/N)")
	{
		if (PID == 0
			|| PID == 4)
		{
			return;
		}
		FileCheck temp_check(_Driver_Loaded);
		int temp_ret = temp_check.CheckSimple(PID);
		if (temp_ret == 1)
		{
			QMessageBox::information(this, "Ret", "not detected hook");
		}
		else if (temp_ret == 0)
		{
			QMessageBox::information(this, "Ret", "detected hook");
		}
		else
		{
			QMessageBox::information(this, "Ret", "Insufficient permissions or file miss");
		}
	}
	if (action->text() == "HookScanner")
	{
		if (PID == 0
			|| PID == 4)
		{
			return;
		}
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_HookScanner.setWindowTitle(process_name);
		_HookScanner.ProcessHookScanner(PID, _Driver_Loaded);
		_HookScanner.show();
	}
	if (action->text() == "HookScannerQuick")
	{
		if (PID == 0
			|| PID == 4)
		{
			return;
		}
		QString process_name = ui.tableView->model()->index(ui.tableView->currentIndex().row(), 2).data().toString();
		_HookScanner.setWindowTitle(process_name);
		_HookScanner.ProcessHookScannerQuick(PID, _Driver_Loaded);
		_HookScanner.show();
	}
}

void Medusa::RightMenuR3ThreadsView(ULONG64 PID)
{
	_Threads._Model->removeRows(0, _Threads._Model->rowCount());
	std::vector<ThreadList> temp_vector = _Threads.GetThreadListR3(PID);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Threads._Model->setVerticalHeaderItem(i, new QStandardItem);
		_Threads._Model->setData(_Threads._Model->index(i, 0), i);
		_Threads._Model->setData(_Threads._Model->index(i, 1), x.TID);
		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.StartAddr;
		_Threads._Model->setData(_Threads._Model->index(i, 3), ret.str().data());
		_Threads._Model->setData(_Threads._Model->index(i, 4), QString::fromWCharArray(x.Name));
		i++;
	}
	_Threads.show();
}

void Medusa::RightMenuR0ThreadsView(ULONG64 PID)
{
	if (_Driver_Loaded)
	{
		_Threads._Model->removeRows(0, _Threads._Model->rowCount());
		std::vector<ThreadList> temp_vector = _Threads.GetThreadListR0(PID);
		std::vector<ThreadList> temp_vector2 = _Threads.GetThreadListR3(PID);
		int i = 0;
		for (auto x : temp_vector)
		{
			bool found = false;
			for (auto y : temp_vector2)
			{
				if (x.TID == y.TID)
				{
					found = true;
					break;
				}
			}
			_Threads._Model->setVerticalHeaderItem(i, new QStandardItem);
			_Threads._Model->setData(_Threads._Model->index(i, 0), i);
			_Threads._Model->setData(_Threads._Model->index(i, 1), x.TID);
			std::ostringstream ret2;
			ret2 << std::hex << "0x" << (ULONG64)x.ETHREAD;
			_Threads._Model->setData(_Threads._Model->index(i, 2), ret2.str().data());
			std::ostringstream ret;
			ret << std::hex << "0x" << (ULONG64)x.StartAddr;
			_Threads._Model->setData(_Threads._Model->index(i, 3), ret.str().data());
			_Threads._Model->setData(_Threads._Model->index(i, 4), QString::fromWCharArray(x.Name));
			if (!found)
			{
				_Threads._Model->item(i, 0)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 1)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 2)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 3)->setBackground(QColor(Qt::red));
				_Threads._Model->item(i, 4)->setBackground(QColor(Qt::red));
			}
			i++;
		}
		_Threads.show();
	}
}





void Medusa::GetProcessList()
{
	_Model->removeRows(0, _Model->rowCount());
	_Process.GetProcessList(_Driver_Loaded);
	int i = 0;
	if (_Driver_Loaded)
	{
		for (auto x : _Process._Process_List_R0)
		{
			bool found = false;
			PROCESS_LIST temp_list;

			int j = 0;
			for (; j < _Process._Process_List_R3.size(); j++)
			{
				if (_Process._Process_List_R3.at(j).PID == x.PID)
				{
					found = true;
					break;
				}
			}
			if (found)
			{
				_Model->setVerticalHeaderItem(i, new QStandardItem);
				_Model->setData(_Model->index(i, 0), i);
				_Model->setData(_Model->index(i, 1), _Process._Process_List_R3.at(j).PID);
				_Model->setData(_Model->index(i, 2), QString::fromWCharArray(_Process._Process_List_R3.at(j).Name));
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.EPROCESS;
				_Model->setData(_Model->index(i, 3), ret.str().data());
				if (std::wstring(_Process._Process_List_R3.at(j).Path) != L"")
				{
					_Model->setData(_Model->index(i, 4), QString::fromWCharArray(_Process._Process_List_R3.at(j).Path));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", _Process._Process_List_R3.at(j).Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
					}
				}
				else
				{
					_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.Path));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
					}
				}
			}
			else
			{
				_Model->setVerticalHeaderItem(i, new QStandardItem);
				_Model->setData(_Model->index(i, 0), i);
				_Model->setData(_Model->index(i, 1), x.PID);
				_Model->setData(_Model->index(i, 2), (char*)x.Name);
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.EPROCESS;
				_Model->setData(_Model->index(i, 3), ret.str().data());
				_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.Path));


				if (x.Check == false)
				{
					_Model->item(i, 0)->setBackground(QColor(Qt::red));
					_Model->item(i, 1)->setBackground(QColor(Qt::red));
					_Model->item(i, 2)->setBackground(QColor(Qt::red));
					_Model->item(i, 3)->setBackground(QColor(Qt::red));
					_Model->item(i, 4)->setBackground(QColor(Qt::red));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
						_Model->item(i, 5)->setBackground(QColor(Qt::red));
					}
				}
				else
				{
					_Model->item(i, 0)->setBackground(QColor(Qt::green));
					_Model->item(i, 1)->setBackground(QColor(Qt::green));
					_Model->item(i, 2)->setBackground(QColor(Qt::green));
					_Model->item(i, 3)->setBackground(QColor(Qt::red));
					_Model->item(i, 4)->setBackground(QColor(Qt::green));
					std::wstring retStr;
					if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
					{
						_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
						_Model->item(i, 5)->setBackground(QColor(Qt::green));
					}
				}
			}



			i++;
		}
		ui.label->setText(QString((std::string("R3 get process number:") +
			std::to_string(_Process._Process_List_R3.size()) +
			std::string("=====R0 get process number:") +
			std::to_string(_Process._Process_List_R0.size())).data()));
	}
	else
	{
		for (auto x : _Process._Process_List_R3)
		{
			_Model->setVerticalHeaderItem(i, new QStandardItem);
			_Model->setData(_Model->index(i, 0), i);
			_Model->setData(_Model->index(i, 1), x.PID);
			_Model->setData(_Model->index(i, 2), QString::fromWCharArray(x.Name));
			_Model->setData(_Model->index(i, 4), QString::fromWCharArray(x.Path));

			std::wstring retStr;
			if (_Process.QueryValue(L"FileDescription", x.Path, retStr))
			{
				_Model->setData(_Model->index(i, 5), QString::fromWCharArray(retStr.data()));
			}
			i++;
		}

		ui.label->setText(QString((std::string("R3 get process number:") +
			std::to_string(_Process._Process_List_R3.size())).data()));
	}
}

void Medusa::GetKernelModuleList()
{
	_Model_Driver->removeRows(0, _Model_Driver->rowCount());
	if (_Driver_Loaded)
	{
		_KernelModules.GetKernelModuleListR0();
		if (_KernelModules._KernelModuleListR0.size() != 0)
		{
			int i = 0;
			for (auto x : _KernelModules._KernelModuleListR0)
			{
				_Model_Driver->setVerticalHeaderItem(i, new QStandardItem);
				_Model_Driver->setData(_Model_Driver->index(i, 0), i);
				_Model_Driver->setData(_Model_Driver->index(i, 1), x.Name.data());
				
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.Addr;
				_Model_Driver->setData(_Model_Driver->index(i, 2), ret.str().data());
				std::ostringstream ret2;
				ret2 << std::hex << "0x" << (ULONG64)x.Size;
				_Model_Driver->setData(_Model_Driver->index(i, 3), ret2.str().data());
				
				std::string temp_str = x.Path;
				temp_str = ReplaceStr2(temp_str, "\\SystemRoot\\", "C:\\Windows\\");
				temp_str = ReplaceStr2(temp_str, "\\??\\", "");
				_Model_Driver->setData(_Model_Driver->index(i, 4), temp_str.data());

				std::wstring retStr;
				std::wstring temp_wstr = C_TO_W(x.Path);
				temp_wstr = ReplaceStr(temp_wstr, L"\\SystemRoot\\", L"C:\\Windows\\");
				temp_wstr = ReplaceStr(temp_wstr, L"\\??\\", L"");
				if (_Process.QueryValue(L"FileDescription", temp_wstr.data(), retStr))
				{
					_Model_Driver->setData(_Model_Driver->index(i, 5), QString::fromWCharArray(retStr.data()));
				}
				else
				{
					_Model_Driver->setData(_Model_Driver->index(i, 5), "");
				}

				std::ostringstream ret3;
				ret3 << std::hex << "0x" << (ULONG64)x.DriverObject;
				_Model_Driver->setData(_Model_Driver->index(i, 6), ret3.str().data());

				QColor temp_color = QColor(Qt::white);
				if (x.Check == 1)
				{
					temp_color = QColor(Qt::green);
				}
				if (x.Check == 2)
				{
					temp_color = QColor(Qt::red);
				}
				if (x.Check == 3)
				{
					temp_color = QColor(Qt::red);
				}
				_Model_Driver->item(i, 0)->setBackground(temp_color);
				_Model_Driver->item(i, 1)->setBackground(temp_color);
				_Model_Driver->item(i, 2)->setBackground(temp_color);
				_Model_Driver->item(i, 3)->setBackground(temp_color);
				_Model_Driver->item(i, 4)->setBackground(temp_color);
				_Model_Driver->item(i, 5)->setBackground(temp_color);
				i++;
			}
			ui.label->setText(QString((std::string("R0 get kernel moduls number:") +
				std::to_string(_KernelModules._KernelModuleListR0.size())).data()));

		}
	}
	else
	{
		KernelModules _KernelModules;
		_KernelModules.GetKernelModuleListR3();
		if (_KernelModules._KernelModuleListR3.size() != 0)
		{
			int i = 0;
			for (auto x: _KernelModules._KernelModuleListR3)
			{
				_Model_Driver->setVerticalHeaderItem(i, new QStandardItem);
				_Model_Driver->setData(_Model_Driver->index(i, 0), i);
				_Model_Driver->setData(_Model_Driver->index(i, 1), x.Name.data());
				std::ostringstream ret;
				ret << std::hex << "0x" << (ULONG64)x.Addr;
				_Model_Driver->setData(_Model_Driver->index(i, 2), ret.str().data());
				std::ostringstream ret2;
				ret2 << std::hex << "0x" << (ULONG64)x.Size;
				_Model_Driver->setData(_Model_Driver->index(i, 3), ret2.str().data());

				std::string temp_str = x.Path;
				temp_str = ReplaceStr2(temp_str, "\\SystemRoot\\", "C:\\Windows\\");
				temp_str = ReplaceStr2(temp_str, "\\??\\", "");
				_Model_Driver->setData(_Model_Driver->index(i, 4), temp_str.data());

				std::wstring temp_wstr = C_TO_W(x.Path);
				if (temp_wstr.find(L"SystemRoot") != std::wstring::npos)
				{
					temp_wstr = ReplaceStr(temp_wstr, L"\\SystemRoot\\", L"C:\\Windows\\");
					temp_wstr = ReplaceStr(temp_wstr, L"\\??\\", L"");
				}
				std::wstring retStr;
				if (_Process.QueryValue(L"FileDescription", temp_wstr.data(), retStr))
				{
					_Model_Driver->setData(_Model_Driver->index(i, 5), QString::fromWCharArray(retStr.data()));
				}
				i++;
			}
			ui.label->setText(QString((std::string("R3 get kernel moduls number:") +
				std::to_string(_KernelModules._KernelModuleListR3.size())).data()));
		}
	}
}

void Medusa::GetUnLoadKernelModuleList()
{
	_Model_UnloadDriver->removeRows(0, _Model_UnloadDriver->rowCount());
	_KernelModules.GetUnLoadKernelModuleListR0();
	if (!_KernelModules._KernelUnLoadModuleListR0.size())
	{
		return;
	}
	int i = 0;
	for (auto x : _KernelModules._KernelUnLoadModuleListR0)
	{
		_Model_UnloadDriver->setVerticalHeaderItem(i, new QStandardItem);
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 0), i);
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 1), QString::fromWCharArray((WCHAR*)x.Name));

		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.Addr;
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 2), ret.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << (ULONG64)x.Size;
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 3), ret2.str().data());
		std::ostringstream ret3;
		ret3 << std::hex << "0x" << (ULONG64)x.UnLoadTime;
		_Model_UnloadDriver->setData(_Model_UnloadDriver->index(i, 4), ret3.str().data());


		QColor temp_color = QColor(Qt::white);
		if (x.Check == 1)
		{
			temp_color = QColor(Qt::red);
		}
		if (x.Check == 2)
		{
			temp_color = QColor(Qt::red);
		}
		_Model_UnloadDriver->item(i, 0)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 1)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 2)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 3)->setBackground(temp_color);
		_Model_UnloadDriver->item(i, 4)->setBackground(temp_color);

		i++;
	}
}

void Medusa::GetALLCallBackList()
{
	_Model_CallBackList->removeRows(0, _Model_CallBackList->rowCount());
	KernelCallBackScanner _KernelCallBackScanner;
	_KernelCallBackScanner.GetALLCallBackList();
	if (!_KernelCallBackScanner.__KernelCallBackListR0.size())
	{
		return;
	}
	if (_KernelModules._KernelModuleListR0.size() == 0)
	{
		_KernelModules.GetKernelModuleListR0();
	}
	int i = 0;
	for (auto x : _KernelCallBackScanner.__KernelCallBackListR0)
	{
		_Model_CallBackList->setVerticalHeaderItem(i, new QStandardItem);
		_Model_CallBackList->setData(_Model_CallBackList->index(i, 0), i);


		std::string module_name;
		bool found = false;
		for (auto y : _KernelModules._KernelModuleListR0)
		{
			if (x.PreOperation >= (ULONG64)y.Addr &&
				x.PreOperation < (ULONG64)y.Addr + (ULONG64)y.Size)
			{
				module_name = y.Name;
				found = true;
				module_name = module_name + "+";
				std::ostringstream ret;
				ret << std::hex << "0x" << x.PreOperation - y.Addr;
				module_name = module_name + ret.str() + "   ";
			}
			if (x.PostOperation!=0 &&
				x.PostOperation >= (ULONG64)y.Addr &&
				x.PostOperation < (ULONG64)y.Addr + (ULONG64)y.Size)
			{
				module_name = y.Name;
				module_name = module_name + "+";
				std::ostringstream ret;
				ret << std::hex << "0x" << x.PostOperation - y.Addr;
				module_name = module_name + ret.str() + "   ";
			}
		}
		_Model_CallBackList->setData(_Model_CallBackList->index(i, 1), module_name.data());

		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.PreOperation;
		_Model_CallBackList->setData(_Model_CallBackList->index(i, 2), ret.str().data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << (ULONG64)x.PostOperation;
		_Model_CallBackList->setData(_Model_CallBackList->index(i, 3), ret2.str().data());
		std::ostringstream ret3;
		ret3 << std::hex << "0x" << (ULONG64)x.CallbackEntry;
		_Model_CallBackList->setData(_Model_CallBackList->index(i, 5), ret3.str().data());

		std::string callback_name;
		if (x.Type == 1)
		{
			callback_name = "PsProcessType";
		}
		if (x.Type == 2)
		{
			callback_name = "PsThreadType";
		}
		if (x.Type == 3)
		{
			callback_name = "LoadImageNotifyRoutine";
		}
		if (x.Type == 4)
		{
			callback_name = "ProcessNotifyRoutine";
		}
		if (x.Type == 5)
		{
			callback_name = "ThreadNotifyRoutine";
		}
		_Model_CallBackList->setData(_Model_CallBackList->index(i, 4), callback_name.data());

		QColor temp_color = QColor(Qt::white);
		if (!found)
		{
			temp_color = QColor(Qt::red);
		}
		_Model_CallBackList->item(i, 0)->setBackground(temp_color);
		_Model_CallBackList->item(i, 1)->setBackground(temp_color);
		_Model_CallBackList->item(i, 2)->setBackground(temp_color);
		_Model_CallBackList->item(i, 3)->setBackground(temp_color);
		_Model_CallBackList->item(i, 4)->setBackground(temp_color);
		_Model_CallBackList->item(i, 5)->setBackground(temp_color);

		i++;
	}
}

void Medusa::GetSSDT()
{
	if (!_Driver_Loaded)
	{
		return;
	}
	_Model_SSDT->removeRows(0, _Model_SSDT->rowCount());
	std::vector<SSDT_STRUCT2> temp_vector = _KernelModules.GetALLSSDT(_Setting_SSDT_SSSDT_PDB);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Model_SSDT->setVerticalHeaderItem(i, new QStandardItem);
		_Model_SSDT->setData(_Model_SSDT->index(i, 0), i);
		_Model_SSDT->setData(_Model_SSDT->index(i, 1), x.FuncName.data());
		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.Addr;
		_Model_SSDT->setData(_Model_SSDT->index(i, 2), ret.str().data());
		_Model_SSDT->setData(_Model_SSDT->index(i, 3), x.Modules.data());
		i++;
	}
}

void Medusa::GetShadowSSDT()
{
	if (!_Driver_Loaded)
	{
		return;
	}
	_Model_SSSDT->removeRows(0, _Model_SSSDT->rowCount());
	std::vector<SSDT_STRUCT2> temp_vector = _KernelModules.GetALLShadowSSDT(_Setting_SSDT_SSSDT_PDB);
	int i = 0;
	for (auto x : temp_vector)
	{
		_Model_SSSDT->setVerticalHeaderItem(i, new QStandardItem);
		_Model_SSSDT->setData(_Model_SSSDT->index(i, 0), i);
		_Model_SSSDT->setData(_Model_SSSDT->index(i, 1), x.FuncName.data());
		std::ostringstream ret;
		ret << std::hex << "0x" << (ULONG64)x.Addr;
		_Model_SSSDT->setData(_Model_SSSDT->index(i, 2), ret.str().data());
		_Model_SSSDT->setData(_Model_SSSDT->index(i, 3), x.Modules.data());
		i++;
	}
}