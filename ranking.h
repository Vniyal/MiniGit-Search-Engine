#ifndef RANKING_H
#define RANKING_H

#include "schema.h"
#include "inverted_index.h"

typedef struct {
    char algorithm[10]; // "tfidf" or "bm25"
    double filenameBoost;
    double exactMatchBoost;
    double recencyWeight;
    double fileSizeWeight;
} RankingOptions;

typedef struct {
    InvertedIndex *index;
} Ranking;

Ranking* ranking_create(InvertedIndex *index);

SearchResult* ranking_rankResults(Ranking *ranking, File *files, int fileCount,
                                  const char *query, double *tfidfScores,
                                  const char **fuzzyMatchedFiles, int fuzzyCount,
                                  RankingOptions *options, int *resultCount);

SearchResult* ranking_rankWithBM25(Ranking *ranking, File *files, int fileCount,
                                   const char *query, const char **fuzzyMatchedFiles,
                                   int fuzzyCount, RankingOptions *options, int *resultCount);

void ranking_free(Ranking *ranking);

#endif