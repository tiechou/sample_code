#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct char_print_parms{
    pthread_t id;
    char character;
    int count;
};

void * char_print(void *arg)
{ 
    struct char_print_parms *tmp = (char_print_parms *)arg; 
    fprintf(stdout, "char:%c, count:%d\n", tmp->character, tmp->count);
    return NULL;
}

void * char_print2(void *arg)
{ 
    struct char_print_parms *tmp = (char_print_parms *)arg; 
    fprintf(stdout, "char:%c, count:%d\n", tmp->character, tmp->count);
    sleep(6);
    return NULL;
}

int main ()
{
    struct char_print_parms* thread_args = (struct char_print_parms*)malloc(sizeof(char_print_parms)*2);

    for (int i=0; i<2; ++i) {
        thread_args[i].character = 'x';
        thread_args[i].count = i*100;
        if (i==1){
            pthread_create (&thread_args[i].id, NULL, &char_print, &thread_args[i]);
        } else {
            pthread_create (&thread_args[i].id, NULL, &char_print2, &thread_args[i]);
        }
    }

    for (int i=0; i<2; ++i) {
        pthread_join (thread_args[i].id, NULL);
        fprintf(stdout, "i=%d\n",i);
    }
    free(thread_args);

    fprintf(stdout, "pia pia pia\n");
    return 0;
}
