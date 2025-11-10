#ifndef COMMANDS_H
#define COMMANDS_H

// Типы команд
typedef enum {
    CMD_UNKNOWN,
    CMD_COMMENT,
    CMD_SIZE,
    CMD_START,
    CMD_MOVE,
    CMD_PAINT,
    CMD_DIG,
    CMD_MOUND,
    CMD_JUMP,
    CMD_GROW,
    CMD_CUT,
    CMD_MAKE,
    CMD_PUSH,
    CMD_EXEC,
    CMD_LOAD,
    CMD_UNDO,
    CMD_IF
} CommandType;

// Направления
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UNKNOWN
} Direction;

// Функции для работы с командами
Direction parse_direction(const char* dir_str);
int get_direction_offset(Direction dir, int* dx, int* dy);

#endif