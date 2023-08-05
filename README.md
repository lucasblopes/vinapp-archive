# Vina++ Archive

This program was developed in C language for the subject Programming II of my graduation in Computer Science. It is a simple archiver that does not perform data compression.

## Usage

``` 
make
./vina++ <option> <archive> [member1 member2 member3 ...]
```
## Options : 

### Insert: -i

- Insert a file at the end of the Archive.
- if the file already exists, it will replace it.

### Append: -a

- Same as insert, but it will only replace it if the file is newer than the present in the archive.

### Move: -m

- Move a file in front of the target.

### Extract: -x

- Extracts the specified file. If no file is specified it will extract all files.

### Remove: -r

- Remove a file from the archive.

### List: -c

- List all files inside the archive. Similar to tar -tvf.

### Help: -h

- Print these instructions.

### Usage examples: 

- Includes the files "file.txt", "pic.jpg", and "expenses.ods" into backup.vpp </br> 
```./vina++ -i backup.vpp file.txt pic.jpg expenses.ods```

- Updates "expenses.ods" in backup.vpp if the external file is newer </br> 
```./vina++ -a backup.vpp expenses.ods```

- Extracts the file file.txt from backup.vpp </br> 
```./vina++ -x backup.vpp file.txt```

- Extracts all files from backup.vpp </br> 
```./vina++ -x backup.vpp```

- Moves the file "file.txt" after "expenses.ods" in backup.vpp </br> 
```./vina++ -m expenses.ods backup.vpp file.txt```

- Includes the files "xy/dir/file.txt", "/dir/pic.jpg" into new_backup.vpp as "./xy/dir/file.txt", "./dir/pic.jpg" </br> 
```./vina++ -i new_backup.vpp xy/dir/file.txt /dir/pic.jpg```

- Extracts all files from new_backup.vpp, creating (if it does not exist) the entire directory hierarchy indicated for the file. </br> 
```./vina++ -x novo_backup.vpp```

## Data structure

```c
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
``` 

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.
