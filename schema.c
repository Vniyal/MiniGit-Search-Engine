#include "schema.h"
#include <stdlib.h>
#include <string.h>

void free_file(File *file) {
    if (!file) return;
    free(file->id);
    free(file->filename);
    free(file->content);
    free(file->type);
    free(file);
}

void free_search_result(SearchResult *result) {
    if (!result) return;
    free(result->fileId);
    free(result->filename);
    free(result->type);
    free(result->contentSnippet);
    free(result->highlightedSnippet);
    if (result->rankingBreakdown) {
        free(result->rankingBreakdown);
    }
    free(result);
}

void free_autocomplete_suggestion(AutocompleteSuggestion *suggestion) {
    if (!suggestion) return;
    free(suggestion->text);
    free(suggestion->type);
    free(suggestion->preview);
    free(suggestion);
}

void free_search_request(SearchRequest *request) {
    if (!request) return;
    free(request->query);
    free(request->scope);
    if (request->fileTypes) {
        for (int i = 0; i < request->fileTypesCount; i++) {
            free(request->fileTypes[i]);
        }
        free(request->fileTypes);
    }
    free(request->rankingAlgorithm);
    free(request);
}