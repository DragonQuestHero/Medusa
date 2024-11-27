#include "ModuleExportFunc.h"


#include "libpeconv/peconv.h"
#include "libpeconv/peconv/pe_loader.h"
#include "libpeconv/peconv/pe_hdrs_helper.h"


ModuleExportFunc::ModuleExportFunc(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	_Model = new QStandardItemModel();
	ui.tableView->setModel(_Model);
	ui.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableView->horizontalHeader()->setSectionsClickable(false);
	ui.tableView->verticalHeader()->setDefaultSectionSize(25);
	ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableView->setContextMenuPolicy(Qt::ActionsContextMenu);

	_Model->setColumnCount(4);
	_Model->setHeaderData(0, Qt::Horizontal, u8"Index");
	_Model->setHeaderData(1, Qt::Horizontal, u8"Name");
	_Model->setHeaderData(2, Qt::Horizontal, u8"Addr");
	_Model->setHeaderData(3, Qt::Horizontal, u8"RVA");

	ui.tableView->setColumnWidth(0, 50);
	ui.tableView->setColumnWidth(1, 250);
	ui.tableView->setColumnWidth(2, 200);
	ui.tableView->setColumnWidth(3, 200);

	_TableView_Action_MemoryView.setText("MemoryView");
	ui.tableView->addAction(&_TableView_Action_MemoryView);
	connect(&_TableView_Action_MemoryView, SIGNAL(triggered(bool)), SLOT(MemoryView(bool)));
}



std::vector<ExportFunc> ModuleExportFunc::GetExportFunc(ULONG64 Addr, std::string Path)
{
	_Model->removeRows(0, _Model->rowCount());

	std::vector<ExportFunc> temp_vector;

	//std::string module_file = Read_ALL(Path);
	BYTE* data = nullptr;
	BYTE* loaded_pe = nullptr;
	size_t v_size = 0;
	size_t bufsize = 0;
	BYTE* buffer = peconv::load_file(C_TO_W(Path).data(), bufsize);
	if (!buffer) return temp_vector;
	loaded_pe = peconv::load_pe_module(buffer, bufsize, v_size, false, true);
	peconv::free_unaligned(buffer);
	if (!loaded_pe) return temp_vector;
	/*bool is64b = peconv::is64bit(loaded_pe);
	if (!is64b) return temp_vector;*/

	void* pBase = (void*)loaded_pe;


	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)pBase;
	PIMAGE_NT_HEADERS32 pNtHdr32 = NULL;
	PIMAGE_NT_HEADERS64 pNtHdr64 = NULL;
	PIMAGE_EXPORT_DIRECTORY pExport = NULL;
	ULONG expSize = 0;
	PUSHORT pAddressOfOrds;
	PULONG pAddressOfNames;
	PULONG pAddressOfFuncs;
	ULONG i;

	if (pBase == NULL)
	{
		peconv::free_pe_buffer(loaded_pe);
		return temp_vector;
	}

	// Not a PE file
	if (pDosHdr->e_magic != IMAGE_DOS_SIGNATURE)
	{
		peconv::free_pe_buffer(loaded_pe);
		return temp_vector;
	}

	pNtHdr32 = (PIMAGE_NT_HEADERS32)((PUCHAR)pBase + pDosHdr->e_lfanew);
	pNtHdr64 = (PIMAGE_NT_HEADERS64)((PUCHAR)pBase + pDosHdr->e_lfanew);

	// Not a PE file
	if (pNtHdr32->Signature != IMAGE_NT_SIGNATURE)
	{
		peconv::free_pe_buffer(loaded_pe);
		return temp_vector;
	}

	// 64-bit image
	if (pNtHdr32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		pExport = (PIMAGE_EXPORT_DIRECTORY)(pNtHdr64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + (ULONG_PTR)pBase);
		expSize = pNtHdr64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	}
	// 32-bit image
	else
	{
		pExport = (PIMAGE_EXPORT_DIRECTORY)(pNtHdr32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress + (ULONG_PTR)pBase);
		expSize = pNtHdr32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
	}

	pAddressOfOrds = (PUSHORT)(pExport->AddressOfNameOrdinals + (ULONG_PTR)pBase);
	pAddressOfNames = (PULONG)(pExport->AddressOfNames + (ULONG_PTR)pBase);
	pAddressOfFuncs = (PULONG)(pExport->AddressOfFunctions + (ULONG_PTR)pBase);

	if ((ULONG64)pAddressOfNames == (ULONG_PTR)pBase)
	{
		peconv::free_pe_buffer(loaded_pe);
		return temp_vector;
	}

	for (int i = 0; i < pExport->NumberOfFunctions; ++i)
	{
		//ULONG_PTR funcRVA = pAddressOfFuncs[i];
		//ULONG_PTR funcOffset = funcRVA - pNtHdr32->OptionalHeader.ImageBase;
		WORD* nameIndex = (WORD*)(pExport->AddressOfNameOrdinals + (BYTE*)loaded_pe + i * sizeof(WORD));
		DWORD* funcRVA = (DWORD*)(pExport->AddressOfFunctions + (BYTE*)loaded_pe + (*nameIndex) * sizeof(DWORD));

		// 获取函数名
		PCHAR funcName = (PCHAR)(pAddressOfNames[i] + (ULONG_PTR)pBase);

		ExportFunc temp_ExportFunc = { 0 };
		temp_ExportFunc.RVA = *funcRVA;
		if (Addr)
		{
			temp_ExportFunc.Addr = Addr + *funcRVA;
		}
		temp_ExportFunc.Name = funcName;
		
		_Model->setVerticalHeaderItem(i, new QStandardItem);
		_Model->setData(_Model->index(i, 0), i);
		_Model->setData(_Model->index(i, 1), temp_ExportFunc.Name.data());
		std::ostringstream ret2;
		ret2 << std::hex << "0x" << temp_ExportFunc.Addr;
		_Model->setData(_Model->index(i, 2), ret2.str().data());
		std::ostringstream ret;
		ret << std::hex << "0x" << temp_ExportFunc.RVA;
		_Model->setData(_Model->index(i, 3), ret.str().data());

		temp_vector.push_back(temp_ExportFunc);
	}
	peconv::free_pe_buffer(loaded_pe);
	return temp_vector;
}

