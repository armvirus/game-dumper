#pragma once

namespace memory 
{
	std::uint32_t find_process_by_id(const std::string& process_name);
	std::uintptr_t get_module_base(std::uint32_t process_id);
}