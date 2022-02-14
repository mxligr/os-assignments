#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#define MAX_PATH_LEN 256

void listDir(char *dirName) {
    DIR *dir;
    struct dirent *dirEntry;
    char name[MAX_PATH_LEN];

    dir = opendir(dirName);
    if (dir == NULL) {
        printf("ERROR\ninvalid directory path\n");
        return;
    } else {
        printf("SUCCESS\n");
        // iterate the directory contents
        while ((dirEntry = readdir(dir)) != 0) {
            // build the complete path to the element in the directory
            if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0) {
                if (dirEntry->d_type == DT_REG || dirEntry->d_type == DT_DIR) {
                    snprintf(name, PATH_MAX, "%s/%s", dirName, dirEntry->d_name);
                    printf("%s\n", name);
                }
            }
        }
    }

    closedir(dir);
}

static int statusListDirRecursive(const char *dir_name) {
    DIR *d;

    /* Open the directory specified by "dir_name". */

    d = opendir(dir_name);

    /* Check it was opened. */
    if (!d) {
        printf("ERROR\ninvalid directory path\n");
        return -1;
    }

    while (1) {
        struct dirent *entry;
        const char *d_name;

        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir(d);
        if (!entry) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = entry->d_name;
        /* Print the name of the file and directory. */
        if (!(entry->d_type & DT_DIR)) {
            if (strcmp(d_name, ".") != 0 && strcmp(d_name, "..") != 0) {
                //printf("%s/%s\n", dir_name, d_name);
            }
        }

        if (entry->d_type & DT_DIR) {

            /* Check that the directory is not "d" or d's parent. */

            if (strcmp(d_name, "..") != 0 &&
                strcmp(d_name, ".") != 0) {
                int path_length;
                char path[MAX_PATH_LEN];

                path_length = snprintf(path, PATH_MAX, "%s/%s", dir_name, d_name);
                //printf("%s\n", path);
                if (path_length >= PATH_MAX) {
                    printf("ERROR\npath name too long\n");
                    return -1;
                }
                /* Recursively call "list_dir" with the new path. */
                statusListDirRecursive(path);
            }
        }
    }
    /* After going through all the entries, close the directory. */
    if (closedir(d)) {
        printf("ERROR\ncould not close directory\n");
        return -1;
    }
    return 0;
}

// example code taken from https://www.lemoda.net/c/recursive-directory/
static void listDirRecursive(const char *dir_name) {
    DIR *d;

    /* Open the directory specified by "dir_name". */

    d = opendir(dir_name);

    /* Check it was opened. */
    if (!d) {
        printf("ERROR\ninvalid directory path\n");
        return;
    }

    while (1) {
        struct dirent *entry;
        const char *d_name;

        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir(d);
        if (!entry) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = entry->d_name;
        /* Print the name of the file and directory. */
        if (!(entry->d_type & DT_DIR)) {
            if (strcmp(d_name, ".") != 0 && strcmp(d_name, "..") != 0) {
                printf("%s/%s\n", dir_name, d_name);
            }
        }

        if (entry->d_type & DT_DIR) {

            /* Check that the directory is not "d" or d's parent. */

            if (strcmp(d_name, "..") != 0 &&
                strcmp(d_name, ".") != 0) {
                int path_length;
                char path[PATH_MAX];

                path_length = snprintf(path, PATH_MAX, "%s/%s", dir_name, d_name);
                printf("%s\n", path);
                if (path_length >= PATH_MAX) {
                    printf("ERROR\npath name too long\n");
                    return;
                }
                /* Recursively call "list_dir" with the new path. */
                listDirRecursive(path);
            }
        }
    }
    /* After going through all the entries, close the directory. */
    if (closedir(d)) {
        printf("ERROR\ncould not close directory\n");
        return;
    }
}

