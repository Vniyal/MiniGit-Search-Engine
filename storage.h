#ifndef STORAGE_H
#define STORAGE_H

#include "schema.h"

typedef struct {
    File *files;
    int fileCount;
    SearchHistory *history;
    int historyCount;
    int indexSize;
    int totalWords;
} Storage;

Storage* storage_create(void);

File* storage_addFile(Storage *storage, const char *filename, const char *content,
                      int size, const char *type);

File* storage_getFile(Storage *storage, const char *id);

File* storage_getAllFiles(Storage *storage, int *count);

int storage_deleteFile(Storage *storage, const char *id);

SearchStats* storage_getStats(Storage *storage);

void storage_addSearchHistory(Storage *storage, const char *query, int resultsCount);

SearchHistory* storage_getSearchHistory(Storage *storage, int limit, int *count);

void storage_setIndexSize(Storage *storage, int size);

void storage_setTotalWords(Storage *storage, int words);

void storage_free(Storage *storage);

#endif