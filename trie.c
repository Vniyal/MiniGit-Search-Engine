#include "trie.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ALPHABET_SIZE 26

static TrieNode* createNode(void) {
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    node->children = (TrieNode **)calloc(ALPHABET_SIZE, sizeof(TrieNode *));
    node->isEndOfWord = 0;
    node->fileIds = (char **)malloc(sizeof(char *) * 100);
    node->fileIdCount = 0;
    node->word = NULL;
    return node;
}

Trie* trie_create(void) {
    Trie *trie = (Trie *)malloc(sizeof(Trie));
    trie->root = createNode();
    return trie;
}

static int charToIndex(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return c - 'A';
    return 0;
}

static char* strToLower(const char *str) {
    char *result = (char *)malloc(strlen(str) + 1);
    for (int i = 0; str[i]; i++) {
        result[i] = tolower((unsigned char)str[i]);
    }
    result[strlen(str)] = '\0';
    return result;
}

void trie_insert(Trie *trie, const char *word, const char *fileId) {
    if (!word || strlen(word) == 0) return;

    char *normalized = strToLower(word);
    TrieNode *current = trie->root;

    for (int i = 0; normalized[i]; i++) {
        int idx = charToIndex(normalized[i]);
        if (!current->children[idx]) {
            current->children[idx] = createNode();
        }
        current = current->children[idx];
    }

    current->isEndOfWord = 1;
    current->word = (char *)malloc(strlen(normalized) + 1);
    strcpy(current->word, normalized);

    int found = 0;
    for (int i = 0; i < current->fileIdCount; i++) {
        if (strcmp(current->fileIds[i], fileId) == 0) {
            found = 1;
            break;
        }
    }
    if (!found) {
        current->fileIds[current->fileIdCount] = (char *)malloc(strlen(fileId) + 1);
        strcpy(current->fileIds[current->fileIdCount], fileId);
        current->fileIdCount++;
    }

    free(normalized);
}

char** trie_search(Trie *trie, const char *word, int *count) {
    if (!word) {
        *count = 0;
        return NULL;
    }

    char *normalized = strToLower(word);
    TrieNode *current = trie->root;

    for (int i = 0; normalized[i]; i++) {
        int idx = charToIndex(normalized[i]);
        if (!current->children[idx]) {
            free(normalized);
            *count = 0;
            return NULL;
        }
        current = current->children[idx];
    }

    free(normalized);
    if (!current || current->fileIdCount == 0) {
        *count = 0;
        return NULL;
    }

    char **result = (char **)malloc(sizeof(char *) * current->fileIdCount);
    for (int i = 0; i < current->fileIdCount; i++) {
        result[i] = (char *)malloc(strlen(current->fileIds[i]) + 1);
        strcpy(result[i], current->fileIds[i]);
    }
    *count = current->fileIdCount;
    return result;
}

static void collectWords(TrieNode *node, char **words, int *count, int limit) {
    if (*count >= limit || !node) return;

    if (node->isEndOfWord && node->word) {
        words[*count] = (char *)malloc(strlen(node->word) + 1);
        strcpy(words[*count], node->word);
        (*count)++;
    }

    for (int i = 0; i < ALPHABET_SIZE && *count < limit; i++) {
        if (node->children[i]) {
            collectWords(node->children[i], words, count, limit);
        }
    }
}

char** trie_startsWith(Trie *trie, const char *prefix, int *count) {
    if (!prefix || strlen(prefix) == 0) {
        *count = 0;
        return NULL;
    }

    char *normalized = strToLower(prefix);
    TrieNode *current = trie->root;

    for (int i = 0; normalized[i]; i++) {
        int idx = charToIndex(normalized[i]);
        if (!current->children[idx]) {
            free(normalized);
            *count = 0;
            return NULL;
        }
        current = current->children[idx];
    }

    free(normalized);
    char **words = (char **)malloc(sizeof(char *) * 10);
    *count = 0;
    collectWords(current, words, count, 10);
    return words;
}

char** trie_getAllWords(Trie *trie, int *count) {
    char **words = (char **)malloc(sizeof(char *) * 1000);
    *count = 0;
    collectWords(trie->root, words, count, 1000);
    return words;
}

static void freeNode(TrieNode *node) {
    if (!node) return;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            freeNode(node->children[i]);
        }
    }
    for (int i = 0; i < node->fileIdCount; i++) {
        free(node->fileIds[i]);
    }
    free(node->fileIds);
    free(node->children);
    free(node->word);
    free(node);
}

void trie_free(Trie *trie) {
    if (!trie) return;
    freeNode(trie->root);
    free(trie);
}