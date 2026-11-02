#include <dirent.h>
#include <string.h>
#include "include/file_reader.h"


Node* scan_directory(const char* dir_path, char* target_lib, int* out_count) {
    DIR* dir = opendir(dir_path);
    if (!dir) return NULL; // Защита, если директория не открылась

    struct dirent *entry;
    Node* start_node = NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Выделяем память под путь
        size_t path_len = strlen(dir_path) + 1 + strlen(entry->d_name) + 1;
        char* new_path = malloc(path_len);
        if (new_path == NULL) {
            closedir(dir);
            return start_node;
        }
        snprintf(new_path, path_len, "%s/%s", dir_path, entry->d_name);

        Node* sub_list = NULL;

        if (entry->d_type == DT_DIR) {
            // Рекурсивно сканируем поддиректорию и получаем ее список нод
            sub_list = scan_directory(new_path, target_lib, out_count);
            free(new_path);
        } 
        else if (entry->d_type == DT_REG) {
            Execfile_Info* info = get_execfile_info(new_path, target_lib);
            //printf("%s\n", info->filepath);
            free(new_path);
            
            if (info == NULL) {
                continue; 
            }

            Node* node = malloc(sizeof(Node));
            if (node == NULL) {
                continue; 
            }
            node->info = info;
            node->next = NULL;
            (*out_count)++;
            
            sub_list = node; // Обернули одну ноду в список для последующей вставки
        } else {
            free(new_path);
            continue;
        }

        while (sub_list != NULL) {
            Node* next_sub = sub_list->next; // Запоминаем следующий элемент подсписка
            sub_list->next = NULL;

            // Если текущий список пуст или элемент sub_list должен встать в самое начало
            if (start_node == NULL || start_node->info->count <= sub_list->info->count) {
                sub_list->next = start_node;
                start_node = sub_list;
            } else {
                // Ищем место для вставки внутри текущего списка
                Node* curr_node = start_node;
                while (curr_node->next != NULL && curr_node->next->info->count > sub_list->info->count) {
                    curr_node = curr_node->next;
                }
                sub_list->next = curr_node->next;
                curr_node->next = sub_list;
            }

            sub_list = next_sub; // Переходим к следующей ноде из поддиректории
        }
    }

    closedir(dir);
    return start_node;
}
