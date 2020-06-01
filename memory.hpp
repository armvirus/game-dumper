#pragma once

namespace memory {
	DWORD find_process_by_id(const std::string& process_name);
	uintptr_t get_module_base(DWORD process_id);
}