void listDirSize(char *dirName, off_t size) {
    DIR *dir;
    struct dirent *dirEntry;
    char name[MAX_PATH_LEN];
    struct stat inode;

    dir = opendir(dirName);
    if (dir == NULL) {
        printf("ERROR\ninvalid directory path\n");
        return;
    } else {
        printf("SUCCESS\n");
        // iterate the directory contents
        while ((dirEntry = readdir(dir)) != 0) {
            // build the complete path to the element in the directory
            if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0) {
                if (dirEntry->d_type == DT_REG || dirEntry->d_type == DT_DIR) {
                    snprintf(name, PATH_MAX, "%s/%s", dirName, dirEntry->d_name);
                    lstat(name, &inode);
                    if (S_ISREG(inode.st_mode)) {
                        if (inode.st_size > size) {
                            printf("%s\n", name);
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
}

static void listDirRecursiveSize(const char *name, off_t size) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            listDirRecursiveSize(path, size);
        } else {
            char path[PATH_MAX];
            struct stat inode;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            lstat (path, &inode);
            if(inode.st_size > size) {
                printf("%s\n", path);
            }
        }
    }
    closedir(dir);
}

void listDirName(char *dirName, char *startsWith) {
    DIR *dir;
    struct dirent *dirEntry;
    char name[MAX_PATH_LEN];

    dir = opendir(dirName);
    if (dir == NULL) {
        printf("ERROR\ninvalid directory path\n");
        return;
    } else {
        printf("SUCCESS\n");
        // iterate the directory contents
        while ((dirEntry = readdir(dir)) != 0) {
            // build the complete path to the element in the directory
            if (strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0) {
                if (dirEntry->d_type == DT_REG || dirEntry->d_type == DT_DIR) {
                    snprintf(name, PATH_MAX, "%s/%s", dirName, dirEntry->d_name);
                    if (strncmp(dirEntry->d_name, startsWith, strlen(startsWith)) == 0) {
                        printf("%s\n", name);
                    }
                }
            }
        }
    }

    closedir(dir);
}

static void listDirRecursiveName(const char *dir_name, char *startsWith) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dir_name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);
            listDirRecursiveName(path, startsWith);
        } else {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dir_name, entry->d_name);
            if(strncmp(entry->d_name, startsWith, strlen(startsWith)) == 0) {
                printf("%s\n", path);
            }
        }
    }
    closedir(dir);
}

char *getArgument(char *arg2) {
    char *pch;
    char *path = (char *) malloc(sizeof(char) * MAX_PATH_LEN);
    strtok(arg2, "=");
    pch = strtok(NULL, "=");
    strcpy(path, pch);

    return path;
}

void readInput2(char *arg1, char *arg2) {
    if (strcmp(arg1, "list") == 0) {
        // store the path of the directory in path
        char *path = getArgument(arg2);
        listDir(path);
        free(path);

    } else if (strcmp(arg2, "list") == 0) {
        // store the path of the directory in path
        char *path = getArgument(arg2);
        listDir(path);
        free(path);
    }
}

void doListDirRecursive(char *arg) {
    char *path = getArgument(arg);
    if (statusListDirRecursive(path) == 0) {
        printf("SUCCESS\n");
        listDirRecursive(path);
    }

    free(path);
}

void readInputListRecursive(char *arg1, char *arg2, char *arg3) {
    if (strncmp(arg3, "path=", 5) == 0) {
        doListDirRecursive(arg3);
    } else if (strncmp(arg2, "path=", 5) == 0) {
        doListDirRecursive(arg2);
    } else if (strncmp(arg1, "path=", 5) == 0) {
        doListDirRecursive(arg1);
    }
}

void doListDirSize(char *path, char *sizeStr) {
    char *sizeString = getArgument(sizeStr);
    int size = atoi(sizeString);
    off_t size_o = (off_t) size;
    listDirSize(path, size_o);
    free(sizeString);
}

void doListDirRecursiveSize(char *path, char *sizeStr) {
    char *sizeString = getArgument(sizeStr);
    long size = atol(sizeString);
    off_t size_o = (off_t)size;
    if (statusListDirRecursive(path) == 0) {
        printf("SUCCESS\n");
        listDirRecursiveSize(path, size_o);
    }
    free(sizeString);
}

void doListDirName(char *path, char *arg) {
    char *nameStartsWith = getArgument(arg);
    listDirName(path, nameStartsWith);
    free(nameStartsWith);
}

void doListDirRecursiveName(char *path, char *arg) {
    char *nameStartsWith = getArgument(arg);
    if (statusListDirRecursive(path) == 0) {
        printf("SUCCESS\n");
        listDirRecursiveName(path, nameStartsWith);
    }
    free(nameStartsWith);
}

