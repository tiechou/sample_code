#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main() {
    int on_stack, *on_heap;

    //局部变量是放在栈上的，所以 on_stack 的地址就是栈的初始地址
    on_stack = 42;
    printf("stack address: %p\n", &on_stack);

    //malloc 的内存是在堆上分配的
    on_heap = (int*)malloc(sizeof(int));
    printf("heap address: %p\n", on_heap);

    printf("run `pmap %d`\n", getpid());
    pause();
}
