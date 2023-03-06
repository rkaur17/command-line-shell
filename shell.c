#include "cmd_handling.h"
#include "debug.h"
#include "history.h"
#include "str_func.h"
#include "timer.h"

int main(void) {
	int counter = 1;
	int status = 1;

	init_built_in();

	double start = get_time();

	while (status) {
		char * line = NULL;
		size_t line_sz = 0;

		if (isatty(STDIN_FILENO)) {
			print_prompt();  
		}
		
		size_t sz = getline(&line, &line_sz, stdin);

		LOG("Line %d has size %zu\n", counter++, sz);

		if (sz == -1 || sz == EOF) {
			LOGP("EOF\n");
			close(fileno(stdin));
			break;
		} else if (sz - 1 == 0) {
			LOGP("Emptiness\n");
			decrease_comm_num();
			continue;
		} else if (sz > (ARG_MAX * BUF_SZ)) {
			LOGP("Broken stuff! Get out!\n");
			decrease_comm_num();
			continue;
		}

		status = run(line, 1);
	}
	sleep(1);

	LOGP("Freeing history\n");
	free_history();
	LOGP("Done freeing history\n");

	LOGP("Freeing environment\n");
	free_env();
	LOGP("Done free environment\n");

	double end = get_time();

	if (isatty(STDIN_FILENO)) {
		printf("Time elapsed: %fs\n", end - start);
	}
	return 0;
}
