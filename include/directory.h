#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

struct Directory {
    FILE *archive;       /* pointer to archive */
    struct Member *m;    /* matadata's array */ 
    int modified;        /* if is equal to 1 update archive */
    char *path;          /* archive's path */
    long pos;            /* directory position */
    long n;              /* number of members */
};

struct Member { 
    char *path;          /* member's path */
    int plength;         /* path's size */
    long order;          /* insertion order */
    long pos;            /* member's position in the archive */
    uid_t uid;           /* user id */
    gid_t gid;           /* group id */
    mode_t mode;         /* permissions */
    size_t size;         /* member's size */
    time_t mtime;        /* modification time */
};

/* initialize the directory */
struct Directory *directory_init(const char *path);

/* returns the index of path in directory */
long directory_search_path(struct Directory *dir, const char *path);

/* returns the index corresponding to the insertion order */
long directory_search_order(struct Directory *dir, long order);

/* returns the index of the last inserted member */
long directory_last_member(struct Directory *dir);

/* insert member at end of directory */
int directory_insert(struct Directory *dir, struct stat st_file, const char *path);

/* moves scr to dest in the array */
int directory_move(struct Directory *dir, long dest, long src);

/* print metadata of all archive members */
int directory_list(struct Directory *dir);

/* free allocated memory */
void directory_free(struct Directory **dir);

#endif
