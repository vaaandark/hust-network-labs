#include <stdio.h>
#include <sys/stat.h>
#include "file.h"
#include "def.h"

LoadedFile *loaded_file_new(const char *filename) {
    struct stat filestat;
    if (stat(filename, &filestat) == -1) {
        INFO("No such file: %s\n", filename);
        return NULL;
    }
    if (!(filestat.st_mode & S_IFREG)) {
        INFO("Not a regular file: %s\n", filename);
        return NULL;
    }
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        INFO("Cannot open file: %s\n", filename);
        return NULL;
    }
    size_t remaining = filestat.st_size;
    INFO("Content size is %lu\n", remaining);
    fflush(stderr);
    LoadedFile *loaded_file = (LoadedFile *)malloc(sizeof(LoadedFile) + remaining);
    loaded_file->buff_size = remaining;
    char *p = loaded_file->buff;
    int read;
    size_t content_size = 0;
    while ((read = fread(p, 1, remaining, fp)) != 0 && remaining > 0) {
        if (read == -1) {
            free(loaded_file);
            INFO("Unknown error\n");
            return NULL;
        }
        p += read;
        remaining -= read;
        content_size += read;
    }
    loaded_file->content_size = content_size;
    return loaded_file;
}

void loaded_file_drop(LoadedFile *loaded_file) {
    free(loaded_file);
}