void readInputListFiltering(char *arg1, char *arg2, char *arg3) {
    if (strcmp(arg1, "list") == 0 || strcmp(arg2, "list") == 0 || strcmp(arg3, "list") == 0) {
        if (strncmp(arg1, "path=", 5) == 0) {
            char *path = getArgument(arg1);
            if (strncmp(arg2, "size_greater=", 13) == 0) {
                doListDirSize(path, arg2);
            } else if (strncmp(arg3, "size_greater=", 13) == 0) {
                doListDirSize(path, arg3);
            } else if (strncmp(arg2, "name_starts_with=", 17) == 0) {
                doListDirName(path, arg2);
            } else if (strncmp(arg3, "name_starts_with=", 17) == 0) {
                doListDirName(path, arg3);
            }
            free(path);
        } else if (strncmp(arg2, "path=", 5) == 0) {
            char *path = getArgument(arg1);
            if (strncmp(arg1, "size_greater=", 13) == 0) {
                doListDirSize(path, arg1);
            } else if (strncmp(arg3, "size_greater=", 13) == 0) {
                doListDirSize(path, arg3);
            } else if (strncmp(arg1, "name_starts_with=", 17) == 0) {
                doListDirName(path, arg1);
            } else if (strncmp(arg3, "name_starts_with=", 17) == 0) {
                doListDirName(path, arg3);
            }
            free(path);
        } else if (strncmp(arg3, "path=", 5) == 0) {
            char *path = getArgument(arg3);
            if (strncmp(arg2, "size_greater=", 13) == 0) {
                doListDirSize(path, arg2);
            } else if (strncmp(arg1, "size_greater=", 13) == 0) {
                doListDirSize(path, arg1);
            } else if (strncmp(arg2, "name_starts_with=", 17) == 0) {
                doListDirName(path, arg2);
            } else if (strncmp(arg1, "name_starts_with=", 17) == 0) {
                doListDirName(path, arg1);
            }
            free(path);
        }
    }
}

void readInputListFilteringRecursive(char *arg1, char *arg2, char *arg3, char *arg4) {
    if (strncmp(arg1, "path=", 5) == 0) {
        char *path = getArgument(arg1);
        if (strncmp(arg2, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg2);
        } else if (strncmp(arg3, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg3);
        } else if (strncmp(arg4, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg4);
        } else if (strncmp(arg2, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg2);
        } else if (strncmp(arg3, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg3);
        } else if (strncmp(arg4, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg4);
        }
        free(path);
    } else if (strncmp(arg2, "path=", 5) == 0) {
        char *path = getArgument(arg1);
        if (strncmp(arg1, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg1);
        } else if (strncmp(arg3, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg3);
        } else if (strncmp(arg4, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg4);
        } else if (strncmp(arg1, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg1);
        } else if (strncmp(arg3, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg3);
        } else if (strncmp(arg4, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg4);
        }
        free(path);
    } else if (strncmp(arg3, "path=", 5) == 0) {
        char *path = getArgument(arg1);
        if (strncmp(arg2, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg2);
        } else if (strncmp(arg1, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg1);
        } else if (strncmp(arg4, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg4);
        } else if (strncmp(arg2, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg2);
        } else if (strncmp(arg1, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg1);
        } else if (strncmp(arg4, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg4);
        }
        free(path);
    } else if (strncmp(arg4, "path=", 5) == 0) {
        char *path = getArgument(arg1);
        if (strncmp(arg2, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg2);
        } else if (strncmp(arg3, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg3);
        } else if (strncmp(arg1, "size_greater=", 13) == 0) {
            doListDirRecursiveSize(path, arg1);
        } else if (strncmp(arg2, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg2);
        } else if (strncmp(arg3, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg3);
        } else if (strncmp(arg1, "name_starts_with=", 17) == 0) {
            doListDirRecursiveName(path, arg1);
        }
        free(path);
    }
}

