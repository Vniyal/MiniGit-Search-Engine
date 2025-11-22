#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <stddef.h>
#include <stdbool.h>

/* Max lengths */
#define MAX_TITLE_LENGTH 256
#define MAX_DESCRIPTION_LENGTH 512
#define MAX_URL_LENGTH 256
#define MAX_SUGGESTION_LENGTH 128
#define MAX_QUERY_LENGTH 256

/* Search parameters */
#define MAX_SEARCH_RESULTS 100
#define MAX_AUTOCOMPLETE_SUGGESTIONS 20
#define DEFAULT_RELEVANCE_THRESHOLD 0.1
#define DEFAULT_SUGGESTION_THRESHOLD 0.3

/* Search result struct */
typedef struct {
    char title[MAX_TITLE_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    char url[MAX_URL_LENGTH];
    float relevance_score;
    int document_id;
    long timestamp;
    int click_count;
    float authority_score;
} search_result_t;

/* Autocomplete result struct */
typedef struct {
    char suggestion[MAX_SUGGESTION_LENGTH];
    float score;
    int frequency;
    bool is_trending;
    long last_used;
} autocomplete_result_t;

/* Search config */
typedef struct {
    float relevance_threshold;
    float suggestion_threshold;
    int max_results;
    int max_suggestions;
    bool enable_fuzzy_search;
    bool enable_trending;
} search_config_t;

/* Core functions */
int init_search_engine(void);
void cleanup_search_engine(void);
int build_search_index(const char* base_directory);
int search_and_rank(const char *query, search_result_t *results, int max_results);
search_config_t* get_search_config(void);
int update_search_config(const search_config_t *config);
void get_search_stats(int *total_documents, int *total_queries, double *avg_response_time);

/* Utilities */
int normalize_query(const char *query, char *normalized_query, size_t max_length);
float calculate_similarity(const char *str1, const char *str2);
void log_search_query(const char *query, int results_count, double response_time);

#endif /* SEARCH_ENGINE_H */
