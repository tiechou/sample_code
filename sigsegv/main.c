#include "sigsegv.h"
#include <string.h>

int die() {
    char *err = NULL;
    strcpy(err, "gonner");
    return 0;
}

int main() {
    return die();
}
