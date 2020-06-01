#include "stdafx.hpp"

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("usage: dumper [process.exe]\n");
		return -1;
	}

	DWORD process_id = memory::find_process_by_id(argv[1]);

	if (!process_id) {
		printf("unable to find process id\n");
		return -1;
	}

	printf("process id [%d]\n", process_id);

	HANDLE game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);

	if (!game_handle) {
		printf("unable to open handle to game\n");
		return -1;
	}

	printf("game handle [%p]\n", game_handle);

	if (dumper::dump_game(game_handle, process_id, argv[1])) {
		printf("dumped game\n");
		return 0;
	}

	printf("failed to dump game\n");
	return -1;
}