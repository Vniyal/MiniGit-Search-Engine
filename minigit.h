#ifndef MINIGIT_H
#define MINIGIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Commit struct */
typedef struct Commit {
    int commit_id;
    char message[256];
    struct Commit *next;
} Commit;

/* File struct for staging index */
typedef struct File {
    char filename[100];
    struct File *next;
} File;

/* Repository wrapper */
typedef struct Repository {
    Commit *head;
    int commit_count;
} Repository;

/* Globals (declared in minigit.c) */
extern Repository repo;
extern File *index_head;

/* API */
void init_repository(void);
void add_file(char *filename);
void commit_staged(char *msg);
void view_commit(int cid);
void delete_commit(int cid);
void view_log(void);

#endif /* MINIGIT_H */
