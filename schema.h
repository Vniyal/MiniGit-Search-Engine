#ifndef SCHEMA_H
#define SCHEMA_H

#include <time.h>

typedef struct {
    char *id;
    char *filename;
    char *content;
    int size;
    char *type;
    long uploadedAt;
} File;

typedef struct {
    double baseScore;
    double recencyBonus;
    double fileSizeBonus;
    double filenameBoost;
    double exactMatchBoost;
    char algorithm[10]; // "tfidf" or "bm25"
} RankingBreakdown;

typedef struct {
    char *fileId;
    char *filename;
    char *type;
    double relevanceScore;
    int matchedInFilename;
    int matchedInContent;
    char *contentSnippet;
    char *highlightedSnippet;
    long uploadedAt;
    char matchType[10]; // "exact", "partial", "fuzzy"
    RankingBreakdown *rankingBreakdown;
} SearchResult;

typedef struct {
    char *text;
    char *type; // "filename", "content", "recent"
    int frequency;
    char *preview;
} AutocompleteSuggestion;

typedef struct {
    char *query;
    int fuzzy;
    char *scope; // "all", "filename", "content"
    char **fileTypes;
    int fileTypesCount;
    long dateFrom;
    long dateTo;
    int limit;
    int offset;
    char *rankingAlgorithm; // "tfidf", "bm25"
} SearchRequest;

typedef struct {
    int totalFiles;
    int totalWords;
    int indexSize;
    long lastIndexed;
} SearchStats;

typedef struct {
    char *query;
    long timestamp;
    int resultsCount;
} SearchHistory;

// Memory cleanup functions
void free_file(File *file);
void free_search_result(SearchResult *result);
void free_autocomplete_suggestion(AutocompleteSuggestion *suggestion);
void free_search_request(SearchRequest *request);

#endif