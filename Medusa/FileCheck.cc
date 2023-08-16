#include "FileCheck.h"

#include "Modules.h"

#include "libpeconv/peconv.h"
#include "libpeconv/peconv/pe_loader.h"
#include "libpeconv/peconv/pe_hdrs_helper.h"

#include <QMessageBox>



DWORD GetSections(char* pe, std::vector<PIMAGE_SECTION_HEADER>& sections) {
	if (!pe) return 0;
	const PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(pe);
	const PIMAGE_NT_HEADERS nt = reinterpret_cast<PIMAGE_NT_HEADERS>(pe + dos->e_lfanew);
	uint32_t numSec = nt->FileHeader.NumberOfSections;
	DWORD count = 0u;
	auto section = IMAGE_FIRST_SECTION(nt);
	for (auto i = 0u; i < numSec; i++, section++) 
	{ 
		if (section->Characteristics & IMAGE_SCN_CNT_CODE) 
		{ 
			sections.push_back(section); 
			count++; 
		} 
	}
	return count;
}

DWORD LoadMem(const HANDLE& proc, const MODULEENTRY32W& modEntry, BYTE*& data) {
	if (!proc || data || !modEntry.modBaseSize) return 0;
	data = new BYTE[modEntry.modBaseSize];
	SIZE_T size = 0;
	bool status = false;
	goto code;
cleanup:
	if (!status) { delete[] data; data = nullptr; }
	return size;
code:
	if (!ReadProcessMemory(proc, modEntry.modBaseAddr, data, modEntry.modBaseSize, &size)) {
		if (GetLastError() == ERROR_PARTIAL_COPY) {
			if (ReadProcessMemory(proc, modEntry.modBaseAddr+ size, data+ size, modEntry.modBaseSize-size, &size)) { status = true; };
		}
		goto cleanup;
	};
	status = true;
	goto cleanup;
}



bool FileCheck::CheckSimple(ULONG64 PID)
{
	auto proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, PID);
	if (!proc)
	{
		//QMessageBox::information(nullptr, "Error", (std::string("GetLastError:") + std::to_string(GetLastError())).data());
		return false;
	}

	Modules _Module;
	std::vector<MODULEENTRY32W> temp_vector = _Module.GetWin32MoudleList(PID);
	if (temp_vector.size() == 0)
	{
		return false;
	}

	int sucss = 0;
	for (auto x : temp_vector)
	{
		BYTE* data = nullptr;
		BYTE* loaded_pe = nullptr;
		do
		{
			/*HANDLE file = CreateFileW(x.szExePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (file == INVALID_HANDLE_VALUE) break;
			HANDLE mapping = CreateFileMappingA(file, 0, PAGE_READONLY | SEC_IMAGE, 0, 0, 0);
			if (mapping == INVALID_HANDLE_VALUE) break;
			char* cold = (char*)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
			if (!cold) break;*/
			size_t v_size = 0;
			size_t bufsize = 0;
			BYTE* buffer = peconv::load_file(x.szExePath, bufsize);
			if (!buffer) break;
			loaded_pe = peconv::load_pe_module(buffer, bufsize, v_size, false, true);
			peconv::free_unaligned(buffer);
			if (!loaded_pe) break;
			/*bool is64b = peconv::is64bit(loaded_pe);
			if (!is64b) break;*/
			

			std::vector<PIMAGE_SECTION_HEADER> coldSections;
			if (!GetSections((char*)loaded_pe, coldSections)) break;
			if (!LoadMem(proc, x, data)) break;
			std::vector<PIMAGE_SECTION_HEADER> hotSections;
			if (!GetSections((char*)data, hotSections)) break;
			if (hotSections.size() != coldSections.size()) break;

			bool text_check = false;
			for (int i = 0; i < hotSections.size(); i++)
			{
				PIMAGE_SECTION_HEADER temp_header = hotSections.at(i);
				PIMAGE_SECTION_HEADER temp_header2 = coldSections.at(i);
				if (temp_header2->Misc.VirtualSize!= temp_header->Misc.VirtualSize)
				{
					text_check = false;
					break;
				}
				if (RtlEqualMemory((void*)(data + temp_header->VirtualAddress),
					(void*)(loaded_pe + temp_header2->VirtualAddress), temp_header2->Misc.VirtualSize))
				{
					text_check = true;
				}
				else
				{
					text_check = false;
					break;
				}

			}
			if (text_check)
			{
				sucss++;
			}
		} while (false);
		if (data)
		{
			delete data;
			data = nullptr;
		}
		if (loaded_pe)
		{
			peconv::free_pe_buffer(loaded_pe);
			loaded_pe = nullptr;
		}
	}
	if (sucss == temp_vector.size())
	{
		return true;
	}
	return false;
}


