#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

// Утилиты для работы с файлами и строками
int file_exists(const char* filename);
char* read_line(FILE* file, char* buffer, int size);

#endif