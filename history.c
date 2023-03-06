#include "cmd_handling.h"
#include "debug.h"
#include "history.h"
#include <stdio.h>

struct history_entry hist_list[HIST_MAX];
int counter = 0;
long int obj_total = 0;

void print_history(void) {
    int i;

    for (i = 0; i < counter; i++) {
    	printf("   %ld %s", hist_list[i].cmd_id, hist_list[i].line);
    }
}

void add_to_history(char * line) {
	if (counter == 100) {
		counter--;
		free(hist_list[0].line);
		memmove(&hist_list[0], &hist_list[1], sizeof(struct history_entry) * counter);
	}

	struct history_entry * item = &hist_list[counter];
	item->cmd_id = obj_total++;
	item->line = strdup(line);
	free(line);

	if (counter < 100) {
		counter++;
	}
}

void free_history(void) {
	int i;

	for (i = 0; i < counter; i++) {
		LOG("Freeing history: %s\n", hist_list[i].line);
		free(hist_list[i].line);
		LOGP("Done freeing history\n");
	}
}

void history_exec(struct command_line * cmds) {
	if (strcmp(cmds[0].tokens[0], "!!") == 0) {
		run_history_command(obj_total - 1);
	} else if (cmds[0].tokens[0][0] == '!') {
		int hist_num = atoi(&cmds[0].tokens[0][1]);
		if (hist_num == 0 && strcmp(&cmds[0].tokens[0][1], "0") == 0) {
			run_history_command(hist_num);
		} else if (hist_num > 0) {
			run_history_command(hist_num);
		} else if (hist_num < 0) {
			printf("%s: invalid history number\n", cmds[0].tokens[0]);
		}
	}
}

void run_history_command(int hist_num) {
	if (hist_num >= obj_total) {
		return;
	}
	if (obj_total > counter) {
		if (hist_num < obj_total - counter) {
			return;
		}
	} else {
		if (hist_num < 0) {
			return;
		}
	}

	int index = counter - (obj_total - hist_num);

	run(hist_list[index].line, 0);
}