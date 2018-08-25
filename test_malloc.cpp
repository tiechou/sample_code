#include <stdio.h>
#include <iostream>

typedef struct string_s {
    std::string str;
    int len;
} string_t;

typedef struct char_s {
    char str[16];
    int len;
} char_t;

int main()
{
    string_t* p_str = new string_t();
    p_str->str = "aiai,xiaodesi";
    p_str->len = 888;
    printf("t1:%s\n", p_str->str.c_str());
    delete p_str;

    string_t* p_str2 = (string_t*)malloc(sizeof(string_t) );
    p_str2->str = "aiai,xiaodesi";
    p_str2->len = 888;
    printf("p2:%s\n", p_str2->str.c_str() );
    delete p_str2;

    char_t* p_char = (char_t* )malloc(sizeof(char_t));
    snprintf(p_char->str, 16, "%s", "aiai,tiechou");
    p_char->len = 16;
    printf("t2:%s\n", p_char->str);
    free(p_char);

    return 0;
}
