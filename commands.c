#include "commands.h"
#include <string.h>

// Парсинг строки 
Direction parse_direction(const char* dir_str) {
    if (dir_str == NULL) return DIR_UNKNOWN;
    
    // Сравниваем строку с известными направлениями (без учета регистра)
    if (strcasecmp(dir_str, "UP") == 0) return DIR_UP;
    if (strcasecmp(dir_str, "DOWN") == 0) return DIR_DOWN;
    if (strcasecmp(dir_str, "LEFT") == 0) return DIR_LEFT;
    if (strcasecmp(dir_str, "RIGHT") == 0) return DIR_RIGHT;
    
    return DIR_UNKNOWN;
}

// Получение смещения координат для указанного направления
int get_direction_offset(Direction dir, int* dx, int* dy) {
    if (dx == NULL || dy == NULL) return -1;
    
    // Устанавливаем смещение в зависимости от направления
    switch (dir) {
        case DIR_UP:
            *dx = 0; *dy = -1;  // Вверх: x не меняется, y уменьшается
            break;
        case DIR_DOWN:
            *dx = 0; *dy = 1;   // Вниз: x не меняется, y увеличивается
            break;
        case DIR_LEFT:
            *dx = -1; *dy = 0;  // Влево: x уменьшается, y не меняется
            break;
        case DIR_RIGHT:
            *dx = 1; *dy = 0;   // Вправо: x увеличивается, y не меняется
            break;
        default:
            return -1;  // Неизвестное направление
    }
    
    return 0;
}