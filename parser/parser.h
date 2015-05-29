#ifndef _PARSER_H
#define _PARSER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    PARSER_UNDEFINED,
    PARSER_STRING,
    PARSER_QUOTED_STRING,
    PARSER_INT,
    PARSER_FLOAT,
    PARSER_PAIR,
    PARSER_ABS_PATH
};

struct pair{
    char* key;
    char* value;
};

struct parser_token_t{
    int type;
    union tokval{
        struct pair pairval;
        char* stringval;
        int intval;
        double floatval;
    }value;
    struct parser_token_t* next;
    struct parser_token_t* head;
};


int tokenize_string(struct parser_token_t** tok, char* param, size_t len);
void free_all_tokens(struct parser_token_t* tok);
#endif
