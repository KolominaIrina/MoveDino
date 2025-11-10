#ifndef PARSER_H
#define PARSER_H

#include "field.h"
#include "commands.h"

#define MAX_LINE_LENGTH 256
#define MAX_FILENAME_LENGTH 100

// Структура для представления команды
typedef struct {
    CommandType type;
    char direction[10];
    int x, y, n;
    char color;
    char filename[MAX_FILENAME_LENGTH];
} ParsedCommand;

// Функции парсера
int parse_line(const char* line, ParsedCommand* cmd);
int is_comment_line(const char* line);
void trim_whitespace(char* str);

#endif