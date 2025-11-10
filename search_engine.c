/**
 * @file search_engine.c
 * @brief Core search engine implementation
 * @author Autocomplete and Ranking Specialist
 * @version 1.0
 * 
 * This module provides the core search engine functionality, integrating
 * autocomplete and ranking systems for comprehensive search capabilities.
 */

#include "search_engine.h"
#include "autocomplete.h"
#include "ranking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* Global search engine state */
static search_config_t g_search_config = {0};
static bool g_search_engine_initialized = false;
static int g_total_documents = 0;
static int g_total_queries = 0;
static double g_avg_response_time = 0.0;

/* Sample documents for demonstration */
static search_result_t sample_documents[10];
static bool sample_documents_initialized = false;

/**
 * @brief Initialize sample documents with dynamic timestamps
 */
static void init_sample_documents(void) {
    if (sample_documents_initialized) return;
    
    time_t current_time = time(NULL);
    
    strcpy(sample_documents[0].title, "Introduction to Search Engines");
    strcpy(sample_documents[0].description, "A comprehensive guide to search engine algorithms and ranking systems");
    strcpy(sample_documents[0].url, "https://example.com/search-engines-intro");
    sample_documents[0].relevance_score = 0.0;
    sample_documents[0].document_id = 1;
    sample_documents[0].timestamp = current_time - 86400;
    sample_documents[0].click_count = 150;
    sample_documents[0].authority_score = 0.8;
    
    strcpy(sample_documents[1].title, "Advanced Ranking Algorithms");
    strcpy(sample_documents[1].description, "Deep dive into TF-IDF, BM25, and machine learning ranking methods");
    strcpy(sample_documents[1].url, "https://example.com/ranking-algorithms");
    sample_documents[1].relevance_score = 0.0;
    sample_documents[1].document_id = 2;
    sample_documents[1].timestamp = current_time - 172800;
    sample_documents[1].click_count = 230;
    sample_documents[1].authority_score = 0.9;
    
    strcpy(sample_documents[2].title, "Autocomplete Systems Design");
    strcpy(sample_documents[2].description, "How to build intelligent autocomplete systems with trie data structures");
    strcpy(sample_documents[2].url, "https://example.com/autocomplete-design");
    sample_documents[2].relevance_score = 0.0;
    sample_documents[2].document_id = 3;
    sample_documents[2].timestamp = current_time - 259200;
    sample_documents[2].click_count = 180;
    sample_documents[2].authority_score = 0.7;
    
    strcpy(sample_documents[3].title, "Information Retrieval Fundamentals");
    strcpy(sample_documents[3].description, "Core concepts in information retrieval and search technology");
    strcpy(sample_documents[3].url, "https://example.com/ir-fundamentals");
    sample_documents[3].relevance_score = 0.0;
    sample_documents[3].document_id = 4;
    sample_documents[3].timestamp = current_time - 345600;
    sample_documents[3].click_count = 320;
    sample_documents[3].authority_score = 0.85;
    
    strcpy(sample_documents[4].title, "Machine Learning for Search");
    strcpy(sample_documents[4].description, "Applying ML techniques to improve search relevance and ranking");
    strcpy(sample_documents[4].url, "https://example.com/ml-search");
    sample_documents[4].relevance_score = 0.0;
    sample_documents[4].document_id = 5;
    sample_documents[4].timestamp = current_time - 432000;
    sample_documents[4].click_count = 290;
    sample_documents[4].authority_score = 0.75;
    
    strcpy(sample_documents[5].title, "Search Engine Optimization");
    strcpy(sample_documents[5].description, "Best practices for optimizing content for search engines");
    strcpy(sample_documents[5].url, "https://example.com/seo-guide");
    sample_documents[5].relevance_score = 0.0;
    sample_documents[5].document_id = 6;
    sample_documents[5].timestamp = current_time - 518400;
    sample_documents[5].click_count = 420;
    sample_documents[5].authority_score = 0.6;
    
    strcpy(sample_documents[6].title, "Data Structures for Search");
    strcpy(sample_documents[6].description, "Efficient data structures used in search engine implementation");
    strcpy(sample_documents[6].url, "https://example.com/search-data-structures");
    sample_documents[6].relevance_score = 0.0;
    sample_documents[6].document_id = 7;
    sample_documents[6].timestamp = current_time - 604800;
    sample_documents[6].click_count = 190;
    sample_documents[6].authority_score = 0.8;
    
    strcpy(sample_documents[7].title, "Query Processing Techniques");
    strcpy(sample_documents[7].description, "How search engines parse and understand user queries");
    strcpy(sample_documents[7].url, "https://example.com/query-processing");
    sample_documents[7].relevance_score = 0.0;
    sample_documents[7].document_id = 8;
    sample_documents[7].timestamp = current_time - 691200;
    sample_documents[7].click_count = 160;
    sample_documents[7].authority_score = 0.7;
    
    strcpy(sample_documents[8].title, "Distributed Search Systems");
    strcpy(sample_documents[8].description, "Building scalable search systems across multiple servers");
    strcpy(sample_documents[8].url, "https://example.com/distributed-search");
    sample_documents[8].relevance_score = 0.0;
    sample_documents[8].document_id = 9;
    sample_documents[8].timestamp = current_time - 777600;
    sample_documents[8].click_count = 140;
    sample_documents[8].authority_score = 0.9;
    
    strcpy(sample_documents[9].title, "Search Analytics and Metrics");
    strcpy(sample_documents[9].description, "Measuring and improving search engine performance");
    strcpy(sample_documents[9].url, "https://example.com/search-analytics");
    sample_documents[9].relevance_score = 0.0;
    sample_documents[9].document_id = 10;
    sample_documents[9].timestamp = current_time - 864000;
    sample_documents[9].click_count = 200;
    sample_documents[9].authority_score = 0.8;
    
    sample_documents_initialized = true;
}

