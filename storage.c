#include "storage.h"
#include <stdlib.h>
#include <string.h>
#include <uuid/uuid.h>

Storage* storage_create(void) {
    Storage *storage = (Storage *)malloc(sizeof(Storage));
    storage->files = (File *)malloc(sizeof(File) * 10000);
    storage->fileCount = 0;
    storage->history = (SearchHistory *)malloc(sizeof(SearchHistory) * 1000);
    storage->historyCount = 0;
    storage->indexSize = 0;
    storage->totalWords = 0;
    return storage;
}

File* storage_addFile(Storage *storage, const char *filename, const char *content,
                      int size, const char *type) {
    uuid_t uuid;
    uuid_generate(uuid);
    char uuidStr[37];
    uuid_unparse(uuid, uuidStr);

    File *file = &storage->files[storage->fileCount];
    file->id = (char *)malloc(37);
    strcpy(file->id, uuidStr);
    file->filename = (char *)malloc(strlen(filename) + 1);
    strcpy(file->filename, filename);
    file->content = (char *)malloc(strlen(content) + 1);
    strcpy(file->content, content);
    file->size = size;
    file->type = (char *)malloc(strlen(type) + 1);
    strcpy(file->type, type);
    file->uploadedAt = time(NULL) * 1000;

    storage->fileCount++;
    storage->indexSize += size;

    int wordCount = 0;
    for (int i = 0; content[i]; i++) {
        if (content[i] == ' ' || content[i] == '\n') wordCount++;
    }
    storage->totalWords += wordCount;

    return file;
}

File* storage_getFile(Storage *storage, const char *id) {
    for (int i = 0; i < storage->fileCount; i++) {
        if (strcmp(storage->files[i].id, id) == 0) {
            return &storage->files[i];
        }
    }
    return NULL;
}

File* storage_getAllFiles(Storage *storage, int *count) {
    *count = storage->fileCount;
    return storage->files;
}

int storage_deleteFile(Storage *storage, const char *id) {
    for (int i = 0; i < storage->fileCount; i++) {
        if (strcmp(storage->files[i].id, id) == 0) {
            storage->indexSize -= storage->files[i].size;
            
            int wordCount = 0;
            for (int j = 0; storage->files[i].content[j]; j++) {
                if (storage->files[i].content[j] == ' ') wordCount++;
            }
            storage->totalWords -= wordCount;

            free(storage->files[i].id);
            free(storage->files[i].filename);
            free(storage->files[i].content);
            free(storage->files[i].type);

            for (int j = i; j < storage->fileCount - 1; j++) {
                storage->files[j] = storage->files[j + 1];
            }
            storage->fileCount--;
            return 1;
        }
    }
    return 0;
}

SearchStats* storage_getStats(Storage *storage) {
    SearchStats *stats = (SearchStats *)malloc(sizeof(SearchStats));
    stats->totalFiles = storage->fileCount;
    stats->totalWords = storage->totalWords;
    stats->indexSize = storage->indexSize;
    stats->lastIndexed = storage->fileCount > 0 ? storage->files[storage->fileCount - 1].uploadedAt : 0;
    return stats;
}

void storage_addSearchHistory(Storage *storage, const char *query, int resultsCount) {
    if (storage->historyCount >= 1000) {
        for (int i = 0; i < 999; i++) {
            storage->history[i] = storage->history[i + 1];
        }
        storage->historyCount = 999;
    }

    SearchHistory *hist = &storage->history[storage->historyCount];
    hist->query = (char *)malloc(strlen(query) + 1);
    strcpy(hist->query, query);
    hist->timestamp = time(NULL) * 1000;
    hist->resultsCount = resultsCount;

    storage->historyCount++;
}

SearchHistory* storage_getSearchHistory(Storage *storage, int limit, int *count) {
    *count = storage->historyCount < limit ? storage->historyCount : limit;
    SearchHistory *result = (SearchHistory *)malloc(sizeof(SearchHistory) * (*count));
    for (int i = 0; i < *count; i++) {
        result[i] = storage->history[i];
    }
    return result;
}

void storage_setIndexSize(Storage *storage, int size) {
    storage->indexSize = size;
}

void storage_setTotalWords(Storage *storage, int words) {
    storage->totalWords = words;
}

void storage_free(Storage *storage) {
    if (!storage) return;
    for (int i = 0; i < storage->fileCount; i++) {
        free(storage->files[i].id);
        free(storage->files[i].filename);
        free(storage->files[i].content);
        free(storage->files[i].type);
    }
    free(storage->files);
    for (int i = 0; i < storage->historyCount; i++) {
        free(storage->history[i].query);
    }
    free(storage->history);
    free(storage);
}