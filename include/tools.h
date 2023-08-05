#ifndef TOOLS_H
#define TOOLS_H

#include <stdlib.h>
#include <stdio.h>
#include "directory.h"

/* print the program's instructions */
void print_help();

/* copy size bytes from read to write */
int copy_file(FILE *write, FILE *read, long write_pos, long read_pos, size_t size);

/* returns FILE after creating the local directory hierarchy */
FILE *create_file(struct Directory *dir, long index);

/* compare two void pointers */
int long_compare(const void *a, const void *b);

/* print mode in text */
void print_mode(mode_t mode);

#endif

