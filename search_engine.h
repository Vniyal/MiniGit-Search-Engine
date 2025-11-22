#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "schema.h"
#include "trie.h"
#include "inverted_index.h"
#include "ranking.h"
#include "fuzzy.h"

typedef struct {
    Trie *filenameTrie;
    Trie *contentTrie;
    InvertedIndex *invertedIndex;
    Ranking *ranking;
    FuzzyMatcher *fuzzyMatcher;
    File *files;
    int fileCount;
    char **filenameToIdMap;
    int mapCount;
    char **filenameTokens;
    int tokenCount;
} SearchEngine;

SearchEngine* searchengine_create(void);

void searchengine_indexFile(SearchEngine *engine, File *file);

SearchResult* searchengine_search(SearchEngine *engine, SearchRequest *request, int *resultCount);

AutocompleteSuggestion* searchengine_getAutocompleteSuggestions(SearchEngine *engine,
                                                                const char *query, int *count);

void searchengine_removeFile(SearchEngine *engine, const char *fileId);

int searchengine_getIndexSize(SearchEngine *engine);

int searchengine_getTotalWords(SearchEngine *engine);

void searchengine_free(SearchEngine *engine);

#endif