#include "kpdb.h"

const char kMagic[] = {
	0x4D, 0x69, 0x63, 0x72, 0x6F, 0x73, 0x6F, 0x66, 0x74, 0x20, 0x43, 0x2F,
	0x43, 0x2B, 0x2B, 0x20, 0x4D, 0x53, 0x46, 0x20, 0x37, 0x2E, 0x30, 0x30,
	0x0D, 0x0A, 0x1A, 0x44, 0x53, 0x00, 0x00, 0x00
};

BOOL KpdbIsPDBMagicValid(SuperBlock* super) {
	return 0 == memcmp(super->FileMagic, kMagic, sizeof(kMagic));
}

PVOID KpdbGetPDBStreamDirectory(PVOID base) {
	SuperBlock* super = (SuperBlock*)base;
	DWORD size = super->NumDirectoryBytes;
	DWORD block_size = super->BlockSize;
	DWORD block_count = (size + block_size - 1) / block_size;
	PDWORD block_id_array = (PDWORD)((BYTE*)base + block_size * super->BlockMapAddr);

	PVOID stream_dir = ExAllocatePool(PagedPool, block_count * block_size);

	PCHAR end_of_stream = (PCHAR)stream_dir;
	for (int i = 0; i < block_count; ++i) {
		PCHAR block = (PCHAR)base + block_size * block_id_array[i];
		memcpy(end_of_stream, block, block_size);
		end_of_stream += block_size;
	}

	return stream_dir;
}

StreamData* KpdbGetPDBStreams(PVOID base, PDWORD streams_count) {
	SuperBlock* super = (SuperBlock*)base;
	if (!KpdbIsPDBMagicValid(super)) return NULL;
	DWORD block_size = super->BlockSize;

	PVOID stream_dir = KpdbGetPDBStreamDirectory(base);
	PDWORD ui32_iter = (PDWORD)stream_dir;
	DWORD stream_num = *ui32_iter++;
	PDWORD stream_array = ui32_iter;
	ui32_iter += stream_num;

	//printf("stream_num = %llu\n", stream_num);

	StreamData* streams = (StreamData*)ExAllocatePool(PagedPool, stream_num * sizeof(StreamData));
	*streams_count = 0;

	for (int i = 0; i < stream_num; ++i) {
		PCHAR current_stream = NULL;
		DWORD current_stream_size = stream_array[i];
		DWORD current_stream_block_count = (current_stream_size + block_size - 1) / block_size;

		current_stream = (PCHAR)ExAllocatePool(PagedPool, current_stream_block_count * block_size);

		PCHAR end_of_stream = current_stream;
		for (int j = 0; j < current_stream_block_count; ++j) {
			DWORD block_id = *ui32_iter++;
			PCHAR block = (PCHAR)base + (block_size * block_id);

			memcpy(end_of_stream, block, block_size);
			end_of_stream += block_size;
		}

		StreamData current_stream_data = { 0 };
		current_stream_data.StreamPointer = current_stream;
		current_stream_data.StreamSize = current_stream_size;
		streams[*streams_count] = current_stream_data;
		(*streams_count)++;
	}

	ExFreePool(stream_dir);

	return streams;
}

BOOL KpdbGetPDBSymbolOffset(PVOID pdbfile, PSYMBOL_DATA SymbolDataList) {
	PCHAR symbols = NULL;
	PCHAR types = NULL;
	StreamData* streams = NULL;
	DWORD streams_count = 0;
	SIZE_T symbolsstreamsize = 0;
	DWORD SymbolsCollected = 0;

	streams = KpdbGetPDBStreams(pdbfile, &streams_count);
	if (!streams) return FALSE;

	types = streams[2].StreamPointer;
	symbols = streams[(((DBIHeader*)streams[3].StreamPointer)->SymRecordStream)].StreamPointer;
	symbolsstreamsize = streams[(((DBIHeader*)streams[3].StreamPointer)->SymRecordStream)].StreamSize;

	{
		PCHAR it = symbols;
		const PCHAR end = (PCHAR)((SIZE_T)it + symbolsstreamsize);
		while (it != end)
		{
			const PUBSYM32* curr = (PUBSYM32*)it;
			if (curr->rectyp == S_PUB32)
			{
				DWORD iteration = 0;
				while (SymbolDataList[iteration].SymbolName) {
					if (strcmp(curr->name, SymbolDataList[iteration].SymbolName) == 0) {
						DbgPrintEx(0, 0, "S_PUB32: [%04X:%08X], Flags : %08X, %s\n", curr->seg, curr->off, curr->pubsymflags, curr->name);

						SymbolDataList[iteration].SectionNumber = curr->seg;
						SymbolDataList[iteration].SectionOffset = curr->off;
						//SymbolDataList[iteration].SymbolNameHash = KpdbCalcHashA(curr->name, KpdbStrlen(curr->name));
						SymbolsCollected++;
						break;
					}
					iteration++;
				}
			}
			it += curr->reclen + 2;
		}
	}

	// free all of the streams and free the stream data
	for (int i = 0; i < streams_count; i++) {
		ExFreePool(streams[i].StreamPointer);
	}

	ExFreePool(streams);

	return TRUE;
}

void KpdbConvertSecOffsetToRVA(DWORD64 ModuleBase, PSYMBOL_DATA SymbolDataList) {
	PIMAGE_NT_HEADERS NTHeader = (PIMAGE_NT_HEADERS)(ModuleBase + ((PIMAGE_DOS_HEADER)ModuleBase)->e_lfanew);
	PIMAGE_SECTION_HEADER SectionHeaderBaseAddr = (PIMAGE_SECTION_HEADER)((DWORD64)NTHeader + sizeof(IMAGE_NT_HEADERS));

	DWORD Iterator = 0;
	while (SymbolDataList[Iterator].SectionOffset) {
		if (SymbolDataList[Iterator].SectionNumber) SymbolDataList[Iterator].SymbolRVA = SymbolDataList[Iterator].SectionOffset + SectionHeaderBaseAddr[SymbolDataList[Iterator].SectionNumber - 1].VirtualAddress;
		Iterator++;
	}
}