#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TREE_WIDTH 256
#define WORD_LEN_MAX  128

struct trie_node_t {
    int count;
    int pass;
    char arc;
    struct trie_node_t *child;
    struct trie_node_t *sibling;
};

static struct trie_node_t root = {0, 0, 0, NULL, NULL};

int insert(const char *word)
{
    struct trie_node_t *current;
    struct trie_node_t *prev;
    if (word[0] == '\0') {
        return 0;
    }
    current = &root;
    prev = current;
    int i = 0;
    while (1) {
        if (current == NULL) {
            current = new trie_node_t();
            current->pass = 0;
            current->count = 0;
            current->arc = word[i];
            current->child = NULL;
            current->sibling = NULL;
            prev->child = current;
        }
        while (current != NULL) {
            if (current->arc == word[i]) {
                break;
            }
            prev = current;
            current = current->sibling;
        }
        if (current == NULL) {
            current = new trie_node_t();
            current->pass = 0;
            current->count = 0;
            current->arc = word[i];
            current->child = NULL;
            current->sibling = NULL;
            prev->sibling = current;
        }
        i++;
        if (word[i] == '\0') {
            break;
        }
        current->pass++;
        prev = current;
        current = current->child;
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
        word_dump[pos] = pRoot->arc;
        word_dump[pos+1] = '\0';
        fprintf (stdout, "word: %s, pass: %d, count: %d\n", word_dump, pRoot->pass, pRoot->count);
        if (pRoot->child == NULL)
            return 0;
    }
    struct trie_node_t *current;
    for (current = pRoot; current != NULL; current = current->sibling) {
        word_dump[pos++] = current->arc;
        do_travel(current->child);
        pos--;
    }
    return 0;
}

int main(void)
{
    static char *text = "babygo baby bechelor badge jar";
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
    do_travel(root.sibling);
    // free
    return 0;
}
