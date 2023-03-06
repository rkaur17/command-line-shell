#ifndef _HISTORY_H_
#define _HISTORY_H_

#include "cmd_handling.h"

#define HIST_MAX 100

struct history_entry {
    unsigned long cmd_id;
    double run_time;
    char * line;
};

void print_history(void);

void add_to_history(char * line);

void free_history(void);

void history_exec(struct command_line * cmds);

void run_history_command(int hist_num);

#endif