/**
 * @brief Initialize the search engine
 */
int init_search_engine(void) {
    printf("Initializing core search engine...\n");
    
    if (g_search_engine_initialized) {
        printf("Search engine already initialized\n");
        return 0;
    }
    
    // Set default configuration
    g_search_config.relevance_threshold = DEFAULT_RELEVANCE_THRESHOLD;
    g_search_config.suggestion_threshold = DEFAULT_SUGGESTION_THRESHOLD;
    g_search_config.max_results = MAX_SEARCH_RESULTS;
    g_search_config.max_suggestions = MAX_AUTOCOMPLETE_SUGGESTIONS;
    g_search_config.enable_fuzzy_search = true;
    g_search_config.enable_trending = true;
    
    // Initialize sample documents
    init_sample_documents();
    
    // Initialize statistics
    g_total_documents = 10; // We have 10 sample documents
    g_total_queries = 0;
    g_avg_response_time = 0.0;
    
    g_search_engine_initialized = true;
    printf("Search engine initialized with %d sample documents\n", g_total_documents);
    return 0;
}

/**
 * @brief Cleanup search engine resources
 */
void cleanup_search_engine(void) {
    if (!g_search_engine_initialized) {
        return;
    }
    
    // Reset state
    memset(&g_search_config, 0, sizeof(search_config_t));
    g_total_documents = 0;
    g_total_queries = 0;
    g_avg_response_time = 0.0;
    g_search_engine_initialized = false;
    
    printf("Search engine cleanup completed\n");
}

/**
 * @brief Build search index from data sources
 */
int build_search_index(void) {
    printf("Building search index...\n");
    
    if (!g_search_engine_initialized) {
        fprintf(stderr, "Error: Search engine not initialized\n");
        return -1;
    }
    
    // In a real implementation, this would:
    // 1. Crawl or load documents from various sources
    // 2. Parse and extract content
    // 3. Build inverted index for efficient searching
    // 4. Calculate document features (PageRank, etc.)
    // 5. Prepare data structures for ranking
    
    printf("Search index built successfully (%d documents indexed)\n", g_total_documents);
    return 0;
}

/**
 * @brief Perform search and return ranked results
 */
