#include <dirent.h>
#include <string.h>
#include "include/file_reader.h"


Node* scan_directory(const char* dir_path, char* target_lib, int* out_count) {
    DIR* dir = opendir(dir_path);
    if (!dir) return NULL;

    struct dirent *entry;
    Node* start_node = NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        size_t path_len = strlen(dir_path) + 1 + strlen(entry->d_name) + 1;
        char* new_path = malloc(path_len);
        if (new_path == NULL) {
            closedir(dir);
            return start_node;
        }

        if (dir_path[strlen(dir_path) - 1] == '/') {
            snprintf(new_path, path_len, "%s%s", dir_path, entry->d_name);
        } else {
            snprintf(new_path, path_len, "%s/%s", dir_path, entry->d_name);
        }

        Node* sub_list = NULL;

        if (entry->d_type == DT_DIR) {
            sub_list = scan_directory(new_path, target_lib, out_count);
            
        } 
        else if (entry->d_type == DT_REG) {
            Execfile_Info* info = get_execfile_info(new_path, target_lib);
            
            if (info != NULL) {
                Node* node = malloc(sizeof(Node));
                if (node != NULL) {
                    node->info = info;
                    node->next = NULL;
                    (*out_count)++;
                    sub_list = node; 
                } else {
                    free(info->filepath);
                    free(info);
                }
            }

        }
        
        free(new_path);
        while (sub_list != NULL) {
            Node* next_sub = sub_list->next; 
            sub_list->next = NULL;

            if (start_node == NULL || start_node->info->count <= sub_list->info->count) {
                sub_list->next = start_node;
                start_node = sub_list;
            } else {
                Node* curr_node = start_node;
                while (curr_node->next != NULL && curr_node->next->info->count > sub_list->info->count) {
                    curr_node = curr_node->next;
                }
                sub_list->next = curr_node->next;
                curr_node->next = sub_list;
            }

            sub_list = next_sub;
        }
    }

    closedir(dir);
    return start_node;
}
