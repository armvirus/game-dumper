#include "stdafx.hpp"

namespace dumper 
{
	template<typename T>
	T read(HANDLE proc, std::uintptr_t address) 
	{
		T buffer;
		ReadProcessMemory(proc, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(buffer), 0);
		return buffer;
	}

	bool dump_process(HANDLE game_handle, std::uint32_t process_id, std::string process_name)
	{
		std::uintptr_t module_base = memory::get_module_base(process_id);

		if (!module_base) 
		{
			printf("[-] failed to get module base of game. disable anticheat");
			return false;
		}

		printf("[+] module base [%p]\n", module_base);

		IMAGE_DOS_HEADER dos_header = read<IMAGE_DOS_HEADER>(game_handle, module_base);
		IMAGE_NT_HEADERS nt_header = read<IMAGE_NT_HEADERS>(game_handle, module_base + dos_header.e_lfanew);

		printf("[+] dos header [%p]\n", dos_header);
		printf("[+] nt header [%p]\n\n", nt_header);

		BYTE* copied_buffer = reinterpret_cast<BYTE*>(VirtualAlloc(0, nt_header.OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

		printf("[+] allocated memory at [%p] with size [%i]\n", copied_buffer, nt_header.OptionalHeader.SizeOfImage);

		if (ReadProcessMemory(game_handle, reinterpret_cast<LPCVOID>(module_base), reinterpret_cast<LPVOID>(copied_buffer), nt_header.OptionalHeader.SizeOfImage, 0))
		{
			printf("[+] copied process to current process [%p] -> [%p]\n\n", module_base, copied_buffer);
		}
		else
		{
			printf("[-] failed to copy process to current process.\n");
			return false;
		}

		PIMAGE_DOS_HEADER image_dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(copied_buffer);

		if (image_dos_header->e_magic != IMAGE_DOS_SIGNATURE) 
		{
			printf("[-] invalid dos header signature\n");
			return false;
		}

		printf("[+] image dos header [%p]\n", image_dos_header);

		PIMAGE_NT_HEADERS image_nt_header = reinterpret_cast<PIMAGE_NT_HEADERS>((copied_buffer + image_dos_header->e_lfanew));

		printf("[+] image nt header [%p]\n\n", image_nt_header);

		if (image_nt_header->Signature != IMAGE_NT_SIGNATURE) 
		{
			printf("[-] invalid nt header signature\n");
			return false;
		}

		auto current_section = IMAGE_FIRST_SECTION(image_nt_header);
		for (int i = 0; i < image_nt_header->FileHeader.NumberOfSections; ++i, ++current_section)
		{
			std::string section_name = reinterpret_cast<char*>(current_section->Name);

			printf("[+] fixing section [%s] address [%p] size [%i]\n", section_name, current_section->VirtualAddress, current_section->Misc.VirtualSize);

			current_section->PointerToRawData = current_section->VirtualAddress;
			current_section->SizeOfRawData = current_section->Misc.VirtualSize;
		}

		std::string dump_name = "Dumped_" + process_name;

		DeleteFileA(dump_name.c_str());

		HANDLE file = CreateFileA(dump_name.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		if (!file) 
		{
			printf("[-] failed to create file\n");
			return false;
		}

		printf("\n[+] file handle [%p]\n", file);

		printf("[+] size of dump [%i MB]\n", image_nt_header->OptionalHeader.SizeOfImage);

		if (WriteFile(file, copied_buffer, image_nt_header->OptionalHeader.SizeOfImage, 0, 0)) 
		{
			printf("[+] wrote dumped process to file [%s]\n", dump_name.c_str());
			return true;
		}

		return false;
	}
}

