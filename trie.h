#ifndef TRIE_H
#define TRIE_H

#include <stdint.h>

typedef struct TrieNode {
    struct TrieNode **children;
    int isEndOfWord;
    char **fileIds;
    int fileIdCount;
    char *word;
} TrieNode;

typedef struct {
    TrieNode *root;
} Trie;

Trie* trie_create(void);
void trie_insert(Trie *trie, const char *word, const char *fileId);
char** trie_search(Trie *trie, const char *word, int *count);
char** trie_startsWith(Trie *trie, const char *prefix, int *count);
char** trie_getAllWords(Trie *trie, int *count);
void trie_free(Trie *trie);

#endif