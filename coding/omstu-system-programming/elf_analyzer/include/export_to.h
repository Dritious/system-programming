#include "include/elf_parser.h"
#include "include/file_reader.h"
#ifndef EXPORT_TO_H
#define EXPORT_TO_H

int export_to_txt(Node* node, int count, Lib_Info* info, char* dirpath);
int export_to_json(Node* node, int count, Lib_Info* info, char* dirpath);
int export_to_html(Node* node, int count, Lib_Info* info, char* dirpath);

#endif