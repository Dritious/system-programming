#ifndef FILE_READER_H
#define FILE_READER_H

#include "include/elf_parser.h"

typedef struct Node {
    Execfile_Info* info;
    struct Node* next;
} Node;

Node* scan_directory(const char* dir_path, char* target_lib, int* out_count);

#endif