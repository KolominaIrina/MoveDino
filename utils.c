#include "utils.h"
#include <string.h>

int file_exists(const char* filename) {
    if (filename == NULL) return 0;
    
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

char* read_line(FILE* file, char* buffer, int size) {
    if (file == NULL || buffer == NULL || size <= 0) {
        return NULL;
    }
    
    if (fgets(buffer, size, file) != NULL) {
        // Убираем символ новой строки
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        return buffer;
    }
    
    return NULL;
}