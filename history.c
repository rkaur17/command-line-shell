#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "logger.h"
#include "history.h"

int history_tracker = 0;
unsigned int history_length = 0;
History history = NULL;

void hist_init(unsigned int limit)
{
    // DONE: set up history data structures, with 'limit' being the maximum
    // number of entries maintained.
    // reserve enough memory to hold on to history memeories
    // make sure there is enough
    history = calloc(limit, sizeof(history_entry));
    history_length = limit;    //history_length must remember how many spots of history
}

unsigned int min(unsigned int a, unsigned int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

void hist_destroy(void)
{
    // DONE: tear down any history-related data structures here
    unsigned int upper_bound = min(history_length, history_tracker);
    for (int index = 0; index < upper_bound; index++) {
        history_entry entry = history[index];
        free(entry.command); //each command part for every entry, is pointing at memory that was reserved by strdup
    }
    free(history);
}

void hist_add(const char *cmd)
{
    // DONE: add a history entry
    int index = history_tracker % history_length;
    free(history[index].command);               // frees whatever existing command there is
    history[index].command = strdup(cmd);
    history_tracker++;
    history[index].command_number = history_tracker;
}

void hist_print(void)
{
    // DONE: print the history list in reverse chronological order
    int index = 0;
    if (history_tracker >= history_length) {
        index = (history_tracker % history_length);
    }
    //stop at number of commands enterded, stop at number of history entries there are
    int upper_bound = min(history_tracker, history_length); //stop at whichever comes first
    for (int total_printed = 0; total_printed < upper_bound; total_printed++, index++) {
        index = index % history_length;
        printf("% 5d %s\n",
            history[index].command_number,
            history[index].command);
        fflush(stdout);
        // LOG("index = '%d'\n", index);
        // LOG("history[%d].command = '%s'\n", index, history[index].command);
        // LOG("history[%d].command_number = '%d'\n", index, history[index].command_number);
    }
}

char* retrieve_bang_bang (char** args) {
    return strdup(history[last_command_index()].command);
}

int last_command_index () { //returns index of last command
    return (history_tracker - 1) % history_length;
}

char *retrieve_numbered_command (char** args) {
    int test_number;
    int upper_bound = min(history_tracker, history_length);
    sscanf(args[0], "!%d", &test_number);

    for (int i = 0; i < upper_bound; i++) {
        history_entry he = history[i]; //going though history at index i, this is history entry
        if (he.command_number == test_number) {
            LOG("command '%s'\n", args[0]);
        }
    } //looked through all possible commands
    return NULL;
}

bool is_number(char* maybe_number) {
    for (int i = 0; i < strlen(maybe_number); i++) {
        if (!isdigit(maybe_number[i])) {
            return false;
        }
    }
    return true;
}

char* retrieve_from_history(char** args) {
    if (strncmp(args[0], "!!", strlen("!!")) == 0) {
        return retrieve_bang_bang(args);
    } else if (is_number(&args[0][1])) { //after exlamation points is number
        return retrieve_numbered_command(args);
    }

    LOG("Could not find '%s'\n", args[0]);
    return NULL;            // there was no entry matching
}

const char *hist_search_prefix(char *prefix)
{
    // TODO: Retrieves the most recent command starting with 'prefix', or NULL
    // if no match found.
    return NULL;
}

const char *hist_search_cnum(int command_number)
{
    // TODO: Retrieves a particular command number. Return NULL if no match
    // found.
    return NULL;
}

unsigned int hist_last_cnum(void)
{
    // TODO: Retrieve the most recent command number.
    return 0;
}
