#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#ifndef ELF_PARSER_H
#define ELF_PARSER_H

typedef struct {
    Elf64_Dyn* entries;
    char* str;
    int entries_count;
} Elf64_Dyn_Ctx;

typedef struct {
    Elf64_Ehdr ehdr;
    Elf64_Phdr* phdr;
    Elf64_Shdr* shdr;
} Elf64_hdrs;

typedef struct {
    char* libname;
    char* arch;
} Lib_Info;

typedef struct {
    char* filepath;
    int has_lib;
    int count;
    char* arch;
    int is_statical;
} Execfile_Info;

Elf64_hdrs* get_elf_headers(FILE* file);
Elf64_Dyn_Ctx* get_dyn_ctx(Elf64_Ehdr ehdr, Elf64_Shdr* shdr, FILE* file);
int is_exec(Elf64_Ehdr ehdr, Elf64_Phdr* phdr);
Lib_Info* get_lib_info(const char* libpath);
Execfile_Info* get_execfile_info(char* filename, char* libname);
void free_elf_headers(Elf64_hdrs* hdrs);
void free_dyn_ctx(Elf64_Dyn_Ctx* dyn_ctx);

#endif