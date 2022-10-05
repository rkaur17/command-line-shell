#define _GNU_SOURCE

#include <stdio.h>
#include <readline/readline.h>
#include <locale.h>
#include <stdlib.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "history.h"
#include "logger.h"
#include "ui.h"

bool scripting = false;

static const char *good_str = "😌";
static const char *bad_str  = "🤯";

static int readline_init(void);

void init_ui(void)
{
    LOGP("Initializing UI...\n");

    char *locale = setlocale(LC_ALL, "en_US.UTF-8");
    LOG("Setting locale: %s\n",
            (locale != NULL) ? locale : "could not set locale!");

    rl_startup_hook = readline_init;

    if (isatty(STDIN_FILENO)) {
        scripting = false;
        LOG("stdin is a TTY; entering interactive %s\n", "mode");
    } else {
        scripting = true;
        // TODO: grab the command from stdin without using read command
        //using getline
        LOG("data piped in on stdin; entering script %s\n", "mode");
    }

}

void destroy_ui(void)
{
    // TODO cleanup code, if necessary
}

char *prompt_line(void)
{
    const char *status = prompt_status() ? bad_str : good_str;

    char cmd_num[25];
    snprintf(cmd_num, 25, "%u", prompt_cmd_num());

    char *user = prompt_username();
    char *host = prompt_hostname();
    char *cwd = prompt_cwd();

    char *format_str = ">>-[%s]-[%s]-[%s@%s:%s]-> ";

    size_t prompt_sz
        = strlen(format_str)
        + strlen(status)
        + strlen(cmd_num)
        + strlen(user)
        + strlen(host)
        + strlen(cwd)
        + 1;

    char *prompt_str =  malloc(sizeof(char) * prompt_sz);

    snprintf(prompt_str, prompt_sz, format_str,
            status,
            cmd_num,
            user,
            host,
            cwd);

    return prompt_str;
}

char *prompt_username(void)
{
    //int getlogin_r(char *buf, size_t bufsize);
    //33 because that is what man page says for useradd
    char* username = (char*)calloc(33, sizeof(char));

    if (getlogin_r(username, 32)) { //if there was error, that means getlogin failed
        free(username);             //free memeory before returning, there is no username
        return NULL;
    } else {
        return username; //if true then return username
    }
    //return "unknown_user"; //not reachable
}

char *prompt_hostname(void)
{
    //int gethostname(char *name, size_t len);
    char* hostname = (char*)calloc(254, sizeof(char));

    if (gethostname(hostname, 253)) { //if there was error, that means getlogin failed
        free(hostname);             //free memeory before returning, there is no username
        return NULL;
    } else {
        return hostname; //if true then return username
    }
   //return "unknown_host";
}


bool prefix(const char *pre, const char *str) { //check whether full cwd starts with home dir
    return strncmp(pre, str, strlen(pre)) == 0; //if first arguement is prefix of full cwd
}
char *prompt_cwd(void)
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir; //home directory poiting to pointer with home dir
    char* full_cwd = get_current_dir_name();
    LOG("homedir = '%s'\n", homedir);
    if (prefix(homedir, full_cwd)) {
        //+1 for tilde character
        size_t short_len = (strlen(full_cwd) - strlen(homedir) + 1);
         //NULL characteer
        char* short_cwd = (char*)calloc(short_len + 1, sizeof(char));
        short_cwd[0] =  '~';
        strncpy(short_cwd + 1, full_cwd + strlen(homedir), short_len - 1);
        free(full_cwd); //free or else reserved memory will be no reference
        //return ~/deleteme
        return short_cwd;
    } else {
        return full_cwd;
    }
}


int prompt_status(void)
{
    return child_exit_status;
}

unsigned int prompt_cmd_num(void)
{
    return 0;
}

char *read_command(void) {
    if (scripting == true) {
        //if this is script, then dont need to print prompt
        //TODO; need to read the command from the script
        char *line = NULL;
        size_t line_sz = 0;
        ssize_t read_sz = getline(&line, &line_sz, stdin);
        if(read_sz == -1) {
            free(line);
            return NULL;
        }
        for (ssize_t i = 0; i < read_sz; i++) {
            if (line[i] == '#') {
                line[i] = '\0';
                break; //stop processing string once you ind hashtag, or if you go through entire string
            }
        }
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        return line;
    } else {
        char *prompt = prompt_line();
        char *command = readline(prompt);
        free(prompt);
        return command;

    }
}


