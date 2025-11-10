#include "commands.h"
#include <string.h>

// Парсинг строки направления
Direction parse_direction(const char* dir_str) {
    if (dir_str == NULL) return DIR_UNKNOWN;
    
    // Сравнение строки с известными направлениями (без учета регистра)
    if (strcasecmp(dir_str, "UP") == 0) return DIR_UP;
    if (strcasecmp(dir_str, "DOWN") == 0) return DIR_DOWN;
    if (strcasecmp(dir_str, "LEFT") == 0) return DIR_LEFT;
    if (strcasecmp(dir_str, "RIGHT") == 0) return DIR_RIGHT;
    
    return DIR_UNKNOWN;
}

// Изменение координат 
int get_direction_offset(Direction dir, int* dx, int* dy) {
    if (dx == NULL || dy == NULL) return -1;
    
    // Перемещение динозавра
    switch (dir) {
        case DIR_UP:
            *dx = 0; *dy = -1;  // Направление вверх: x не меняется, y уменьшается
            break;
        case DIR_DOWN:
            *dx = 0; *dy = 1;   // Направление вниз: x не меняется, y увеличивается
            break;
        case DIR_LEFT:
            *dx = -1; *dy = 0;  // Направление влево: x уменьшается, y не меняется
            break;
        case DIR_RIGHT:
            *dx = 1; *dy = 0;   // Направление вправо: x увеличивается, y не меняется
            break;
        default:
            return -1;  // Неизвестное направление
    }
    
    return 0;
}