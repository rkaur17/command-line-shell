/**
 * @file
 *
 * Contains shell history data structures and retrieval functions.
 */

#ifndef _HISTORY_H_
#define _HISTORY_H_
#include <stdbool.h>


struct history_entry {
    char *command;  
    int command_number;
};
typedef struct history_entry history_entry;
typedef history_entry* History;


void hist_init(unsigned int);
void hist_destroy(void);
void hist_add(const char *);
void hist_print(void);
char* retrieve_from_history(char** args);
char* retrieve_bang_bang (char** args);
char *retrieve_numbered_command (char** args);
bool is_number(char* maybe_number);
int last_command_index ();
const char *hist_search_prefix(char *);
const char *hist_search_cnum(int);
unsigned int hist_last_cnum(void);

#endif
