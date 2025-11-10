#include "utils.h"
#include <string.h>

// Проверка существования файла
int file_exists(const char* filename) {
    if (filename == NULL) return 0;
    
    // Пытаемся открыть файл для чтения
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return 1;  // Файл существует
    }
    return 0;  // Файл не существует
}

// Чтение строки из файла
char* read_line(FILE* file, char* buffer, int size) {
    if (file == NULL || buffer == NULL || size <= 0) {
        return NULL;
    }
    
    // Читаем строку из файла
    if (fgets(buffer, size, file) != NULL) {
        // Убираем символ новой строки в конце
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        return buffer;
    }
    
    return NULL;  // Достигнут конец файла или ошибка чтения
}