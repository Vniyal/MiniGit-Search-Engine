/*
 * File: gui.c
 * Brief: GTK 4 GUI for the Mini-Git & Smart Search Engine.
 *
 * This file creates a tabbed window to interact with the
 * minigit and search engine functionalities.
 *
 * To compile, you'll need the GTK 4 development libraries.
 * See the updated Makefile for compilation instructions.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include your project's backend headers
#include "minigit.h"
#include "search_engine.h"
#include "autocomplete.h"
#include "ranking.h"

// --- Global GTK Widgets ---
// We make these global so callback functions can access them.

// Search Tab Widgets
GtkWidget *search_entry;
GtkWidget *suggestions_view;
GtkWidget *search_results_view;

// Mini-Git Tab Widgets
GtkWidget *git_output_view;
GtkWidget *git_filename_entry;
GtkWidget *git_commit_entry;
GtkWidget *git_commit_id_entry;

// --- Helper Functions ---

/**
 * @brief Sets the text in a GtkTextView.
 * @param text_view The GtkTextView widget.
 * @param text The string to set.
 */
static void set_text_view_text(GtkWidget *text_view, const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, text, -1);
}

/**
 * @brief Appends text to a GtkTextView.
 * @param text_view The GtkTextView widget.
 * @param text The string to append.
 */
static void append_text_view_text(GtkWidget *text_view, const char *text) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, text, -1);
}

// --- Search Engine Callbacks ---

/**
 * @brief Callback for the "Suggest" button.
 */
static void on_suggest_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters

    const char *prefix = gtk_editable_get_text(GTK_EDITABLE(search_entry));
    if (!prefix || strlen(prefix) == 0) {
        set_text_view_text(suggestions_view, "Please enter a prefix to get suggestions.");
        return;
    }

    autocomplete_result_t suggestions[MAX_AUTOCOMPLETE_SUGGESTIONS];
    int count = get_autocomplete_suggestions(prefix, suggestions, MAX_AUTOCOMPLETE_SUGGESTIONS);

    if (count == 0) {
        set_text_view_text(suggestions_view, "No suggestions found.");
        return;
    }

    // Build the output string
    GString *output = g_string_new("");
    for (int i = 0; i < count; i++) {
        g_string_append_printf(output, "- %s (Score: %.2f)\n",
                               suggestions[i].suggestion, suggestions[i].score);
    }

    set_text_view_text(suggestions_view, output->str);
    g_string_free(output, TRUE);
}

/**
 * @brief Callback for the "Search" button.
 */
static void on_search_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters

    const char *term = gtk_editable_get_text(GTK_EDITABLE(search_entry));
    if (!term || strlen(term) == 0) {
        set_text_view_text(search_results_view, "Please enter a search term.");
        return;
    }

    search_result_t results[MAX_SEARCH_RESULTS]; // From cli.h (or implied)
    int count = search_and_rank(term, results, MAX_SEARCH_RESULTS);

    if (count == 0) {
        set_text_view_text(search_results_view, "No results found.");
        return;
    }

    // Build the output string
    GString *output = g_string_new("");
    for (int i = 0; i < count; i++) {
        g_string_append_printf(output, "%d. %s (Relevance: %.2f)\n",
                               i + 1, results[i].title, results[i].relevance_score);
        g_string_append_printf(output, "   %s\n", results[i].description);
        g_string_append_printf(output, "   URL: %s\n\n", results[i].url);
    }

    set_text_view_text(search_results_view, output->str);
    g_string_free(output, TRUE);
}

// --- Mini-Git Callbacks ---

/**
 * @brief Callback for the "Initialize Repository" button.
 */
static void on_init_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters
    init_repository();
    set_text_view_text(git_output_view, "Repository has been initialized.\n");
}

/**
 * @brief Callback for the "Add File" button.
 */
