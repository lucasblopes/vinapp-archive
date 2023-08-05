#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "../include/archive.h"
#include "../include/directory.h"
#include "../include/tools.h"

/* read metadata from an existing archive */
int archive_directory_read(struct Directory *dir) {

    assert(dir != NULL);

    fread(&dir->pos, sizeof(long), 1, dir->archive);

    fseek(dir->archive, dir->pos, SEEK_SET);

    fread(&dir->n, sizeof(long), 1, dir->archive);

    dir->m = malloc((dir->n)  *sizeof(struct Member));
    if (!dir->m) {
        return 1;
    }
    for (long i = 0; i < dir->n; i++) {
        fread(&dir->m[i].plength, sizeof(int), 1, dir->archive);
        dir->m[i].path = malloc(dir->m[i].plength);
        if (!dir->m[i].path) {
            free(dir->m[i].path);
            return 1;
        }
        fread(dir->m[i].path, dir->m[i].plength, 1, dir->archive);
        fread(&dir->m[i].order, sizeof(long), 1, dir->archive);
        fread(&dir->m[i].pos, sizeof(long), 1, dir->archive);
        fread(&dir->m[i].uid, sizeof(uid_t), 1, dir->archive);
        fread(&dir->m[i].gid, sizeof(gid_t), 1, dir->archive);
        fread(&dir->m[i].mode, sizeof(mode_t), 1, dir->archive);
        fread(&dir->m[i].size, sizeof(size_t), 1, dir->archive);
        fread(&dir->m[i].mtime, sizeof(time_t), 1, dir->archive);
    }
    return 0;
}

/* insert a member into the directory and write its contents to the archive */
int archive_insert(struct Directory *dir, const char *path, int flag_i) {

    assert(dir != NULL && path != NULL);

    /* open an external file */
    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("Unable to open external file!");
        return 1;
    }

    /* stores the file's metadata */
    struct stat st_file;
    if (stat(path, &st_file) == -1) {
        perror("Unable to get information about external file");
        fclose(file);
        return 1;
    }

    long index = directory_search_path(dir, path);

    /* replaces if it is -i or -a and the external file is newer */
    if (index != -1) {
        if (flag_i || difftime(st_file.st_mtime, dir->m[index].mtime) > 0) { 
            archive_remove(dir, index);
        } 
    }

    /* write the new member to the directory */
    if (directory_insert(dir, st_file, path) != 0) {
        perror("Failed to add new member to directory!");
        fclose(file);
        return 1;
    }

    /* copy the contents of the external file into the archive  */
    if (copy_file(dir->archive, file, dir->m[dir->n-1].pos, 0, st_file.st_size) != 0) {
        perror("Failed to write data to archive");
        fclose(file);
        return 1;
    }
    fclose(file);

    /* if replaced adjust order in directory */
    if (index != -1) {
        if (directory_move(dir, index, dir->n-1) != 0) {
            perror("Failed to move the replaced member to its initial position.");
            return 1;
        }
    }
    dir->modified = 1;
    return 0;
}

/* extract a member in the local directory */
int archive_extract(struct Directory *dir, char *path) {

    assert(dir != NULL && path != NULL);

    if (!dir->archive || !dir->n) {
        perror("Archive empty or not found");
        return 1;
    }
    if (!path) {
        /* extract all the members  */
        for (long i = 0; i < dir->n; i++) {
            
            /* create directory hierarchy and open file */
            FILE *file = create_file(dir, i);
            if (!file) {
                return 1;
            }

            copy_file(file, dir->archive, 0, dir->m[i].pos, dir->m[i].size);
            fclose(file);
        }
    } else {
        /* extract specified member */
        long index = directory_search_path(dir, path);
        if (index == -1) {
            perror("Member not found in archive!");
            return 1;
        }

        /* create directory hierarchy and open file */
        FILE *file = create_file(dir, index);
        if (!file) {
            return 1;
        }

        copy_file(file, dir->archive, 0, dir->m[index].pos, dir->m[index].size);
        fclose(file);
    }
    return 0;
}

/* remove a member from the archive */
int archive_remove(struct Directory *dir, long index) {

    assert(dir != NULL && index >= 0);

    if (!dir->archive || !dir || index < 0) {
        return 1;
    }

    FILE *write = fopen(dir->path, "r+b");
    if (!write) {
        return 1;
    }

    long write_pos = dir->m[index].pos;

    /* temp stores ascending order of members */
    long *order_tmp = malloc(dir->n  *sizeof(long));
    if (!order_tmp) {
        free(order_tmp);
        fclose(write);
        return 1;
    }
    for(long i = 0; i < dir->n; i++) {
        order_tmp[i] = dir->m[i].order;
    }

    qsort(order_tmp, dir->n, sizeof(long), long_compare);
    long i = 0;

    /* performs the left shift to fill the removed space */
    do {
        if (order_tmp[i] > dir->m[index].order) {
            long ind = directory_search_order(dir, order_tmp[i]);
            copy_file(write, dir->archive, write_pos, dir->m[ind].pos, dir->m[ind].size);
            dir->m[ind].pos = write_pos;
            write_pos += dir->m[ind].size;
        }
        i++;
    } while (i < dir->n);

    ftruncate(fileno(dir->archive), (off_t) write_pos);
    fclose(write);

    free(order_tmp);
    free(dir->m[index].path); 

    /* adjust the directory */
    for (long i = index; i < dir->n - 1; i++) {
        dir->m[i] = dir->m[i+1];
    }
    dir->n--;
    dir->modified = 1;

    /* archive update */
    dir->pos -= dir->m[index].size;
    fseek(dir->archive, 0, SEEK_SET);
    fwrite(&dir->pos, sizeof(long), 1, dir->archive);

    return 0;
}  

/* write the updated directory at the end of the archive */
int archive_directory_write(struct Directory *dir) {

    assert(dir->archive != NULL && dir != NULL);

    if (dir->m && dir->n > 0) {
        long b = directory_last_member(dir);
        dir->pos = dir->m[b].pos + dir->m[b].size;
    } else {
        dir->pos = sizeof(long);
    }
    fseek(dir->archive, dir->pos, SEEK_SET);
 
    fwrite(&dir->n, sizeof(long), 1, dir->archive);

    for(int i = 0; i < dir->n; i++) {
        fwrite(&dir->m[i].plength, sizeof(int), 1, dir->archive);
        fwrite(dir->m[i].path, dir->m[i].plength, 1, dir->archive);
        fwrite(&dir->m[i].order, sizeof(long), 1, dir->archive);
        fwrite(&dir->m[i].pos, sizeof(long), 1, dir->archive);
        fwrite(&dir->m[i].uid, sizeof(uid_t), 1, dir->archive);
        fwrite(&dir->m[i].gid, sizeof(gid_t), 1, dir->archive);
        fwrite(&dir->m[i].mode, sizeof(mode_t), 1, dir->archive);
        fwrite(&dir->m[i].size, sizeof(size_t), 1, dir->archive);
        fwrite(&dir->m[i].mtime, sizeof(time_t), 1, dir->archive);
    }

    fseek(dir->archive, 0, SEEK_SET);
    fwrite(&dir->pos, sizeof(long), 1, dir->archive);
    return 0;
}

