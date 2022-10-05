#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

#include "history.h"
#include "logger.h"
#include "ui.h"


char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

static int number_of_children = 0;
extern bool scripting;

void interrupt_Signal_Handler(int dummy) {
    if (number_of_children == 0 && scripting == false) {
        char* prompt = prompt_line();
        printf("\n");
        printf(prompt);
        free(prompt);
        fflush(stdout);
    }
}

void child_Signal_Handler(int dummy) {
    waitpid(-1, &child_exit_status, WNOHANG);
    LOG("dummy: %d\n", dummy);
}

char** break_command_into_args(char* command) {
    char** args = NULL;
    int arg_count = 0;
    char* next_tok = command;
    char* curr_tok;
    char** new_args;

    while ((curr_tok = next_token(&next_tok, " \t\r\n")) != NULL) {
        arg_count++; //each arg is one of the arrays of characters
        new_args = (char**) realloc(args, sizeof(char*) * arg_count);
        if (new_args) {
            args = new_args;
            args[arg_count - 1] = strdup(curr_tok);
        } else {
            free(args);
            args = NULL;
            return args;
        }
    }

    new_args = (char**) realloc(args, sizeof(char*) * (arg_count + 1));
    
    if (new_args) {
        args = new_args;
        args[arg_count] = NULL;
    } else {
        free(args);
        args = NULL;
    }
    return args;
}

void args_destroy(char** args) {
    int index = 0;
    for (char* arg = args[0]; arg != NULL; arg = args[++index]) {
        free(arg);
    }
    free(args);
}

bool is_background(const char* command) {
    int last_char_index = strlen(command) - 1;
    return command[last_char_index] == '&';
}

int child_exit_status;

int main(void) {
    hist_init(100);
    signal(SIGINT, interrupt_Signal_Handler);
    signal(SIGCHLD, child_Signal_Handler);

    init_ui();

    char *command;
    while(true) {
        command = read_command();
        if (command == NULL) {
            break;
        }
        char* original_command = strdup(command);
        char** args = break_command_into_args(command);

        if (args[0] == (char *) 0) {
            goto cleanup;
        }

        if (args[0][0] == '!') {
            char* command_from_history = retrieve_from_history(args);
            if (command_from_history == NULL) {
                goto cleanup;
            } else {
                free(original_command);
                args_destroy(args);
                free(command);
                command = command_from_history;
                original_command = strdup(command);
                args = break_command_into_args(command);
                if (!scripting) {
                    printf("%s\n", original_command);
                }
            }
        }

        hist_add(original_command);
        if (strcmp(args[0], "cd") == 0) { //args[0] is cd, args[1] is what is after cd
            if (args[1]) {
                chdir(args[1]);
            } else {
                struct passwd *pw = getpwuid(getuid());
                const char *homedir = pw->pw_dir;
                chdir(homedir);
            }
            goto cleanup;
        }

        if (strcmp(args[0], "history") == 0) { //if = to 0 then we know its history
            hist_print();
            goto cleanup;
        }

        if (strcmp(args[0], "exit") == 0) { 
            //TODO; figure out what to do here
            free(command);
            hist_destroy();
            exit(0);
        }

        pid_t child = fork();
        if (child == -1) {
            perror("fork");
            goto cleanup;
        } else if (child == 0) { // parent needs to know child process, but not child so it is ==0 
            if (execvp(args[0], args) != 0) {
                close(fileno(stdin));
                perror("execvp");
                exit(1);
            }
        } else {
            number_of_children++;
            if (is_background(original_command)) {
                waitpid(-1, &child_exit_status, WNOHANG); //allows child process to run in background and main shell to continue processing
            } else {
                waitpid(child, &child_exit_status, 0); //wait until child exits
            }
            number_of_children--;
        }
cleanup:
        free(original_command);
        free(command);
        args_destroy(args);
    }
    hist_destroy();
    return 0;
}

