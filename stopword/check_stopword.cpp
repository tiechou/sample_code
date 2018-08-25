
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

#include "ksb_stopword.h"

// extern int ksb_load_stopword(const char *stopword_dict, stopword_map_t* stopword);

inline int ws_check_stopword(const char *word, int len, stopword_map_t* stopword) 
{
    if (len > 3) {
        return 0;
    }
    char tword[4];
    wchar_t termUcs[4];
    int i;
    for (i = 0; i < len; i++) {
        tword[i] = word[i];
    }
    tword[i] = 0;
    swprintf(termUcs, sizeof(termUcs) / sizeof(wchar_t), L"%s", tword);
    if (termUcs[1] == 0 && ksb_is_stopword(termUcs[0], stopword) ) {
        return 1;
    }
    return 0;
}

int print_stopword(const char *stopword_dict) 
{
    setlocale(LC_ALL, "zh_CN.UTF-8");
    FILE *fp = NULL;
    fp = fopen(stopword_dict, "rb");
    if (!fp) {
        fprintf(stderr, "Can not open %s for read.\n", stopword_dict);
        return -1;
    }
    char line[128];
    int word;
    while (fgets(line, sizeof(line), fp) != NULL) {
        word = strtol(line, NULL, 10);
        if (word <= 0 || word >= 65536) {
            fprintf(stderr, "Invalid stopword %d. Skip!\n", word);
            continue;
        }
        printf("stopword[%lc]: ascii[%d]\n", word, word);
    }
    fclose(fp);
    fp = NULL;
    return 0;
}

int main (int argc, char** argv)
{   
    if (argc != 2) {
        return -1;
    }
   
    stopword_map_t *stopword;  
    ksb_load_stopword(argv[1], stopword); 
    const char* cn_stopword[] = { "的", "了", "在", "是", "我", "有", "和", "就",
        "不", "人", "都", "一", "一个", "上", "也", "很", "到", "说", "要", "去", "你",
        "会", "着", "没有", "看", "好", "自己", "这" };

    uint32_t size = sizeof(cn_stopword)/sizeof(const char *);
    for (uint32_t j=0; j<size; ++j) 
    {
        uint32_t len = strlen(cn_stopword[j]);
        if (len > 3) {
            continue;
        }
        char tword[4];
        wchar_t termUcs[4];
        int i;
        for (i = 0; i < len; i++) {
            tword[i] = cn_stopword[j][i];
        }
        tword[i] = 0;
        swprintf(termUcs, sizeof(termUcs) / sizeof(wchar_t), L"%s", tword);
        fprintf(stderr, "%d\n", termUcs[0]);
    }

    print_stopword(argv[1]);
    return 0;
}
