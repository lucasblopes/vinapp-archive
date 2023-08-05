#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/archive.h"
#include "../include/directory.h"
#include "../include/tools.h"

int main(int argc, char **argv) {

    if (argc < 3 ) {
        print_help();
    }

    const char *archive_path;
    if (strcmp(argv[1], "-m") != 0) {
        archive_path = argv[2];
    } else {
        archive_path = argv[3];
    }

    struct Directory *directory = directory_init(archive_path);
    if (!directory) {
        return 1;
    }

    if (directory->archive) {
        if (archive_directory_read(directory) != 0) {
            perror("Failed to read archive directory!");
            return 1;
        }
    } else if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "-a") == 0) {
        directory->archive = fopen(directory->path, "w+b");
        if (!directory->archive) {
            perror("Failed to create archive");
            return 1;
        }
    }
     
    int op;
    while ((op = getopt (argc, argv, "iamxrc:")) != -1) {
        switch (op) {
            case 'i':  
                if (argc < 4) {
                    perror("Insufficient arguments for '-i'");
                    print_help();
                    return 1;
                }
                for (int i = 3; i < argc; i++) {
                    if (archive_insert(directory, argv[i], 1) != 0) {
                        printf("Failed to add member '%s' to archive.\n", argv[i]);
                    }
                }
                break;
            case 'a':  
                if (argc < 4) {
                    perror("Insufficient arguments for '-a'");
                    print_help();
                    return 1;
                }
                for (int i = 3; i < argc; i++) {
                    if (archive_insert(directory, argv[i], 0) != 0) {
                        printf("Failed to add member '%s' to archive.\n", argv[i]);
                    }
                }
                break;
            case 'm':
                if (argc < 5) {
                    perror("Insufficient arguments for '-m target'");
                    print_help();
                    return 1;
                }
                long target = directory_search_path(directory, argv[2]) + 1;
                long member = directory_search_path(directory, argv[4]);
                if (directory_move(directory, target, member) != 0) {
                    printf("Failed to move %s\n", argv[4]);
                    return 1;
                }
                break;
            case 'x':
                if (argc < 3) {
                    perror("Insufficient arguments for '-x'");
                    print_help();
                    return 1;
                } else if (argc == 3) {
                    if (archive_extract(directory, 0) != 0) {
                        printf("Failed to extract files from '%s'\n", argv[2]);
                    }
                } else {
                    for (int i = 3; i < argc; i++) {
                        if (archive_extract(directory, argv[i]) != 0) {
                            printf("Failed to extract member '%s' to archive.\n", argv[i]);
                        }
                    }
                }
                break;
            case 'r':
                if (argc < 4) {
                    perror("Insufficient arguments for '-x'");
                    return 1;
                }
                for (int i = 3; i < argc; i++) {
                    if (archive_remove(directory, directory_search_path(directory, argv[i])) != 0) {
                        printf("Failed to remove member '%s' from archive.\n", argv[i]);
                    }
                }
                break;
            case 'c':  
                if (argc != 3) {
                    print_help();
                    return 1;
                }
                if (directory_list(directory) != 0) {
                    perror("Archive empty or not found!");
                    return 1;
                } 
                break;
            default:
                print_help();
                break;
        }
    }

    if (directory->modified > 0) {
        if (archive_directory_write(directory) != 0) {
            perror("Unable to update archive!");
            return 1;
        }
    }
    directory_free(&directory);
    return 0;
}
