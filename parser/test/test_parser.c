#include <parser.h>

#define TEST(a,b) if(!(a)){printf(b"\n");exit(1);}

int main(int argc, char** argv)
{
    const char* test1 = "This is a test.";
    const char* test2 = "'this is a quoted string'";
    const char* test3 = "\"this is a quoted string\"";
    const char* test4 = "/this/is/a/path/to/source.c";
    const char* test5 = "123456";
    const char* test6 = "123456.654321";
    const char* test7 = "key=value";
    struct parser_token_t* head = NULL;
    int i;

    /** Test 1 */
    tokenize_string(&head,(char*)test1,strlen(test1));
    TEST(head != NULL, "Parsing failed.");
    for(i=0;i<4 && head != NULL;i++)
    {
        TEST(head->type == PARSER_STRING,"Token was of wrong type.");
        head = head->next;
    }
    free_all_tokens(head);

    /** Test 2 */
    tokenize_string(&head,(char*)test2,strlen(test2));
    TEST(head != NULL,"Parsing failed.");
    TEST(head->type == PARSER_QUOTED_STRING,"Single-quoted string was not detected.");
    free_all_tokens(head);

    /** Test 3 */
    tokenize_string(&head,(char*)test3,strlen(test3));
    TEST(head != NULL,"Parsing failed.");
    TEST(head->type == PARSER_QUOTED_STRING,"Double-quoted string was not detected.");
    free_all_tokens(head);

    /** Test 4 */
    tokenize_string(&head,(char*)test4,strlen(test4));
    TEST(head != NULL,"Parsing failed.");
    TEST(head->type == PARSER_ABS_PATH,"Absolute path was not detected.");
    free_all_tokens(head);

    /** Test 5 */
    tokenize_string(&head,(char*)test5,strlen(test5));
    TEST(head != NULL,"Parsing failed.");
    TEST(head->type == PARSER_INT,"Integer value was not detected.");
    free_all_tokens(head);

    /** Test 6 */
    tokenize_string(&head,(char*)test6,strlen(test6));
    TEST(head != NULL,"Parsing failed.");
    TEST(head->type == PARSER_FLOAT,"Floating point value was not detected.");
    free_all_tokens(head);

    /** Test 7 */
    tokenize_string(&head,(char*)test7,strlen(test7));
    TEST(head != NULL,"Parsing failed.");
    TEST(head->type == PARSER_PAIR,"Key-value pair was not detected.");
    free_all_tokens(head);

    return 0;
}
