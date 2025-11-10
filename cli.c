#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "minigit.h"
#include "search_engine.h"
#include "autocomplete.h"
#include "ranking.h"

void print_help() {
    printf("\n--- Mini-Git & Smart Search Engine ---\n");
    printf("Mini-Git Commands:\n");
    printf("  init                      - Initialize a new repository.\n");
    printf("  add <filename>            - Add a file to the staging area.\n");
    printf("  commit \"<message>\"        - Commit staged files.\n");
    printf("  log                       - View commit history.\n");
    printf("  view <commit_id>          - View details of a specific commit.\n");
    printf("  delete <commit_id>        - Delete a commit.\n");
    printf("\nSearch Engine Commands:\n");
    printf("  search <term>             - Perform full search with ranking.\n");
    printf("  suggest <prefix>          - Get autocomplete suggestions.\n");
    printf("\nGeneral Commands:\n");
    printf("  help                      - Show this help message.\n");
    printf("  exit                      - Quit the application.\n\n");
}

void handle_search(const char *term) {
    search_result_t results[MAX_RESULTS];
    int count = search_and_rank(term, results, MAX_RESULTS);

    printf("\nSearch results for '%s':\n", term);
    if (count == 0) {
        printf("  No results found.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("  %d. %s (Relevance: %.2f)\n",
               i + 1, results[i].title, results[i].relevance_score);
        printf("      %s\n", results[i].description);
        printf("      URL: %s\n\n", results[i].url);
    }
}

void handle_suggest(const char *term) {
    autocomplete_result_t suggestions[MAX_AUTOCOMPLETE_SUGGESTIONS];
    int count = get_autocomplete_suggestions(term, suggestions, MAX_AUTOCOMPLETE_SUGGESTIONS);

    printf("\nAutocomplete suggestions for '%s':\n", term);
    if (count == 0) {
        printf("  No suggestions found.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("  - %s (Score: %.2f)\n",
               suggestions[i].suggestion, suggestions[i].score);
    }
    printf("\n");
}

int main() {
    char input[MAX_INPUT_BUFFER];
    char *command, *argument;

    init_repository();
    init_search_engine();
    init_autocomplete_system();
    init_ranking_system();

    print_help();

    while (1) {
        printf("cli> ");

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        input[strcspn(input, "\n")] = 0;
        command = strtok(input, " ");
        if (!command) continue;
        argument = strtok(NULL, "");

        if (strcmp(command, "exit") == 0) {
            printf("Goodbye!\n");
            break;
        }
        else if (strcmp(command, "help") == 0) print_help();
        else if (strcmp(command, "init") == 0) init_repository();
        else if (strcmp(command, "add") == 0) argument ? add_file(argument) : printf("Usage: add <filename>\n");
        else if (strcmp(command, "commit") == 0) argument ? commit_staged(argument) : printf("Usage: commit \"<message>\"\n");
        else if (strcmp(command, "log") == 0) view_log();
        else if (strcmp(command, "view") == 0) argument ? view_commit(atoi(argument)) : printf("Usage: view <commit_id>\n");
        else if (strcmp(command, "delete") == 0) argument ? delete_commit(atoi(argument)) : printf("Usage: delete <commit_id>\n");
        else if (strcmp(command, "search") == 0) argument ? handle_search(argument) : printf("Usage: search <term>\n");
        else if (strcmp(command, "suggest") == 0) argument ? handle_suggest(argument) : printf("Usage: suggest <prefix>\n");
        else printf("Unknown command: '%s'. Type 'help' for assistance.\n", command);
    }

    cleanup_ranking_system();
    cleanup_autocomplete_system();
    cleanup_search_engine();
    return 0;
}

