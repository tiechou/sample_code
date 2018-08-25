#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TREE_WIDTH 256
#define WORD_LEN_MAX  128

struct trie_node_t {
    int count;
    int pass;
    struct trie_node_t *next[TREE_WIDTH];
};

static struct trie_node_t root={0,0, {NULL}};

int insert(const char *word)
{
    struct trie_node_t *current;
    struct trie_node_t *new_node;
    if (word[0] == '\0') {
        return 0;
    }
    current = &root;
    for (int i=0; ; ++i) {
        if (word[i] == '\0') {
            break;
        }
        current->pass++;
        if (current->next[word[i]] == NULL) {
            new_node = (struct trie_node_t *)malloc(sizeof(struct trie_node_t));
            memset(new_node, 0, sizeof(struct trie_node_t));
            current->next[word[i]] = new_node;
        }
        current = current->next[word[i]];
    }
    current->count ++;
    return 0;
};

int do_travel(struct trie_node_t *pRoot)
{
    static char word_dump[WORD_LEN_MAX];
    static int pos = 0;
    if (pRoot == NULL) {
        return 0;
    }
    if (pRoot->count > 0) {
        word_dump[pos] = '\0';
        fprintf (stdout, "word: %s, pass: %d, count: %d\n", word_dump, pRoot->pass, pRoot->count);
    }
    for (int i=0; i<TREE_WIDTH; ++i) {
        word_dump[pos++] = i;
        do_travel(pRoot->next[i]);
        pos--;
    }
    return 0;
}
int main(void)
{
    static char *text = "babygo baby bechelor baby badge jar";
    char buffer[128];
    snprintf (buffer, 128, "%s", text);
    char *ptr = buffer;
    while (1) {
        char *word = strsep(&ptr, " ");
        if (word == NULL) {
            break;
        }
        if (word[0] == '\0') {
            continue;
        }
        insert(word);
    }
    do_travel(&root);
    // do free
    return 0;
}