int parse(char *path) {

    int fd = open(path, O_RDONLY);
    char *buf = (char *) malloc(sizeof(char) * MAX_PATH_LEN);

    read(fd, buf, 1);

    if (strcmp(buf, "o") != 0) {
        printf("ERROR\nwrong magic\n");
        free(buf);
        return -1;
    }

    lseek(fd, 2, SEEK_CUR);

    int version = 0;
    read(fd, &version, 1);
    if (version < 89 || version > 198) {
        printf("ERROR\nwrong version\n");
        free(buf);
        return -1;
    }

    int nrOfSections = 0;
    read(fd, &nrOfSections, 1);
    if (nrOfSections < 6 || nrOfSections > 16) {
        printf("ERROR\nwrong sect_nr\n");
        free(buf);
        return -1;
    }

    for (int i = 0; i < nrOfSections; i++) {
        lseek(fd, 9, SEEK_CUR);

        int sectType = 0;
        read(fd, &sectType, 1);
        if (sectType != 29 && sectType != 98 && sectType != 35 && sectType != 63 && sectType != 16 && sectType != 78) {
            printf("ERROR\nwrong sect_types\n");
            free(buf);
            return -1;
        }

        lseek(fd, 8, SEEK_CUR);
    }

    free(buf);
    close(fd);
    return 0;
}

void displayParseFields(char *path) {
    int fd = open(path, O_RDONLY);
    char *buf = (char *) malloc(sizeof(char) * MAX_PATH_LEN);

    printf("SUCCESS\n");

    /*
     *  SUCCESS
        version=<version_number>
        nr_sections=<no_of_sections>
        section1: <NAME_1> <TYPE_1> <SIZE_1>
        section2: <NAME_2> <TYPE_2> <SIZE_2>
    */

    lseek(fd, 3, SEEK_CUR);

    int version = 0;
    read(fd, &version, 1);
    printf("version=%d\n", version);

    int nrOfSect = 0;
    read(fd, &nrOfSect, 1);
    printf("nr_sections=%d\n", nrOfSect);

    for (int i = 0; i < nrOfSect; i++) {
        printf("section%d: ", i + 1);

        read(fd, buf, 9);
        printf("%s ", buf);

        int type = 0;
        read(fd, &type, 1);
        printf("%d ", type);

        lseek(fd, 4, SEEK_CUR);

        long int size = 0;
        read(fd, &size, 4);
        printf("%ld\n", size);
    }

    free(buf);
    close(fd);
}

void extract(char *path, int section, long line) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("ERROR\ninvalid file\n");
        return;
    }

    // read the number of sections
    lseek(fd, 4, SEEK_CUR);
    int nrOfSect = 0;
    read(fd, &nrOfSect, 1);

    if (nrOfSect < section) {
        printf("ERROR\ninvalid section\n");
        return;
    }

    // jump to the header of the section we want
    lseek(fd, (section - 1) * 18, SEEK_CUR);

    // now that we are at the header of the section of interest, read the SECT_OFFSET
    lseek(fd, 10, SEEK_CUR);

    // read the section's offset
    int offset = 0;
    read(fd, &offset, 4);
    // read the section's size
    int size = 0;
    read(fd, &size, 4);

    //jump to the end of the wanted section
    lseek(fd, offset + size, SEEK_SET);

    // read the file backwards until we reach the beginning of our desired line, and count the number of end-of-line characters
    long currentLine = 1;
    while (currentLine < line) {
        lseek(fd, -1, SEEK_CUR);
        char c = 0;
        read(fd, &c, sizeof(c));
        if (c == 0x0a) {
            lseek(fd, -2, SEEK_CUR);
            char c2 = 0;
            read(fd, &c2, sizeof(c2));
            if (c2 == 0x0d) {
                currentLine++;
                lseek(fd, -1, SEEK_CUR);
                continue;
            }
        }
        // check if we haven't reached the end of the section
        if (c == 0x00) {
            lseek(fd, -2, SEEK_CUR);
            char c2 = 0;
            read(fd, &c2, sizeof(c2));
            if (c2 == 0x00) {
                printf("ERROR\ninvalid section\n");
                return;
            }
        }
        lseek(fd, -1, SEEK_CUR);
    }

    if (currentLine == line) {
        printf("SUCCESS\n");
        while (1) {
            lseek(fd, -1, SEEK_CUR);
            char c = 0;
            read(fd, &c, sizeof(c));
            if (c == 0x0a) {
                lseek(fd, -2, SEEK_CUR);
                char c2 = 0;
                read(fd, &c2, sizeof(c2));
                if (c2 == 0x0d) {
                    //reached eol
                    printf("\n");
                    break;
                }
            }
            if (c == 0x00) {
                lseek(fd, -2, SEEK_CUR);
                char c2 = 0;
                read(fd, &c2, sizeof(c2));
                if (c2 == 0x00) {
                    printf("\n");
                    break;
                }
            }
            printf("%c", c);
            lseek(fd, -1, SEEK_CUR);
        }
    }
}

