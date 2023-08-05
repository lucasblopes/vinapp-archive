#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>

#include "../include/tools.h"

#define BUFFER_SIZE 1024 /* max buffer size */

/* print the program's instrctions */
void print_help() {

    printf("Usage: vina++ <option> <archive> [member1 member2 ...]\n");
    printf("\n");
    printf("Options:\n");   
    printf("    -i        : inserts/adds one or more members to the archive. If the member already exists in the archive, it must be replaced.\n");
    printf("    -a        : same as -i, but replaces an existing member ONLY if the parameter is newer than the archived one.\n");
    printf("    -m target : moves the member indicated on the command line to immediately after the existing target member in the file.\n");
    printf("    -x        : extract indicated members from archive. If none are indicated, then extract all\n");
    printf("    -r        : remove members from archive.\n");
    printf("    -c        : list content in archive.\n");
    printf("    -h        : print this message.\n");

}

/* copy size bytes from read to write */
int copy_file(FILE *write, FILE *read, long write_pos, long read_pos, size_t size) {

    assert(write != NULL && read != NULL);

    char buffer[BUFFER_SIZE];

    fseek(write, write_pos, SEEK_SET);
    fseek(read, read_pos, SEEK_SET);

    size_t remaining_size = size;
    size_t bytes_to_read, write_size;

    while (remaining_size > 0) {
        bytes_to_read = (remaining_size < BUFFER_SIZE) ? remaining_size : BUFFER_SIZE;
        bytes_to_read = fread(buffer, sizeof(char), bytes_to_read, read);
        if (bytes_to_read <= 0) {
            fclose(read);
            perror("Error reading data from file!");
            return 1;
        }
        write_size = fwrite(buffer, sizeof(char), bytes_to_read, write);
        if (write_size != bytes_to_read) {
            fclose(read);
            perror("Erro ao escrever dados no archive!");
            return 1;
        }
        remaining_size -= bytes_to_read;
    }
    return 0;
}

/* create directory hierarchy recursively */
int create_directory_hierarchy(char *path) {

    char* substr = strrchr(path, '/');

    int err = 0;
    if(substr) {
        *substr = 0;
        err = create_directory_hierarchy(path);
        *substr = '/';

        if (err) {
            return 1;
        }
    }
    if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) && errno != EEXIST) {
        return 1;
    }
    return 0;
} 

/* returns FILE after creating the local directory hierarchy */
FILE *create_file(struct Directory *dir, long index) {
    
    assert(dir != NULL && index >= 0);

    char *relative_path = strdup(dir->m[index].path);
    if (!relative_path) {
        perror("malloc");
        return NULL;
    }

    if (relative_path[0] == '/') {
        snprintf(relative_path, dir->m[index].plength + 1, ".%s", dir->m[index].path);
    }

    char *s = strrchr(relative_path, '/');
    if (s) {
        s[0] = '\0';
        create_directory_hierarchy(relative_path);
        s[0] = '/';
    }

    FILE *file = fopen(relative_path, "wb");
    chmod(relative_path, dir->m[index].mode);
    chown(relative_path, dir->m[index].uid, dir->m[index].gid);

    if (!file) {
        free(relative_path);
        return NULL;
    }

    free(relative_path);

    return file;
}

/* compare two void pointers */
int long_compare(const void *a, const void *b) {
    
    assert(a != NULL && b != NULL);

    long *pa = (long*) a;
    long *pb = (long*) b;
    
    if (*pa > *pb) return  1 ;
    if (*pa < *pb) return -1 ;
    return 0 ;
}

/* print mode in text */
void print_mode(mode_t mode) {
    
    printf((S_ISDIR(mode))  ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
    return;
}
