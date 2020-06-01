#include "stdafx.hpp"

namespace memory {
    uintptr_t get_module_base(DWORD process_id) {
        MODULEENTRY32 module_entry;
        module_entry.dwSize = sizeof(module_entry);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        Module32First(snapshot, &module_entry);
        return (uintptr_t)module_entry.modBaseAddr;
    }

    DWORD find_process_by_id(const std::string& process_name) {
        PROCESSENTRY32 process_info;
        process_info.dwSize = sizeof(process_info);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return 0;
        }

        Process32First(snapshot, &process_info);
        if (!process_name.compare(process_info.szExeFile))
        {
            CloseHandle(snapshot);
            return process_info.th32ProcessID;
        }

        while (Process32Next(snapshot, &process_info))
        {
            if (!process_name.compare(process_info.szExeFile))
            {
                CloseHandle(snapshot);
                return process_info.th32ProcessID;
            }
        }

        CloseHandle(snapshot);
        return 0;
    }
}
