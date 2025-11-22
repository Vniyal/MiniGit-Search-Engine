#include "search_engine.h"
#include "trie_index.h"
#include "ranking.h"
#include "autocomplete.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_FILES 1000
#define MAX_FILEPATH_LENGTH 256
#define MAX_BUFFER_SIZE 10000

static bool engine_initialized = false;
static search_config_t config;
static int total_docs = 0;
static int total_queries = 0;
static double avg_response_ms = 0.0;

static char indexed_files[MAX_FILES][MAX_FILEPATH_LENGTH];

// Utility: Lowercase normalize query
int normalize_query(const char *query, char *norm_query, size_t max_len) {
    if (!query || !norm_query || max_len == 0) return -1;
    size_t qlen = strlen(query);
    if (qlen >= max_len) return -1;

    int pos = 0;
    bool last_space = false;
    for (size_t i=0; i<qlen; i++) {
        char c = tolower(query[i]);
        if (isalnum(c)) {
            norm_query[pos++] = c;
            last_space = false;
        } else if (!last_space && pos>0) {
            norm_query[pos++] = ' ';
            last_space = true;
        }
    }
    if (pos > 0 && norm_query[pos-1] == ' ') pos--;
    norm_query[pos] = '\0';
    return 0;
}

// Read file contents
char* read_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    if (size <= 0 || size > MAX_BUFFER_SIZE) {
        fclose(f);
        return NULL;
    }
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    size_t read_sz = fread(buffer, 1, size, f);
    buffer[read_sz] = '\0';
    fclose(f);
    return buffer;
}

// Index words from file contents
void index_file(const char* filepath) {
    char* content = read_file(filepath);
    if (!content) {
        printf("Failed to read %s\n", filepath);
        return;
    }
    size_t len = strlen(content);

    char word[64];
    int wlen = 0;
    for (size_t i=0; i<=len; i++) {
        char c = content[i];
        if (isalnum(c)) {
            if (wlen < 63) word[wlen++] = tolower(c);
        } else {
            if (wlen > 0) {
                word[wlen] = '\0';
                trie_insert_word(word, filepath);
                wlen = 0;
            }
        }
    }
    free(content);
}

// Recursive directory scan to index all txt files
void scan_and_index(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) {
        printf("Cannot open dir: %s\n", dir_path);
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") ==0 || strcmp(entry->d_name, "..")==0)
            continue;

        char path[MAX_FILEPATH_LENGTH];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat s;
        if (stat(path, &s) == 0) {
            if (S_ISDIR(s.st_mode)) {
                scan_and_index(path);
            } else if (S_ISREG(s.st_mode)) {
                size_t len = strlen(entry->d_name);
                if (len > 4 && strcmp(entry->d_name + len - 4, ".txt") == 0) {
                    if (total_docs < MAX_FILES) {
                        strncpy(indexed_files[total_docs], path, MAX_FILEPATH_LENGTH - 1);
                        indexed_files[total_docs][MAX_FILEPATH_LENGTH -1] = '\0';
                        total_docs++;
                        printf("Indexing: %s\n", path);
                        index_file(path);
                    }
                }
            }
        }
    }
    closedir(dir);
}

// Initialize Search Engine
int init_search_engine(void) {
    if(engine_initialized) return 0;
    config.relevance_threshold = DEFAULT_RELEVANCE_THRESHOLD;
    config.suggestion_threshold = DEFAULT_SUGGESTION_THRESHOLD;
    config.max_results = MAX_SEARCH_RESULTS;
    config.max_suggestions = MAX_AUTOCOMPLETE_SUGGESTIONS;
    config.enable_fuzzy_search = true;
    config.enable_trending = true;

    initialize_trie();
    engine_initialized = true;
    total_docs = 0;
    total_queries = 0;
    avg_response_ms = 0.0;
    return 0;
}

// Cleanup (free resources)
void cleanup_search_engine(void) {
    free_trie(NULL); // Free root and all nodes
    engine_initialized = false;
    total_docs = 0;
    total_queries = 0;
    avg_response_ms = 0.0;
}

// Build Search index by scanning directory
int build_search_index(const char* base_dir) {
    if (!engine_initialized) return -1;
    total_docs = 0;
    scan_and_index(base_dir);
    printf("Indexed %d files.\n", total_docs);
    return 0;
}

// Search and rank results for single-word query only (for simplicity)
int search_and_rank(const char* query, search_result_t* results, int max_results) {
    if (!engine_initialized || !query || !results || max_results <= 0) return 0;
    
    clock_t start = clock();

    char norm_query[MAX_QUERY_LENGTH];
    if (normalize_query(query, norm_query, sizeof(norm_query)) != 0) {
        printf("Invalid query.\n");
        return 0;
    }

    // For demo: just search one keyword word (first token)
    char* space = strchr(norm_query, ' ');
    if (space) *space = '\0';

    // Retrieve files from trie containing this word
    // For simplicity, print matches here (in real case, collect docs for ranking)
    printf("Searching for \"%s\"\n", norm_query);
    search_word_in_trie(norm_query);

    // Ranking and result population can be added here as needed with your ranking module

    clock_t end = clock();
    double time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    total_queries++;
    avg_response_ms = (avg_response_ms * (total_queries - 1) + time_ms) / total_queries;

    log_search_query(query, max_results, time_ms);

    return max_results; // For demo, pretend we return max_results
}

// Other utility functions omitted here for brevity; reuse from your existing code
search_config_t* get_search_config(void) { return &config; }
int update_search_config(const search_config_t *cfg) { if(cfg) {config = *cfg; return 0;} return -1; }
void get_search_stats(int* td, int* tq, double* avg) {
    if(td)*td = total_docs;
    if(tq)*tq = total_queries;
    if(avg)*avg = avg_response_ms;
}
void log_search_query(const char* query, int results_count, double response_time) {
    printf("SEARCH: query='%s', results=%d, time=%.2fms\n", query, results_count, response_time);
}