int search_and_rank(const char *query, search_result_t *results, int max_results) {
    if (!query || !results || max_results <= 0) {
        return 0;
    }
    
    if (!g_search_engine_initialized) {
        fprintf(stderr, "Error: Search engine not initialized\n");
        return 0;
    }
    
    clock_t start_time = clock();
    
    // Copy sample documents to results (in practice, this would be a real search)
    int num_results = g_total_documents < max_results ? g_total_documents : max_results;
    
    // Simple keyword matching to filter relevant documents
    int actual_results = 0;
    char query_lower[MAX_QUERY_LENGTH];
    strncpy(query_lower, query, sizeof(query_lower) - 1);
    query_lower[sizeof(query_lower) - 1] = '\0';
    
    // Convert query to lowercase for case-insensitive matching
    for (int i = 0; query_lower[i]; i++) {
        query_lower[i] = tolower(query_lower[i]);
    }
    
    // Find matching documents
    for (int i = 0; i < g_total_documents && actual_results < max_results; i++) {
        char title_lower[MAX_TITLE_LENGTH];
        char desc_lower[MAX_DESCRIPTION_LENGTH];
        
        strncpy(title_lower, sample_documents[i].title, sizeof(title_lower) - 1);
        strncpy(desc_lower, sample_documents[i].description, sizeof(desc_lower) - 1);
        title_lower[sizeof(title_lower) - 1] = '\0';
        desc_lower[sizeof(desc_lower) - 1] = '\0';
        
        // Convert to lowercase
        for (int j = 0; title_lower[j]; j++) title_lower[j] = tolower(title_lower[j]);
        for (int j = 0; desc_lower[j]; j++) desc_lower[j] = tolower(desc_lower[j]);
        
        // Check if query terms appear in title or description
        if (strstr(title_lower, query_lower) || strstr(desc_lower, query_lower)) {
            results[actual_results] = sample_documents[i];
            actual_results++;
        }
    }
    
    // If no exact matches found, include all documents for ranking
    if (actual_results == 0) {
        for (int i = 0; i < num_results; i++) {
            results[i] = sample_documents[i];
        }
        actual_results = num_results;
    }
    
    // Rank the results using the ranking system
    if (rank_search_results(query, results, actual_results, NULL) != 0) {
        fprintf(stderr, "Warning: Ranking failed, returning unranked results\n");
    }
    
    // Calculate response time
    clock_t end_time = clock();
    double response_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    
    // Update statistics
    g_total_queries++;
    g_avg_response_time = (g_avg_response_time * (g_total_queries - 1) + response_time) / g_total_queries;
    
    // Log the search
    log_search_query(query, actual_results, response_time);
    
    return actual_results;
}

/**
 * @brief Get search engine configuration
 */
search_config_t* get_search_config(void) {
    return &g_search_config;
}

/**
 * @brief Update search engine configuration
 */
int update_search_config(const search_config_t *config) {
    if (!config) {
        return -1;
    }
    
    g_search_config = *config;
    return 0;
}

/**
 * @brief Get search statistics
 */
void get_search_stats(int *total_documents, int *total_queries, double *avg_response_time) {
    if (total_documents) *total_documents = g_total_documents;
    if (total_queries) *total_queries = g_total_queries;
    if (avg_response_time) *avg_response_time = g_avg_response_time;
}

/**
 * @brief Normalize and clean a query string
 */
int normalize_query(const char *query, char *normalized_query, size_t max_length) {
    if (!query || !normalized_query || max_length == 0) {
        return -1;
    }
    
    size_t query_len = strlen(query);
    if (query_len >= max_length) {
        return -1;
    }
    
    // Copy and normalize
    strcpy(normalized_query, query);
    
    // Convert to lowercase and remove extra spaces
    int write_pos = 0;
    bool prev_was_space = false;
    
    for (int i = 0; normalized_query[i]; i++) {
        char c = tolower(normalized_query[i]);
        
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!prev_was_space && write_pos > 0) {
                normalized_query[write_pos++] = ' ';
                prev_was_space = true;
            }
        } else if (isalnum(c)) {
            normalized_query[write_pos++] = c;
            prev_was_space = false;
        }
    }
    
    // Remove trailing space
    if (write_pos > 0 && normalized_query[write_pos - 1] == ' ') {
        write_pos--;
    }
    
    normalized_query[write_pos] = '\0';
    return 0;
}

/**
 * @brief Calculate similarity between two strings
 */
float calculate_similarity(const char *str1, const char *str2) {
    if (!str1 || !str2) {
        return 0.0;
    }
    
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    
    if (len1 == 0 && len2 == 0) {
        return 1.0;
    }
    
    if (len1 == 0 || len2 == 0) {
        return 0.0;
    }
    
    // Simple similarity based on common characters
    int common_chars = 0;
    int total_chars = len1 + len2;
    
    for (int i = 0; i < len1; i++) {
        for (int j = 0; j < len2; j++) {
            if (tolower(str1[i]) == tolower(str2[j])) {
                common_chars++;
                break;
            }
        }
    }
    
    return (float)(2 * common_chars) / total_chars;
}

/**
 * @brief Log search query for analytics
 */
void log_search_query(const char *query, int results_count, double response_time) {
    if (!query) {
        return;
    }
    
    // Log to console (in production, this would go to a log file or analytics system)
    printf("SEARCH LOG: query='%s', results=%d, time=%.2fms\n", 
           query, results_count, response_time);
}