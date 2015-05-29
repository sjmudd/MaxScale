#include <parser.h>

int main(int argc, char** argv)
{
    const char* str = "This is a test. "
        " These are key=value pairs and \"this is a quoted\" string"
        "    This    has    lots   of   space "
        " 'this is a long    single quoted string' "
        "This is an 123 integer and a 122.12512 floating point value"
" 'this is a path:' /usr/bin ";
    struct parser_token_t* head = NULL;
    struct parser_token_t* tmp;

    tokenize_string(&head,(char*)str,strlen(str));

    if(head)
    {
        tmp = head;
        while(tmp)
        {
            switch(tmp->type)
            {
            case PARSER_UNDEFINED:
                printf("PARSER_UNDEFINED: \n");
                break;
            case PARSER_STRING:
                printf("PARSER_STRING: %s\n",tmp->value.stringval);
                break;
            case PARSER_ABS_PATH:
                printf("PARSER_ABS_PATH: %s\n",tmp->value.stringval);
                break;
            case PARSER_QUOTED_STRING:
                printf("PARSER_QUOTED_STRING: %s\n",tmp->value.stringval);
                break;
            case PARSER_INT:
                printf("PARSER_INT: %d\n",tmp->value.intval);
                break;
            case PARSER_FLOAT:
                printf("PARSER_FLOAT: %f\n",tmp->value.floatval);
                break;
            case PARSER_PAIR:
                printf("PARSER_PAIR: %s %s\n",
                       tmp->value.pairval.key,
                       tmp->value.pairval.value);
                break;
            }
            tmp = tmp->next;
        }
        free_all_tokens(head);
    }
    else
    {
        printf("Head was NULL\n");
    }
    return 0;
}
