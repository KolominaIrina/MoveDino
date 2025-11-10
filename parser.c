#include "parser.h"
#include <ctype.h>
#include <string.h>

int is_comment_line(const char* line) {
    // Пропускаем начальные пробелы
    while (*line && isspace(*line)) line++;
    return (line[0] == '/' && line[1] == '/');
}

void trim_whitespace(char* str) {
    if (str == NULL) return;
    
    char* end;
    
    // Убираем пробелы в конце
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    
    // Убираем пробелы в начале
    char* start = str;
    while (*start && isspace(*start)) start++;
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int parse_line(const char* line, ParsedCommand* cmd) {
    if (line == NULL || cmd == NULL) return -1;
    
    char line_copy[MAX_LINE_LENGTH];
    strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
    line_copy[MAX_LINE_LENGTH - 1] = '\0';
    
    trim_whitespace(line_copy);
    
    // Пропускаем пустые строки и комментарии
    if (line_copy[0] == '\0' || is_comment_line(line_copy)) {
        cmd->type = CMD_COMMENT;
        return 0;
    }
    
    // Разбиваем строку на токены
    char* tokens[10];
    int token_count = 0;
    char* token = strtok(line_copy, " ");
    
    while (token != NULL && token_count < 10) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }
    
    if (token_count == 0) return -1;
    
    // Анализируем команду
    cmd->type = CMD_UNKNOWN;
    
    if (strcasecmp(tokens[0], "SIZE") == 0) {
        if (token_count != 3) return -2;
        cmd->type = CMD_SIZE;
        cmd->x = atoi(tokens[1]);
        cmd->y = atoi(tokens[2]);
    }
    else if (strcasecmp(tokens[0], "START") == 0) {
        if (token_count != 3) return -2;
        cmd->type = CMD_START;
        cmd->x = atoi(tokens[1]);
        cmd->y = atoi(tokens[2]);
    }
    else if (strcasecmp(tokens[0], "MOVE") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_MOVE;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "PAINT") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_PAINT;
        cmd->color = tokens[1][0];
    }
    else if (strcasecmp(tokens[0], "DIG") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_DIG;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "MOUND") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_MOUND;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "JUMP") == 0) {
        if (token_count != 3) return -2;
        cmd->type = CMD_JUMP;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
        cmd->n = atoi(tokens[2]);
    }
    else if (strcasecmp(tokens[0], "GROW") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_GROW;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "CUT") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_CUT;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "MAKE") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_MAKE;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "PUSH") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_PUSH;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "EXEC") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_EXEC;
        strncpy(cmd->filename, tokens[1], sizeof(cmd->filename) - 1);
    }
    else if (strcasecmp(tokens[0], "LOAD") == 0) {
        if (token_count != 2) return -2;
        cmd->type = CMD_LOAD;
        strncpy(cmd->filename, tokens[1], sizeof(cmd->filename) - 1);
    }
    else if (strcasecmp(tokens[0], "UNDO") == 0) {
        if (token_count != 1) return -2;
        cmd->type = CMD_UNDO;
    }
    else if (strcasecmp(tokens[0], "IF") == 0) {
        // Упрощенная реализация IF (полная реализация требует больше кода)
        if (token_count < 8) return -2;
        cmd->type = CMD_IF;
        // Здесь должна быть более сложная логика разбора
    }
    
    return (cmd->type != CMD_UNKNOWN) ? 0 : -1;
}