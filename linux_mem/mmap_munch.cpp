#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/mman.h>

int main() {
    char *random_bytes;
    FILE *f;
    int offset = 0;

    // open "random" for reading
    f = fopen("random", "r");
    if (!f) {
        perror("couldn't open file");
        return -1;
    }

    // we want to inspect memory before mapping the file
    printf("run `pmap %d`, then press ", getpid());
    getchar();

    random_bytes = (char *)mmap(0, 1000000000, PROT_READ, MAP_SHARED, fileno(f), 0);

    if (random_bytes == MAP_FAILED) {
        perror("error mapping the file");
        return -1;
    }

    while (1) {
        printf("random number: %d (press  for next number)", *(int*)(random_bytes+offset));
        getchar();

        offset += 4;
    }
}
