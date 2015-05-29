#include <parser.h>

struct parser_token_t* alloc_token();
struct parser_token_t* create_token(int type, char* val);
void free_token(struct parser_token_t*);
void free_all_tokens(struct parser_token_t*);
void link_tokens(struct parser_token_t** a, struct parser_token_t* b);

struct parser_token_t* create_token(int type, char* val)
{
    struct parser_token_t* tok = alloc_token();
    char *ptr,*saved;

    if(tok)
    {
        tok->type = type;
        switch(type)
        {
        case PARSER_STRING:
        case PARSER_QUOTED_STRING:
        case PARSER_ABS_PATH:
            tok->value.stringval = strdup(val);
            break;
        case PARSER_INT:
            tok->value.intval = atoi(val);
            break;
        case PARSER_FLOAT:
            tok->value.floatval = atof(val);
            break;
        case PARSER_PAIR:
            ptr = strtok_r(val,"=",&saved);
            tok->value.pairval.key = strdup(ptr);
            ptr = strtok_r(NULL,"=",&saved);
            tok->value.pairval.value = strdup(ptr);
            break;
        default:
            break;
        }
    }
    return tok;   
}

struct parser_token_t* alloc_token()
{
    struct parser_token_t* tok;
    if((tok = malloc(sizeof(struct parser_token_t))) != NULL)
    {
        tok->next = NULL;
        tok->type = PARSER_UNDEFINED;
    }
    return tok;
}

void free_token(struct parser_token_t* tok)
{
    if(tok)
    {
        switch(tok->type)
        {
        case PARSER_STRING:
        case PARSER_QUOTED_STRING:
        case PARSER_ABS_PATH:
            free(tok->value.stringval);
            break;
        case PARSER_PAIR:
            free(tok->value.pairval.key);
            free(tok->value.pairval.value);
            break;
        }
        free(tok);
    }
}

void free_all_tokens(struct parser_token_t* tok)
{
    struct parser_token_t *head = tok;
    struct parser_token_t *node;
    while(head)
    {
        node = head;
        head = head->next;
        free_token(node);
    }
}

void link_tokens(struct parser_token_t** a, struct parser_token_t* b)
{
    if(*a == NULL)
    {
        b->head = b;
        *a = b;
        
    }
    else
    {
        b->head = (*a)->head;
        (*a)->next = b;
        *a = b;
    }
}

