#include "parser.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>

// Проверка, является ли строка комментарием
int is_comment_line(const char* line) {
    // Пропускаем начальные пробелы
    while (*line && isspace(*line)) line++;
    // Комментарий начинается с //
    return (line[0] == '/' && line[1] == '/');
}

// Удаление пробелов в начале и конце строки
void trim_whitespace(char* str) {
    if (str == NULL) return;
    
    char* end;
    
    // Убираем пробелы в конце строки
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
    
    // Убираем пробелы в начале строки
    char* start = str;
    while (*start && isspace(*start)) start++;
    
    // Сдвигаем строку, если были пробелы в начале
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

// Основная функция разбора строки команды
int parse_line(const char* line, ParsedCommand* cmd) {
    if (line == NULL || cmd == NULL) {
        printf("Error: Null pointer when parsing command\n");
        return -1;
    }
    
    // Создаем копию строки для безопасной работы
    char line_copy[MAX_LINE_LENGTH];
    strncpy(line_copy, line, MAX_LINE_LENGTH - 1);
    line_copy[MAX_LINE_LENGTH - 1] = '\0';
    
    // Удаляем лишние пробелы
    trim_whitespace(line_copy);
    
    // Пропускаем пустые строки и комментарии
    if (line_copy[0] == '\0' || is_comment_line(line_copy)) {
        cmd->type = CMD_COMMENT;
        return 0;
    }
    
    // Разбиваем строку на токены (слова)
    char* tokens[20];  // Увеличиваем для IF команды
    int token_count = 0;
    char* token = strtok(line_copy, " ");
    
    while (token != NULL && token_count < 20) {
        tokens[token_count++] = token;
        token = strtok(NULL, " ");
    }
    
    if (token_count == 0) {
        printf("Error: Empty command\n");
        return -1;
    }
    
    // Инициализируем тип команды как неизвестный
    cmd->type = CMD_UNKNOWN;
    
    // Анализируем команду по первому токену
    if (strcasecmp(tokens[0], "SIZE") == 0) {
        if (token_count != 3) {
            printf("Syntax Error: SIZE requires 2 arguments (width height)\n");
            return -2;
        }
        cmd->type = CMD_SIZE;
        cmd->x = atoi(tokens[1]);
        cmd->y = atoi(tokens[2]);
    }
    else if (strcasecmp(tokens[0], "START") == 0) {
        if (token_count != 3) {
            printf("Syntax Error: START requires 2 arguments (x y)\n");
            return -2;
        }
        cmd->type = CMD_START;
        cmd->x = atoi(tokens[1]);
        cmd->y = atoi(tokens[2]);
    }
    else if (strcasecmp(tokens[0], "MOVE") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: MOVE requires 1 argument (direction)\n");
            return -2;
        }
        cmd->type = CMD_MOVE;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "PAINT") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: PAINT requires 1 argument (color)\n");
            return -2;
        }
        cmd->type = CMD_PAINT;
        cmd->color = tokens[1][0];
    }
    else if (strcasecmp(tokens[0], "DIG") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: DIG requires 1 argument (direction)\n");
            return -2;
        }
        cmd->type = CMD_DIG;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "MOUND") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: MOUND requires 1 argument (direction)\n");
            return -2;
        }
        cmd->type = CMD_MOUND;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "JUMP") == 0) {
        if (token_count != 3) {
            printf("Syntax Error: JUMP requires 2 arguments (direction distance)\n");
            return -2;
        }
        cmd->type = CMD_JUMP;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
        cmd->n = atoi(tokens[2]);
    }
    else if (strcasecmp(tokens[0], "GROW") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: GROW requires 1 argument (direction)\n");
            return -2;
        }
        cmd->type = CMD_GROW;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "CUT") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: CUT requires 1 argument (direction)\n");
            return -2;
        }
        cmd->type = CMD_CUT;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "MAKE") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: MAKE requires 1 argument (direction)\n");
            return -2;
        }
        cmd->type = CMD_MAKE;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "PUSH") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: PUSH requires 1 argument (direction)\n");
            return -2;
        }
        cmd->type = CMD_PUSH;
        strncpy(cmd->direction, tokens[1], sizeof(cmd->direction) - 1);
    }
    else if (strcasecmp(tokens[0], "EXEC") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: EXEC requires 1 argument (filename)\n");
            return -2;
        }
        cmd->type = CMD_EXEC;
        strncpy(cmd->filename, tokens[1], sizeof(cmd->filename) - 1);
    }
    else if (strcasecmp(tokens[0], "LOAD") == 0) {
        if (token_count != 2) {
            printf("Syntax Error: LOAD requires 1 argument (filename)\n");
            return -2;
        }
        cmd->type = CMD_LOAD;
        strncpy(cmd->filename, tokens[1], sizeof(cmd->filename) - 1);
    }
    else if (strcasecmp(tokens[0], "UNDO") == 0) {
        if (token_count != 1) {
            printf("Syntax Error: UNDO requires no arguments\n");
            return -2;
        }
        cmd->type = CMD_UNDO;
    }
    else if (strcasecmp(tokens[0], "IF") == 0) {
        // IF CELL x y IS symbol THEN command
        if (token_count < 8) {
            printf("Syntax Error: IF requires at least 7 arguments\n");
            return -2;
        }
        // Проверяем правильность формата IF команды
        if (strcasecmp(tokens[1], "CELL") != 0 || strcasecmp(tokens[4], "IS") != 0 || 
            strcasecmp(tokens[6], "THEN") != 0) {
            printf("Syntax Error: IF must follow format: IF CELL x y IS symbol THEN command\n");
            return -2;
        }
        cmd->type = CMD_IF;
        cmd->x = atoi(tokens[2]);
        cmd->y = atoi(tokens[3]);
        cmd->color = tokens[5][0];
        
        // Собираем команду после THEN в одну строку
        strcpy(cmd->then_command, "");
        for (int i = 7; i < token_count; i++) {
            if (i > 7) strcat(cmd->then_command, " ");
            strcat(cmd->then_command, tokens[i]);
        }
    }
    
    // Проверяем, удалось ли распознать команду
    if (cmd->type == CMD_UNKNOWN) {
        printf("Error: Unknown command '%s'\n", tokens[0]);
        return -1;
    }
    
    return 0;
}