#ifndef PARSER_H
#define PARSER_H

#include "field.h"
#include "commands.h"

#define MAX_LINE_LENGTH 256
#define MAX_FILENAME_LENGTH 100

// Структура для представления разобранной команды
typedef struct {
    CommandType type;               // Тип команды
    char direction[10];             // Направление (для MOVE, DIG, JUMP и т.д.)
    int x, y, n;                    // Координаты и числовые параметры
    char color;                     // Цвет для покраски
    char filename[MAX_FILENAME_LENGTH]; // Имя файла для EXEC/LOAD
    char then_command[MAX_LINE_LENGTH]; // Команда после THEN для IF
} ParsedCommand;

// Прототипы функций парсера
int parse_line(const char* line, ParsedCommand* cmd);
int is_comment_line(const char* line);
void trim_whitespace(char* str);

#endif