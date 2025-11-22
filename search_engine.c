#include "search_engine.h"
#include <stdlib.h>
#include <string.h>

SearchEngine* searchengine_create(void) {
    SearchEngine *engine = (SearchEngine *)malloc(sizeof(SearchEngine));
    engine->filenameTrie = trie_create();
    engine->contentTrie = trie_create();
    engine->invertedIndex = invertedindex_create();
    engine->ranking = ranking_create(engine->invertedIndex);
    engine->fuzzyMatcher = fuzzy_create();
    engine->files = (File *)malloc(sizeof(File) * 10000);
    engine->fileCount = 0;
    engine->filenameToIdMap = (char **)malloc(sizeof(char *) * 10000);
    engine->mapCount = 0;
    engine->filenameTokens = (char **)malloc(sizeof(char *) * 10000);
    engine->tokenCount = 0;
    return engine;
}

static char** tokenize(const char *text, int *count) {
    char **tokens = (char **)malloc(sizeof(char *) * 1000);
    *count = 0;

    char *word = (char *)malloc(strlen(text) + 1);
    int wordLen = 0;

    for (int i = 0; text[i]; i++) {
        if (isalnum((unsigned char)text[i]) || text[i] == '_') {
            word[wordLen++] = tolower((unsigned char)text[i]);
        } else {
            if (wordLen > 1) {
                word[wordLen] = '\0';
                tokens[*count] = (char *)malloc(wordLen + 1);
                strcpy(tokens[*count], word);
                (*count)++;
                wordLen = 0;
            }
        }
    }
    if (wordLen > 1) {
        word[wordLen] = '\0';
        tokens[*count] = (char *)malloc(wordLen + 1);
        strcpy(tokens[*count], word);
        (*count)++;
    }

    free(word);
    return tokens;
}

void searchengine_indexFile(SearchEngine *engine, File *file) {
    engine->files[engine->fileCount] = *file;
    engine->fileCount++;

    char *filenameLower = (char *)malloc(strlen(file->filename) + 1);
    strcpy(filenameLower, file->filename);
    for (int i = 0; filenameLower[i]; i++) {
        filenameLower[i] = tolower((unsigned char)filenameLower[i]);
    }

    engine->filenameToIdMap[engine->mapCount] = filenameLower;
    engine->mapCount++;

    int filenameWordCount;
    char **filenameWords = tokenize(file->filename, &filenameWordCount);
    for (int i = 0; i < filenameWordCount; i++) {
        trie_insert(engine->filenameTrie, filenameWords[i], file->id);
        free(filenameWords[i]);
    }
    free(filenameWords);

    int contentWordCount;
    char **contentWords = tokenize(file->content, &contentWordCount);
    for (int i = 0; i < contentWordCount; i++) {
        trie_insert(engine->contentTrie, contentWords[i], file->id);
        free(contentWords[i]);
    }
    free(contentWords);

    invertedindex_addDocument(engine->invertedIndex, file);
}

SearchResult* searchengine_search(SearchEngine *engine, SearchRequest *request, int *resultCount) {
    // Simplified implementation
    return NULL;
}

AutocompleteSuggestion* searchengine_getAutocompleteSuggestions(SearchEngine *engine,
                                                                const char *query, int *count) {
    AutocompleteSuggestion *suggestions = 
        (AutocompleteSuggestion *)malloc(sizeof(AutocompleteSuggestion) * 10);
    *count = 0;

    int filenameWordCount;
    char **filenameWords = trie_startsWith(engine->filenameTrie, query, &filenameWordCount);
    
    for (int i = 0; i < filenameWordCount && *count < 10; i++) {
        suggestions[*count].text = (char *)malloc(strlen(filenameWords[i]) + 1);
        strcpy(suggestions[*count].text, filenameWords[i]);
        suggestions[*count].type = (char *)malloc(9);
        strcpy(suggestions[*count].type, "filename");
        suggestions[*count].frequency = 1;
        (*count)++;
        free(filenameWords[i]);
    }
    free(filenameWords);

    int contentWordCount;
    char **contentWords = trie_startsWith(engine->contentTrie, query, &contentWordCount);
    
    for (int i = 0; i < contentWordCount && *count < 10; i++) {
        suggestions[*count].text = (char *)malloc(strlen(contentWords[i]) + 1);
        strcpy(suggestions[*count].text, contentWords[i]);
        suggestions[*count].type = (char *)malloc(8);
        strcpy(suggestions[*count].type, "content");
        (*count)++;
        free(contentWords[i]);
    }
    free(contentWords);

    return suggestions;
}

void searchengine_removeFile(SearchEngine *engine, const char *fileId) {
    for (int i = 0; i < engine->fileCount; i++) {
        if (strcmp(engine->files[i].id, fileId) == 0) {
            for (int j = i; j < engine->fileCount - 1; j++) {
                engine->files[j] = engine->files[j + 1];
            }
            engine->fileCount--;
            break;
        }
    }
    invertedindex_removeDocument(engine->invertedIndex, fileId);
}

int searchengine_getIndexSize(SearchEngine *engine) {
    int size = 0;
    for (int i = 0; i < engine->fileCount; i++) {
        size += engine->files[i].size;
    }
    return size;
}

int searchengine_getTotalWords(SearchEngine *engine) {
    int total = 0;
    for (int i = 0; i < engine->fileCount; i++) {
        int wordCount;
        char **words = tokenize(engine->files[i].content, &wordCount);
        total += wordCount;
        for (int j = 0; j < wordCount; j++) {
            free(words[j]);
        }
        free(words);
    }
    return total;
}

void searchengine_free(SearchEngine *engine) {
    if (!engine) return;
    trie_free(engine->filenameTrie);
    trie_free(engine->contentTrie);
    invertedindex_free(engine->invertedIndex);
    ranking_free(engine->ranking);
    fuzzy_free(engine->fuzzyMatcher);
    for (int i = 0; i < engine->mapCount; i++) {
        free(engine->filenameToIdMap[i]);
    }
    free(engine->filenameToIdMap);
    free(engine->filenameTokens);
    free(engine->files);
    free(engine);
}