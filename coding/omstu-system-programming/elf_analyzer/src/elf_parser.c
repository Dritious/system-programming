#include <string.h>
#include "include/elf_parser.h"

Elf64_Dyn_Ctx* get_dyn_ctx(Elf64_Ehdr ehdr, Elf64_Shdr* shdr, FILE* file){
    Elf64_Shdr *dyn_shdr = NULL;
    Elf64_Shdr *dynstr_shdr = NULL;

    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (shdr[i].sh_type == SHT_DYNAMIC){
            dyn_shdr = &shdr[i];
        }
    }

    if (dyn_shdr) {
        dynstr_shdr = &shdr[dyn_shdr->sh_link];
    }

    if (!dyn_shdr || !dynstr_shdr) {
        return NULL;
    }

    Elf64_Dyn_Ctx* dyn_ctx = malloc(sizeof(Elf64_Dyn_Ctx));

    char *dynstr = malloc(dynstr_shdr->sh_size);
    fseek(file, dynstr_shdr->sh_offset, SEEK_SET);
    fread(dynstr, dynstr_shdr->sh_size, 1, file);
    dyn_ctx->str = dynstr;

    int dyn_entries_count = dyn_shdr->sh_size / sizeof(Elf64_Dyn);
    dyn_ctx->entries_count = dyn_entries_count;

    Elf64_Dyn *dyn_entries = malloc(dyn_shdr->sh_size);
    fseek(file, dyn_shdr->sh_offset, SEEK_SET);
    fread(dyn_entries, sizeof(Elf64_Dyn), dyn_entries_count, file);
    dyn_ctx->entries = dyn_entries;

    return dyn_ctx;
}

Elf64_Phdr* get_phdr(Elf64_Ehdr ehdr, FILE* file){
   Elf64_Phdr* phdr = malloc(ehdr.e_phentsize * ehdr.e_phnum);
   if (!phdr) return NULL;

   fseek(file, ehdr.e_phoff, SEEK_SET);
   if (fread(phdr, ehdr.e_phentsize, ehdr.e_phnum, file) != ehdr.e_phnum){
    fprintf(stderr, "Error while reading Section Header\n");
    free(phdr);
    return NULL;
   } 
   return phdr;
}

Elf64_Shdr* get_shdr(Elf64_Ehdr ehdr, FILE* file){
   Elf64_Shdr* shdr = malloc(ehdr.e_shentsize * ehdr.e_shnum);
   if (!shdr) return NULL;

   fseek(file, ehdr.e_shoff, SEEK_SET);
   if (fread(shdr, ehdr.e_shentsize, ehdr.e_shnum, file) != ehdr.e_shnum){
    fprintf(stderr, "Error while reading Section Header\n");
    free(shdr);
    return NULL;
   } 
   return shdr;
}

Elf64_hdrs* get_elf_headers(FILE* file){
    Elf64_Ehdr ehdr;
    if (fread(&ehdr, sizeof(Elf64_Ehdr), 1, file) != 1){
        //fprintf(stderr, "Error while reading ELF-header\n");
        return NULL;
    }

    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 || ehdr.e_ident[EI_MAG3] != ELFMAG3){

        //fprintf(stderr, "Error while reading ELF magic numbers\n");
        return NULL;
    }

    Elf64_hdrs* hdrs = malloc(sizeof(Elf64_hdrs));
    hdrs->ehdr = ehdr;

    Elf64_Phdr *phdr = get_phdr(ehdr, file);
    if (phdr == NULL){
        free(hdrs);
        return NULL;
    }
    hdrs->phdr = phdr;

    Elf64_Shdr *shdr = get_shdr(ehdr, file);
    if (shdr == NULL){
        free(hdrs->phdr);
        free(hdrs);
        return NULL;
    }
    hdrs->shdr = shdr;
    return hdrs;
}

int is_exec(Elf64_Ehdr ehdr, Elf64_Phdr* phdr){
    if (ehdr.e_type==ET_EXEC){
        return 1;
    } 
    else if (ehdr.e_type==ET_DYN){
        for (int i = 0; i<ehdr.e_phnum; i++){
            if (phdr[i].p_type == PT_INTERP){
                return 1;
            }
        }
        return 0;
    }
    else {
        return 0;
    }
}


char* get_filename(const char* path) {
    if (path == NULL) return NULL;

    char *slash = strrchr(path, '/');

    if (slash != NULL) {
        return slash + 1; 
    }

    return path;
}

char* get_arch(uint16_t e_machine){
    switch (e_machine) {
        case EM_386:
            return "Intel x86 (32-bit)";
            break;
        case EM_ARM:
            return "ARM (32-bit)";
            break;
        case EM_X86_64:
            return "AMD64 / Intel 64 (x86_64)";
            break;
        case EM_AARCH64:
            return "ARM 64-bit (AArch64)";
            break;
        default:
            return "Undefinded";
    }
}

Execfile_Info* get_execfile_info(char* filename, char* libname){
    FILE *file = fopen(filename, "rb");
    if (!file){
        perror("No such file");
        return NULL;
    }

    Elf64_hdrs* hdrs = get_elf_headers(file); 
    if (hdrs == NULL){
        fclose(file);
        return NULL;
    }
    if (!is_exec(hdrs->ehdr, hdrs->phdr)) {
        fclose(file);
        return NULL;
    }
    Elf64_Dyn_Ctx *dyn_ctx = get_dyn_ctx(hdrs->ehdr, hdrs->shdr, file);
    if (dyn_ctx == NULL) {
        free(hdrs->shdr);
        free(hdrs->phdr);
        free(hdrs);
        fclose(file);
        return NULL;
    }

    Execfile_Info* info = malloc(sizeof(Execfile_Info));
    info->filepath = malloc(strlen(filename)+1);
    strcpy(info->filepath, filename);
    info->count = 0;
    info->has_lib = 0;
    info->arch = get_arch(hdrs->ehdr.e_machine);
    for (int i = 0; i < dyn_ctx->entries_count; i++){
        if (dyn_ctx->entries[i].d_tag == DT_NEEDED){
            Elf64_Xword string_off = dyn_ctx->entries[i].d_un.d_val;
            if (strcmp(&dyn_ctx->str[string_off], libname)==0){
                info->has_lib = 1;
            }
            info->count++;
        }

        if (dyn_ctx->entries[i].d_tag == DT_NULL){
            break;
        }
    }

    free(dyn_ctx->entries);
    free(dyn_ctx->str);
    free(dyn_ctx);
    free(hdrs->shdr);
    free(hdrs->phdr);
    free(hdrs);
    fclose(file);

    return info;
}

Lib_Info* get_lib_info(const char* libpath){
    Lib_Info* info = malloc(sizeof(info));
    FILE *file = fopen(libpath, "rb");
    if (!file){
        perror("No such file");
        return NULL;
    }
    
    Elf64_hdrs* hdrs = get_elf_headers(file); 
    if (hdrs == NULL){
        return NULL;
    }

    info->libname = get_filename(libpath);
    info->arch = get_arch(hdrs->ehdr.e_machine);

    return info;
}
