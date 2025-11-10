#include "minigit.h"

/* Globals */
Repository repo;
File *index_head = NULL;

/* Initialize repository */
void init_repository() {
    repo.head = NULL;
    repo.commit_count = 0;
    printf("Repository has been initialized.\n");
}

/* Add file to staging index */
void add_file(char *filename) {
    if (!filename || strlen(filename) == 0) {
        printf("Invalid filename.\n");
        return;
    }
    File *new_file = (File *)malloc(sizeof(File));
    if (!new_file) {
        printf("Memory allocation failed.\n");
        return;
    }
    strncpy(new_file->filename, filename, sizeof(new_file->filename) - 1);
    new_file->filename[sizeof(new_file->filename) - 1] = '\\0';
    new_file->next = index_head;
    index_head = new_file;
    printf("File '%s' added to index.\n", new_file->filename);
}

/* Commit staged files */
void commit_staged(char *msg) {
    if (index_head == NULL) {
        printf("No files to commit.\n");
        return;
    }
    Commit *new_commit = (Commit *)malloc(sizeof(Commit));
    if (!new_commit) {
        printf("Memory allocation failed for commit.\n");
        return;
    }
    new_commit->commit_id = ++repo.commit_count;
    strncpy(new_commit->message, msg ? msg : "No message", sizeof(new_commit->message) - 1);
    new_commit->message[sizeof(new_commit->message) - 1] = '\\0';
    new_commit->next = repo.head;
    repo.head = new_commit;
    printf("Commit %d added with message: %s\n", new_commit->commit_id, new_commit->message);

    /* Clear staging */
    while (index_head != NULL) {
        File *tmp = index_head;
        index_head = index_head->next;
        free(tmp);
    }
}

/* View a specific commit */
void view_commit(int cid) {
    Commit *temp = repo.head;
    while (temp) {
        if (temp->commit_id == cid) {
            printf("Details for Commit %d: %s\n", temp->commit_id, temp->message);
            return;
        }
        temp = temp->next;
    }
    printf("Commit not found.\n");
}

/* Delete commit */
void delete_commit(int cid) {
    Commit *temp = repo.head, *prev = NULL;
    while (temp != NULL && temp->commit_id != cid) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) {
        printf("Commit not found.\n");
        return;
    }
    if (prev == NULL)
        repo.head = temp->next;
    else
        prev->next = temp->next;
    free(temp);
    printf("Commit %d deleted.\n", cid);
}

/* Print commit log (latest -> oldest) */
void view_log(void) {
    Commit *temp = repo.head;
    if (!temp) {
        printf("No commits yet.\n");
        return;
    }
    while (temp) {
        printf("Commit %d: %s\n", temp->commit_id, temp->message);
        temp = temp->next;
    }
}