std::vector<_CheckDifferent> FileCheck::CheckPlain(ULONG64 PID)
{
	std::vector<_CheckDifferent> temp_vector_check;

	auto proc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, PID);
	if (!proc)
	{
		QMessageBox::information(nullptr, "Error", (std::string("GetLastError:") + std::to_string(GetLastError())).data());
		return temp_vector_check;
	}

	Modules _Module;
	std::vector<MODULEENTRY32W> temp_vector = _Module.GetWin32MoudleList(PID);
	if (temp_vector.size() == 0)
	{
		return temp_vector_check;
	}

	int sucss = 0;
	for (auto x : temp_vector)
	{
		BYTE* data = nullptr;
		BYTE* loaded_pe = nullptr;
		std::vector<PIMAGE_SECTION_HEADER> coldSections;
		std::vector<PIMAGE_SECTION_HEADER> hotSections;

		bool already = false;
		do
		{
			size_t v_size = 0;
			size_t bufsize = 0;
			BYTE* buffer = peconv::load_file(x.szExePath, bufsize);
			if (!buffer) break;
			loaded_pe = peconv::load_pe_module(buffer, bufsize, v_size, false, true);
			peconv::free_unaligned(buffer);
			if (!loaded_pe) break;
			/*bool is64b = peconv::is64bit(loaded_pe);
			if (!is64b) return temp_vector_check;*/
			

			if (!GetSections((char*)loaded_pe, coldSections)) break;
			if (!LoadMem(proc, x, data)) break;
			if (!GetSections((char*)data, hotSections)) break;
			if (hotSections.size() != coldSections.size()) break;
			already = true;
		} while (false);
		if (!already)
		{
			if (data)
			{
				delete data;
				data = nullptr;
			}
			if (loaded_pe)
			{
				peconv::free_pe_buffer(loaded_pe);
				loaded_pe = nullptr;
			}
			_CheckDifferent temp__CheckDifferent;
			RtlZeroMemory(&temp__CheckDifferent, sizeof(_CheckDifferent));
			temp__CheckDifferent.Fail = true;
			RtlCopyMemory(temp__CheckDifferent.Name, x.szModule, 256);
			RtlCopyMemory(temp__CheckDifferent.Path, x.szExePath, MAX_PATH);
			temp_vector_check.push_back(temp__CheckDifferent);
			continue;
		}

		for (int i = 0; i < hotSections.size(); i++)
		{
			PIMAGE_SECTION_HEADER temp_header = hotSections.at(i);
			PIMAGE_SECTION_HEADER temp_header2 = coldSections.at(i);
			if (temp_header2->Misc.VirtualSize != temp_header->Misc.VirtualSize)
			{
				_CheckDifferent temp__CheckDifferent;
				RtlZeroMemory(&temp__CheckDifferent, sizeof(_CheckDifferent));
				temp__CheckDifferent.Fail = true;
				RtlCopyMemory(temp__CheckDifferent.Name, x.szModule, 256);
				RtlCopyMemory(temp__CheckDifferent.Path, x.szExePath, MAX_PATH);
				temp_vector_check.push_back(temp__CheckDifferent);
				continue;
			}

			for (int j = 0; j < temp_header2->Misc.VirtualSize; j++)
			{
				if (*(char*)(data + temp_header->VirtualAddress + j) != *(char*)(loaded_pe + temp_header->VirtualAddress + j))
				{
					_CheckDifferent temp__CheckDifferent;
					RtlZeroMemory(&temp__CheckDifferent, sizeof(_CheckDifferent));
					RtlCopyMemory(temp__CheckDifferent.Name, x.szModule, 256);
					RtlCopyMemory(temp__CheckDifferent.Path, x.szExePath, MAX_PATH);
					temp__CheckDifferent.Addr = (ULONG64)x.modBaseAddr + temp_header->VirtualAddress + j;
					RtlCopyMemory(temp__CheckDifferent.FileHex, loaded_pe + temp_header2->VirtualAddress + j - 5, 20);
					RtlCopyMemory(temp__CheckDifferent.FileHex, data + temp_header->VirtualAddress + j - 5, 20);
					temp_vector_check.push_back(temp__CheckDifferent);
					j = j + 19;
					continue;
				}
			}
		}
		if (data)
		{
			delete data;
			data = nullptr;
		}
		if (loaded_pe)
		{
			peconv::free_pe_buffer(loaded_pe);
			loaded_pe = nullptr;
		}
	}
	return temp_vector_check;
}