static void on_add_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters
    const char *filename = gtk_editable_get_text(GTK_EDITABLE(git_filename_entry));
    if (!filename || strlen(filename) == 0) {
        set_text_view_text(git_output_view, "Error: Please enter a filename to add.\n");
        return;
    }

    // The add_file function takes a char*, not const char*
    char *filename_copy = strdup(filename);
    add_file(filename_copy);
    free(filename_copy);
    
    // add_file prints its own message, so let's just confirm it
    char output[256];
    snprintf(output, sizeof(output), "File '%s' added to index.\n(See console for backend output)\n", filename);
    set_text_view_text(git_output_view, output);
    gtk_editable_set_text(GTK_EDITABLE(git_filename_entry), ""); // Clear entry
}

/**
 * @brief Callback for the "Commit" button.
 */
static void on_commit_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters
    const char *message = gtk_editable_get_text(GTK_EDITABLE(git_commit_entry));
    if (!message || strlen(message) == 0) {
        set_text_view_text(git_output_view, "Error: Please enter a commit message.\n");
        return;
    }

    char *message_copy = strdup(message);
    commit_staged(message_copy);
    free(message_copy);
    
    // commit_staged prints its own message
    set_text_view_text(git_output_view, "Commit successful.\n(See console for backend output)\n");
    gtk_editable_set_text(GTK_EDITABLE(git_commit_entry), ""); // Clear entry
}

/**
 * @brief Callback for the "View Log" button.
 * This re-implements the view_log logic to capture output.
 */
static void on_log_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters
    
    Commit *temp = repo.head;
    if (!temp) {
        set_text_view_text(git_output_view, "No commits yet.\n");
        return;
    }

    GString *output = g_string_new("Commit Log:\n");
    while (temp) {
        g_string_append_printf(output, "Commit %d: %s\n", temp->commit_id, temp->message);
        temp = temp->next;
    }
    
    set_text_view_text(git_output_view, output->str);
    g_string_free(output, TRUE);
}

/**
 * @brief Callback for the "View Commit" button.
 */
static void on_view_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters
    const char *id_str = gtk_editable_get_text(GTK_EDITABLE(git_commit_id_entry));
    int cid = atoi(id_str);

    if (cid <= 0) {
        set_text_view_text(git_output_view, "Error: Please enter a valid commit ID.\n");
        return;
    }

    Commit *temp = repo.head;
    while (temp) {
        if (temp->commit_id == cid) {
            char output[512];
            snprintf(output, sizeof(output), "Details for Commit %d:\n%s\n", temp->commit_id, temp->message);
            set_text_view_text(git_output_view, output);
            return;
        }
        temp = temp->next;
    }
    set_text_view_text(git_output_view, "Commit not found.\n");
}

/**
 * @brief Callback for the "Delete Commit" button.
 */
static void on_delete_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button; (void)user_data; // Unused parameters
    const char *id_str = gtk_editable_get_text(GTK_EDITABLE(git_commit_id_entry));
    int cid = atoi(id_str);

    if (cid <= 0) {
        set_text_view_text(git_output_view, "Error: Please enter a valid commit ID.\n");
        return;
    }

    // We can just call the backend function
    delete_commit(cid);

    // And then refresh the log view
    on_log_button_clicked(NULL, NULL);
    append_text_view_text(git_output_view, "\n...Attempted to delete commit.\n(See console for backend output)\n");
}


// --- GUI Setup ---

/**
 * @brief Creates the Search Engine tab.
 */
static GtkWidget* create_search_tab(void) {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);

    // Search Entry
    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Enter search term...");
    
    GtkWidget *suggest_button = gtk_button_new_with_label("Suggest");
    g_signal_connect(suggest_button, "clicked", G_CALLBACK(on_suggest_button_clicked), NULL);

    GtkWidget *search_button = gtk_button_new_with_label("Search");
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_button_clicked), NULL);

    // Suggestions View
    GtkWidget *suggest_scrolled_win = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(suggest_scrolled_win, TRUE);
    suggestions_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(suggestions_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(suggestions_view), FALSE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(suggest_scrolled_win), suggestions_view);

    // Search Results View
    GtkWidget *results_scrolled_win = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(results_scrolled_win, TRUE);
    search_results_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(search_results_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(search_results_view), FALSE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(results_scrolled_win), search_results_view);
    
    // Layout
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Query:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), search_entry, 1, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), suggest_button, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), search_button, 4, 0, 1, 1);
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Suggestions:"), 0, 1, 5, 1);
    gtk_grid_attach(GTK_GRID(grid), suggest_scrolled_win, 0, 2, 5, 1);
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Search Results:"), 0, 3, 5, 1);
    gtk_grid_attach(GTK_GRID(grid), results_scrolled_win, 0, 4, 5, 1);

    return grid;
}

