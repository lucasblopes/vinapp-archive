#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <stdio.h>
#include <stdlib.h>

#include "directory.h"

/* read metadata from an existing archive */
int archive_directory_read(struct Directory *dir);

/* insert a member into the directory and write its contents to the archive */
int archive_insert(struct Directory *dir, const char *path, int flag_i);

/* extract a member in the local directory */
int archive_extract(struct Directory *dir, char *path);

/* remove a member from the archive */
int archive_remove(struct Directory *dir, long index);

/* write the updated directory at the end of the archive */
int archive_directory_write(struct Directory *dir);

#endif

