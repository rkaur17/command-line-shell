#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "str_func.h"

char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    char quotes[] = "\'\"";

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);
    size_t quote_start = strcspn(*str_ptr, quotes);

    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

    if (quote_start < tok_end) {
    quotes[0] = *(*str_ptr + quote_start);
    quotes[1] = '\0';
    int offset = quote_start + 1;
    size_t quote_end = strcspn(*str_ptr + offset, quotes) + offset;
        tok_end = strcspn(*str_ptr + quote_end, delim) + quote_end - tok_start;
    }

    char *current_ptr = *str_ptr + tok_start;

    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        
        *str_ptr = NULL;
    } else {
        **str_ptr = '\0';
        (*str_ptr)++;
    }

    return current_ptr;
}

char *expand_var(char *str)
{
    size_t var_start = 0;
    var_start = strcspn(str, "$");
    if (var_start == strlen(str)) {
        return NULL;
    }

    size_t var_len = strcspn(str + var_start, " \t\r\n\'\"");

    char *var_name = malloc(sizeof(char) * var_len + 1);
    if (var_name == NULL) {
        return NULL;
    }
    strncpy(var_name, str + var_start, var_len);
    var_name[var_len] = '\0';

    if (strlen(var_name) <= 1) {
        free(var_name);
        return NULL;
    }

   
    char *value = getenv(&var_name[1]);
    if (value == NULL) {
        value = "";
    }

    free(var_name);

    size_t remain_sz = strlen(str + var_start + var_len);

    size_t newstr_sz = var_start + strlen(value) + remain_sz + 1;

    char *newstr = malloc(sizeof(char) * newstr_sz);
    if (newstr == NULL) {
        return NULL;
    }

    strncpy(newstr, str, var_start);
    newstr[var_start] = '\0';
    strcat(newstr, value);
    strcat(newstr, str + var_start + var_len);

    return newstr;
}
