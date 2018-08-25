#include <stdio.h>
#include <unistd.h>
#include <signal.h>  
#include <string.h>  
#include <sys/types.h>

#define PROMPT "你想终止程序吗?"

char *prompt=PROMPT;

void ctrl_c_op(int signo)
{
    //write(STDERR_FILENO,prompt,strlen(prompt));
        fprintf(stderr,"Instasdfall Signa:\n\a");
}

int  main()
{
    struct sigaction act;

    act.sa_handler=ctrl_c_op;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    if(sigaction(SIGINT,&act,NULL)<0)
    {
        fprintf(stderr,"Install Signal Action Error:\n\a");
        return 0;
    }
    while(1);
} 
