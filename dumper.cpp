#include "stdafx.hpp"

namespace dumper {
	template<typename T>
	T read(HANDLE proc, uintptr_t address) {
		T buffer;
		ReadProcessMemory(proc, (LPCVOID)address, &buffer, sizeof(buffer), 0);
		return buffer;
	}

	bool dump_game(HANDLE game_handle, DWORD process_id, std::string process_name) {
		uintptr_t module_base = memory::get_module_base(process_id);

		if (!module_base) {
			printf("failed to get module base of game. disable ac");
			return false;
		}

		printf("module base [%p]\n", module_base);

		IMAGE_DOS_HEADER dos = read<IMAGE_DOS_HEADER>(game_handle, module_base);
		IMAGE_NT_HEADERS nt = read<IMAGE_NT_HEADERS>(game_handle, module_base + dos.e_lfanew);

		printf("dos header [%p]\n", dos);
		printf("nt header [%p]\n", nt);

		BYTE* buffer = (BYTE*)malloc(nt.OptionalHeader.SizeOfImage);

		for (DWORD off = 0x0; off < nt.OptionalHeader.SizeOfImage; off += 0x1000) {
			ReadProcessMemory(game_handle, (LPCVOID)(module_base + off), (LPVOID)(buffer + off), 0x1000, 0);
		}

		PIMAGE_DOS_HEADER pDOS = (PIMAGE_DOS_HEADER)buffer;
		if (pDOS->e_magic != IMAGE_DOS_SIGNATURE) {
			printf("invalid dos header signature\n");
			return false;
		}

		printf("pdos header [%p]\n", pDOS);

		PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(buffer + ((IMAGE_DOS_HEADER*)buffer)->e_lfanew);
		PIMAGE_OPTIONAL_HEADER pOptionalHeader = (PIMAGE_OPTIONAL_HEADER)&pNTHeader->OptionalHeader;

		printf("pnt header [%p]\n", pNTHeader);
		printf("potional header [%p]\n", pOptionalHeader);

		if (pNTHeader->Signature != IMAGE_NT_SIGNATURE) {
			printf("invalid nt header signature\n");
			return false;
		}

		int i = 0;
		unsigned int bufPtr = pOptionalHeader->SizeOfHeaders;

		for (PIMAGE_SECTION_HEADER pSecHeader = IMAGE_FIRST_SECTION(pNTHeader); i < pNTHeader->FileHeader.NumberOfSections; ++i, ++pSecHeader)
		{
			pSecHeader->Misc.VirtualSize = pSecHeader->SizeOfRawData;

			memcpy(buffer + bufPtr, pSecHeader, sizeof(IMAGE_SECTION_HEADER));
			bufPtr += sizeof(IMAGE_SECTION_HEADER);

			ReadProcessMemory(game_handle, (LPCVOID)(pOptionalHeader->ImageBase + pSecHeader->VirtualAddress), (LPVOID)(buffer + pSecHeader->PointerToRawData), pSecHeader->SizeOfRawData, 0);
		}

		std::string dump_name = "Dumped_" + process_name;
		HANDLE file = CreateFileA(dump_name.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		if (!file) {
			printf("failed to create file\n");
			return false;
		}

		printf("file handle [%p]\n", file);

		printf("size of dump [%d]\n", pOptionalHeader->SizeOfImage);
		if (WriteFile(file, buffer, (int)pOptionalHeader->SizeOfImage, 0, 0)) {
			printf("wrote dump to file\n");
			return true;
		}

		return false;
	}
}

