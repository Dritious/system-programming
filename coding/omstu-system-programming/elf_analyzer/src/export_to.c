#include "include/export_to.h"
#include <time.h>
#include <json-c/json.h>

int export_to_txt(Node* node, int count, Lib_Info* info, char* dirpath){
    FILE* file = fopen("report.txt", "w");
    time_t create_time = time(NULL);
    char* create_time_str = ctime(&create_time);
    fprintf(file, "Директория: %s\n", dirpath);
    fprintf(file, "Библиотека: %s , архитектура: %s\n", info->libname, info->arch);
    fprintf(file, "Дата: %s\n", create_time_str);
    for (int i = 0; i < count; i++){
        fprintf(file, "%s, %d, %d, %s\n", node->info->filepath, node->info->count, 
            node->info->has_lib, node->info->arch);
        node = node->next;
    }
    fclose(file);
    return 0;
}
int export_to_json(Node* node, int count, Lib_Info* info, char* dirpath) {
    struct json_object * root = json_object_new_object();
    if (!root) return 1;
    time_t  create_time = time(NULL);
    char* create_time_str = ctime(&create_time);
    json_object_object_add(root, "created_date", json_object_new_string(create_time_str));
    json_object_object_add(root, "lib_name", json_object_new_string(info->libname));
    json_object_object_add(root, "lib_arch", json_object_new_string(info->arch));
    for (int i = 0; i<count; i++){
        struct json_object * json_node = json_object_new_object();
        json_object_object_add(json_node, "count", json_object_new_int(node->info->count));
        json_object_object_add(json_node, "file_arch", json_object_new_string(node->info->arch));
        json_object_object_add(root, node->info->filepath, json_node);
        node = node->next;
    }
    const char* output = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    if (output==NULL) return 1;
    FILE* file = fopen("report.json", "w");
    fprintf(file, "%s\n", output);
    printf("%s\n", output);
    json_object_put(root);
    fclose(file);
    return 0;
}

int export_to_html(Node* node, int count, Lib_Info* info, char* dirpath) {
    FILE* file = fopen("report.html", "w");
    if (file == NULL) {
        return 1; 
    }

    time_t create_time = time(NULL);
    char* create_time_str = ctime(&create_time);

    fprintf(file, "<!DOCTYPE html>\n<html lang=\"ru\">\n<head>\n");
    fprintf(file, "    <meta charset=\"UTF-8\">\n");
    fprintf(file, "    <title>Отчет по библиотеке</title>\n");
    fprintf(file, "    <style>\n");
    fprintf(file, "        body { font-family: sans-serif; margin: 20px; line-height: 1.6; }\n");
    fprintf(file, "        .meta { margin-bottom: 20px; background: #f4f4f9; padding: 15px; border-left: 5px solid #007BFF; }\n");
    fprintf(file, "        table { width: 100%%; border-collapse: collapse; margin-top: 10px; }\n");
    fprintf(file, "        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n");
    fprintf(file, "        th { background-color: #007BFF; color: white; }\n");
    fprintf(file, "        tr:nth-child(even) { background-color: #f9f9f9; }\n");
    fprintf(file, "    </style>\n</head>\n<body>\n");

    fprintf(file, "    <h1>Отчет о сканировании</h1>\n");

    fprintf(file, "    <div class=\"meta\">\n");
    fprintf(file, "        <p><strong>Директория:</strong> %s</p>\n", dirpath);
    fprintf(file, "        <p><strong>Библиотека:</strong> %s , <strong>архитектура:</strong> %s</p>\n", info->libname, info->arch);
    fprintf(file, "        <p><strong>Дата:</strong> %s</p>\n", create_time_str);
    fprintf(file, "    </div>\n");

    fprintf(file, "    <table>\n");
    fprintf(file, "        <thead>\n");
    fprintf(file, "            <tr>\n");
    fprintf(file, "                <th>Путь к файлу</th>\n");
    fprintf(file, "                <th>Количество</th>\n");
    fprintf(file, "                <th>Наличие либы</th>\n");
    fprintf(file, "                <th>Архитектура</th>\n");
    fprintf(file, "            </tr>\n");
    fprintf(file, "        </thead>\n");
    fprintf(file, "        <tbody>\n");

    for (int i = 0; i < count; i++) {
        if (node == NULL) break;
        
        fprintf(file, "            <tr>\n");
        fprintf(file, "                <td>%s</td>\n", node->info->filepath);
        fprintf(file, "                <td>%d</td>\n", node->info->count);
        fprintf(file, "                <td>%d</td>\n", node->info->has_lib);
        fprintf(file, "                <td>%s</td>\n", node->info->arch);
        fprintf(file, "            </tr>\n");
        
        node = node->next;
    }

    fprintf(file, "        </tbody>\n");
    fprintf(file, "    </table>\n");
    fprintf(file, "</body>\n</html>\n");

    fclose(file);
    return 0;
}