int findSection(char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("ERROR\ninvalid directory path\n");
        return -1;
    }

    // read the number of sections
    lseek(fd, 4, SEEK_CUR);
    int nrOfSect = 0;
    read(fd, &nrOfSect, 1);

    for (int i = 0; i < nrOfSect; i++) {
        lseek(fd, 5 + 18 * i + 10, SEEK_SET);
        int offset = 0;
        read(fd, &offset, 4);
        int size = 0;
        read(fd, &size, 4);

        // jump to the start of the current section
        lseek(fd, offset, SEEK_SET);

        int readBytes = 0;
        int nrOfLines = 1;
        while (readBytes <= size) {
            char c = 0;
            read(fd, &c, sizeof(c));
            readBytes++;
            if (c == 0x0d) {
                char c2 = 0;
                read(fd, &c2, sizeof(c2));
                readBytes++;
                if (c2 == 0x0a) {
                    nrOfLines++;
                    continue;
                }
            }
        }

        if (nrOfLines >= 16) {
            return 0;
        }
    }
    return 0;
}

void findall(const char *name)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            findall(path);
        } else {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            if(findSection(path) == 0) {
                printf("%s\n",path);
            }
        }
    }
    closedir(dir);
}


int main(int argc, char **argv) {
    if (argc == 2) {
        if (strcmp(argv[1], "variant") == 0) {
            printf("12170\n");
        }
    } else if (argc == 3) {
        if (strcmp(argv[1], "list") == 0 || strcmp(argv[2], "list") == 0) {
            readInput2(argv[1], argv[2]);
        }
        if (strcmp(argv[1], "parse") == 0) {
            char *pch;
            char *path = (char *) malloc(sizeof(char) * MAX_PATH_LEN);
            strtok(argv[2], "=");
            pch = strtok(NULL, "=");
            strcpy(path, pch);

            int success = parse(path);
            if (success == 0) {
                displayParseFields(path);
            }

            free(path);
        } else if (strcmp(argv[2], "parse") == 0) {
            char *pch;
            char *path = (char *) malloc(sizeof(char) * MAX_PATH_LEN);
            pch = strtok(argv[1], "=");
            strcpy(path, pch);

            int success = parse(path);
            if (success == 0) {
                displayParseFields(path);
            }

            free(path);
        } else if (strcmp(argv[1], "findall") == 0) {
            if (strncmp(argv[2], "path=", 5) == 0) {
                char *path = getArgument(argv[2]);
                printf("SUCCESS\n");
                findall(path);
                free(path);
            }
        }

    } else if (argc == 4) {
        if (strcmp(argv[1], "list") == 0 || strcmp(argv[2], "list") == 0 || strcmp(argv[3], "list") == 0) {
            if (strcmp(argv[1], "recursive") == 0 || strcmp(argv[2], "recursive") == 0 ||
                strcmp(argv[3], "recursive") == 0) {
                readInputListRecursive(argv[1], argv[2], argv[3]);
            } else {
                readInputListFiltering(argv[1], argv[2], argv[3]);
            }
        }
    } else if (argc == 5) {
        if (strcmp(argv[1], "list") == 0 || strcmp(argv[2], "list") == 0 || strcmp(argv[3], "list") == 0 ||
            strcmp(argv[4], "list") == 0) {
            if (strcmp(argv[1], "recursive") == 0 || strcmp(argv[2], "recursive") == 0 ||
                strcmp(argv[3], "recursive") == 0 || strcmp(argv[4], "recursive") == 0) {
                readInputListFilteringRecursive(argv[1], argv[2], argv[3], argv[4]);
            }
        } else if (strcmp(argv[1], "extract") == 0) {
            char *path = getArgument(argv[2]);
            char *sectionArg = getArgument(argv[3]);
            int section = atoi(sectionArg);
            char *lineArg = getArgument(argv[4]);
            long line = atol(lineArg);
            extract(path, section, line);
            free(lineArg);
            free(sectionArg);
            free(path);
        }
    }

    return 0;
}
