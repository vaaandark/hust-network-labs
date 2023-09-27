#ifndef FILE_H_
#define FILE_H_

#include <stdlib.h>

typedef struct {
    size_t buff_size;
    size_t content_size;
    char buff[0];
} LoadedFile;

LoadedFile *loaded_file_new(const char *filename);

void loaded_file_drop(LoadedFile *loaded_file);

#endif
