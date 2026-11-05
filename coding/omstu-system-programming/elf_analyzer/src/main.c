#include "include/file_reader.h"
#include "include/export_to.h"
#include <string.h>
#include <getopt.h>

int print_help() {
    printf("Использование: bldd [ОПЦИИ] <путь_к_библиотеке> <каталог_сканирования>\n");
    printf("Обратный ldd: поиск ELF-файлов, зависящих от указанной разделяемой библиотеки.\n\n");
    printf("Опции:\n");
    printf("  -h, --help               Показать эту справку\n");
    printf("  -e, --export-to FORMAT   Формат отчета (txt, json, html). По умолчанию txt.\n\n");
    printf("Примеры:\n");
    printf("  bldd /lib/x86_64-linux-gnu/libc.so.6 /usr/bin\n");
    printf("  bldd -e html /usr/lib/libm.so /opt/software\n");
    return 0;
}

int main(int argc, char *argv[]){
    int c;
    char *export_to = "txt"; 

    struct option long_options[] = {
        {"help",      no_argument,       NULL, 'h'},
        {"export-to", required_argument, NULL, 'e'},
        {NULL,        0,                 NULL,  0 }
    };

    while ((c = getopt_long(argc, argv, "he:", long_options, NULL)) != -1) {
        switch (c) {
            case 'h':
                print_help();
                return 0; 
            case 'e':
                export_to = optarg; 
                break;
            case '?':
                return 1;
            default:
                abort();
        }
    }

    int remaining_args = argc - optind;

    if (remaining_args != 2) {
        fprintf(stderr, "Ошибка: Неверное количество обязательных аргументов (%d вместо 2).\n", remaining_args);
        print_help();
        return 1;
    }

    const char *libpath = argv[optind];
    const char *dir_path = argv[optind + 1];

    Lib_Info* lib_info = get_lib_info(libpath);
    int count = 0;
    Node* node = scan_directory(dir_path, lib_info->libname, &count);
    if (node == NULL) {
        return 0;
    }
    if (strcmp(export_to, "txt")==0){
        export_to_txt(node, count, lib_info, dir_path);
    }
    else if (strcmp(export_to, "json")==0){
        export_to_json(node, count, lib_info, dir_path);
    }
    else if (strcmp(export_to, "html")==0){
        export_to_html(node, count, lib_info, dir_path);
    }
    else {
        printf("Invalid argument for --export-to: %s", export_to);
        return 1;
    }
    return 0;
}

