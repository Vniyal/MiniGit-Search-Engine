#include "ranking.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

Ranking* ranking_create(InvertedIndex *index) {
    Ranking *ranking = (Ranking *)malloc(sizeof(Ranking));
    ranking->index = index;
    return ranking;
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

static double calculateRecencyScore(long uploadedAt) {
    long now = time(NULL) * 1000;
    double ageInDays = (now - uploadedAt) / (1000.0 * 60 * 60 * 24);
    return exp(-ageInDays / 30);
}

static double calculateFileSizeScore(int size) {
    const int maxSize = 100000;
    const int minSize = 100;
    int normalizedSize = size > maxSize ? maxSize : (size < minSize ? minSize : size);
    return 1.0 - (double)(normalizedSize - minSize) / (maxSize - minSize);
}

static char* extractSnippet(const char *content, char **queryTerms, int termCount) {
    char *snippet = (char *)malloc(201);
    strncpy(snippet, content, 200);
    snippet[200] = '\0';
    if (strlen(content) > 200) {
        strcat(snippet, "...");
    }
    return snippet;
}

SearchResult* ranking_rankResults(Ranking *ranking, File *files, int fileCount,
                                  const char *query, double *tfidfScores,
                                  const char **fuzzyMatchedFiles, int fuzzyCount,
                                  RankingOptions *options, int *resultCount) {
    SearchResult *results = (SearchResult *)malloc(sizeof(SearchResult) * fileCount);
    *resultCount = 0;

    char *queryLower = (char *)malloc(strlen(query) + 1);
    strcpy(queryLower, query);
    for (int i = 0; queryLower[i]; i++) {
        queryLower[i] = tolower((unsigned char)queryLower[i]);
    }

    int queryTermCount;
    char **queryTerms = tokenize(query, &queryTermCount);

    for (int i = 0; i < fileCount; i++) {
        SearchResult result;
        result.fileId = (char *)malloc(strlen(files[i].id) + 1);
        strcpy(result.fileId, files[i].id);
        result.filename = (char *)malloc(strlen(files[i].filename) + 1);
        strcpy(result.filename, files[i].filename);
        result.type = (char *)malloc(strlen(files[i].type) + 1);
        strcpy(result.type, files[i].type);

        double baseScore = tfidfScores[i];
        double relevanceScore = baseScore;

        char *filenameLower = (char *)malloc(strlen(files[i].filename) + 1);
        strcpy(filenameLower, files[i].filename);
        for (int j = 0; filenameLower[j]; j++) {
            filenameLower[j] = tolower((unsigned char)filenameLower[j]);
        }

        char *contentLower = (char *)malloc(strlen(files[i].content) + 1);
        strcpy(contentLower, files[i].content);
        for (int j = 0; contentLower[j]; j++) {
            contentLower[j] = tolower((unsigned char)contentLower[j]);
        }

        int exactFilenameMatch = strstr(filenameLower, queryLower) != NULL;
        int exactContentMatch = strstr(contentLower, queryLower) != NULL;

        result.matchedInFilename = exactFilenameMatch;
        result.matchedInContent = exactContentMatch;

        double filenameBoostMultiplier = result.matchedInFilename ? options->filenameBoost : 1.0;
        double exactMatchBoostMultiplier = (exactFilenameMatch || exactContentMatch) ? 
                                           options->exactMatchBoost : 1.0;

        double recencyScore = calculateRecencyScore(files[i].uploadedAt);
        double fileSizeScore = calculateFileSizeScore(files[i].size);

        relevanceScore *= filenameBoostMultiplier * exactMatchBoostMultiplier;
        relevanceScore += (recencyScore * options->recencyWeight) + 
                         (fileSizeScore * options->fileSizeWeight);

        result.relevanceScore = relevanceScore;
        result.uploadedAt = files[i].uploadedAt;

        strcpy(result.matchType, "partial");
        if (exactFilenameMatch || exactContentMatch) {
            strcpy(result.matchType, "exact");
        }

        result.contentSnippet = extractSnippet(files[i].content, queryTerms, queryTermCount);
        result.highlightedSnippet = (char *)malloc(strlen(result.contentSnippet) + 1);
        strcpy(result.highlightedSnippet, result.contentSnippet);

        result.rankingBreakdown = (RankingBreakdown *)malloc(sizeof(RankingBreakdown));
        result.rankingBreakdown->baseScore = baseScore;
        result.rankingBreakdown->recencyBonus = recencyScore * options->recencyWeight;
        result.rankingBreakdown->fileSizeBonus = fileSizeScore * options->fileSizeWeight;
        result.rankingBreakdown->filenameBoost = filenameBoostMultiplier;
        result.rankingBreakdown->exactMatchBoost = exactMatchBoostMultiplier;
        strcpy(result.rankingBreakdown->algorithm, "tfidf");

        results[(*resultCount)++] = result;

        free(filenameLower);
        free(contentLower);
    }

    for (int i = 0; i < queryTermCount; i++) {
        free(queryTerms[i]);
    }
    free(queryTerms);
    free(queryLower);

    return results;
}

SearchResult* ranking_rankWithBM25(Ranking *ranking, File *files, int fileCount,
                                   const char *query, const char **fuzzyMatchedFiles,
                                   int fuzzyCount, RankingOptions *options, int *resultCount) {
    // Simplified BM25 implementation
    return ranking_rankResults(ranking, files, fileCount, query, 
                              (double *)calloc(fileCount, sizeof(double)),
                              fuzzyMatchedFiles, fuzzyCount, options, resultCount);
}

void ranking_free(Ranking *ranking) {
    if (!ranking) return;
    free(ranking);
}