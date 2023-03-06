#ifndef _CMD_HANDLING_H_
#define _CMD_HANDLING_H_

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "str_func.h"

#define BUF_SZ 128

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

struct command_line {
	char ** tokens;
	int tokens_size;
	bool stdout_pipe;
	char * stdout_file;
};

void execute_cmd(char ** tokens);

void execute_pipeline(struct command_line * cmds);

void free_cmds(struct command_line * cmds, int comm_sz);

void remove_quotes(char ** tok);

int get_commands(char * line, struct command_line * cmds);

int run(char * line, int add);

void init_built_in(void);

void set_home(void);

void print_prompt(void);

void change_dir(char ** tokens, int tokens_size);

void change_env(char ** tokens, int tokens_size);

int check_built_in(struct command_line * cmds, int comm_sz, char * line, int add);

void free_env(void);

void decrease_comm_num(void);

#endif