/**
 * @brief Creates the Mini-Git tab.
 */
static GtkWidget* create_minigit_tab(void) {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);

    // Controls
    GtkWidget *init_button = gtk_button_new_with_label("Initialize Repository");
    g_signal_connect(init_button, "clicked", G_CALLBACK(on_init_button_clicked), NULL);

    git_filename_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(git_filename_entry), "filename.txt");
    GtkWidget *add_button = gtk_button_new_with_label("Add File");
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_button_clicked), NULL);

    git_commit_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(git_commit_entry), "Commit message");
    GtkWidget *commit_button = gtk_button_new_with_label("Commit");
    g_signal_connect(commit_button, "clicked", G_CALLBACK(on_commit_button_clicked), NULL);

    GtkWidget *log_button = gtk_button_new_with_label("View Log");
    g_signal_connect(log_button, "clicked", G_CALLBACK(on_log_button_clicked), NULL);

    git_commit_id_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(git_commit_id_entry), "Commit ID");
    GtkWidget *view_button = gtk_button_new_with_label("View");
    g_signal_connect(view_button, "clicked", G_CALLBACK(on_view_button_clicked), NULL);
    GtkWidget *delete_button = gtk_button_new_with_label("Delete");
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_button_clicked), NULL);

    // Output View
    GtkWidget *output_scrolled_win = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(output_scrolled_win, TRUE);
    git_output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(git_output_view), FALSE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(output_scrolled_win), git_output_view);

    // Layout
    gtk_grid_attach(GTK_GRID(grid), init_button, 0, 0, 3, 1);
    
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("File:"), 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), git_filename_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), add_button, 2, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Msg:"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), git_commit_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), commit_button, 2, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), log_button, 0, 3, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ID:"), 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), git_commit_id_entry, 1, 4, 1, 1);
    GtkWidget *id_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_append(GTK_BOX(id_box), view_button);
    gtk_box_append(GTK_BOX(id_box), delete_button);
    gtk_grid_attach(GTK_GRID(grid), id_box, 2, 4, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Output/Log:"), 0, 5, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), output_scrolled_win, 0, 6, 3, 1);

    return grid;
}

/**
 * @brief Called when the application is activated.
 */
static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data; // Unused parameter

    // Create the main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Mini-Git & Search Engine GUI");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 600);

    // Create the Notebook (tabbed interface)
    GtkWidget *notebook = gtk_notebook_new();
    gtk_window_set_child(GTK_WINDOW(window), notebook);

    // Create and add tabs
    GtkWidget *search_tab_content = create_search_tab();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), search_tab_content, gtk_label_new("Search Engine"));

    GtkWidget *minigit_tab_content = create_minigit_tab();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), minigit_tab_content, gtk_label_new("Mini-Git"));

    gtk_widget_show(window);
}

/**
 * @brief Main function for the GTK application.
 */
int main(int argc, char *argv[]) {
    // --- Initialize Backend Systems ---
    // These print to console, which is fine for init.
    printf("Initializing backend systems...\n");
    init_repository();
    init_search_engine();
    init_autocomplete_system();
    init_ranking_system();
    printf("Backend systems initialized.\n");
    
    // --- Start GTK Application ---
    GtkApplication *app = gtk_application_new("com.example.minigitsearchgui", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    
    g_object_unref(app);

    // --- Cleanup Backend Systems ---
    printf("Cleaning up backend systems...\n");
    cleanup_ranking_system();
    cleanup_autocomplete_system();
    cleanup_search_engine();
    printf("Cleanup complete. Exiting.\n");

    return status;
}