int readline_init(void)
{
    rl_bind_keyseq("\\e[A", key_up);
    rl_bind_keyseq("\\e[B", key_down);
    rl_variable_bind("show-all-if-ambiguous", "on");
    rl_variable_bind("colored-completion-prefix", "on");
    rl_attempted_completion_function = command_completion;
    return 0;
}

int key_up(int count, int key)
{
    /* Modify the command entry text: */
    rl_replace_line("User pressed 'up' key", 1);

    /* Move the cursor to the end of the line: */
    rl_point = rl_end;

    // TODO: step back through the history until no more history entries are
    // left. Once the end of the history is reached, stop updating the command
    // line.

    // int key_up(int count, int key) { 
//     if (search_str == NULL) {
//         search_str = strdup(r1 line buffer);
//         nav_idx -1;
//     }
//     const char *result = hist_search_prefix_from(search_str, &nav_idx);
//     if (result == NULL) {
//         return 0;
//     }
//     rl_replace_line(result, 1);
//     rl point = rl_end;
//     }
// }

    return 0;
}

int key_down(int count, int key)
{
    /* Modify the command entry text: */
    rl_replace_line("User pressed 'down' key", 1);

    /* Move the cursor to the end of the line: */
    rl_point = rl_end;

    // TODO: step forward through the history (assuming we have stepped back
    // previously). Going past the most recent history command blanks out the
    // command line to allow the user to type a new command.

    return 0;
}

char **command_completion(const char *text, int start, int end)
{
    /* Tell readline that if we don't find a suitable completion, it should fall
     * back on its built-in filename completion. */
    rl_attempted_completion_over = 0;

    return rl_completion_matches(text, command_generator);
}

int initialize_commands(char*** _commands, const char *text) {
    char** commands = NULL;
    char** new_commands = NULL;
    char* paths = getenv("PATH"); //give us strings seperated by colon
    if (paths) {
        paths = strdup(paths);
    }
    char* builtin_commands[] = {"cd", "history", "jobs", "exit"};
    int builtin_commands_length = 4;
    int command_count = 0;
    LOG("paths =\n'%s'\n", paths);
    if (!paths) {
        *_commands = commands;
        return -1;
    }
    char* path = strtok(paths, ":");
    while (path) {
        LOG("path = '%s'\n", path);
        path = strtok(NULL, ":");
    }
    for (int builtin_commands_index = 0; builtin_commands_index < builtin_commands_length; builtin_commands_index++) {
        if (text && text[0] != '\0' && strncmp(builtin_commands[builtin_commands_index], text, strlen(text))) {
            continue;
        }
        command_count++;
        new_commands = (char**) realloc(commands, sizeof(char*) * command_count);
        if (!new_commands) {
            free(commands);
            commands = NULL;
            *_commands = commands;
            return -1;
        }
        commands = new_commands;
        commands[command_count - 1] = strdup(builtin_commands[builtin_commands_index]);
        if (!commands[command_count - 1]) {
            free(commands);
            commands = NULL;
            *_commands = commands;
            return -1;
        }
    }
    command_count++;
    new_commands = (char**) realloc(commands, sizeof(char*) * command_count);
    if (!new_commands) {
        free(commands);
        commands = NULL;
        *_commands = commands;
        return -1;
    }
    commands = new_commands;
    commands[command_count - 1] = NULL;
    *_commands = commands;
    return command_count;
}

/**
 * This function is called repeatedly by the readline library to build a list of
 * possible completions. It returns one match per function call. Once there are
 * no more completions available, it returns NULL.
 */
char *command_generator(const char *text, int state)
{
    // TODO: find potential matching completions for 'text.' If you need to
    // initialize any data structures, state will be set to '0' the first time
    // this function is called. You will likely need to maintain static/global
    // variables to track where you are in the search so that you don't start
    // over from the beginning.
    static char** commands = NULL;
    int command_count = 0;
    LOG("text = '%s', state = '%d'\n", text, state);

    static int command_index = 0;
    if (state == 0) { //generate all possible return values
        command_count = initialize_commands(&commands, text);
        if (command_count < 0) {
            return NULL;
        }
        command_index = 0;
    } else {
        command_index++;
    }
    
    if (command_index < command_count) {
        return commands[command_index];
    }
    return NULL;
}
