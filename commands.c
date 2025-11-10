#include "commands.h"
#include <string.h>

Direction parse_direction(const char* dir_str) {
    if (dir_str == NULL) return DIR_UNKNOWN;
    
    if (strcasecmp(dir_str, "UP") == 0) return DIR_UP;
    if (strcasecmp(dir_str, "DOWN") == 0) return DIR_DOWN;
    if (strcasecmp(dir_str, "LEFT") == 0) return DIR_LEFT;
    if (strcasecmp(dir_str, "RIGHT") == 0) return DIR_RIGHT;
    
    return DIR_UNKNOWN;
}

int get_direction_offset(Direction dir, int* dx, int* dy) {
    if (dx == NULL || dy == NULL) return -1;
    
    switch (dir) {
        case DIR_UP:
            *dx = 0; *dy = -1;
            break;
        case DIR_DOWN:
            *dx = 0; *dy = 1;
            break;
        case DIR_LEFT:
            *dx = -1; *dy = 0;
            break;
        case DIR_RIGHT:
            *dx = 1; *dy = 0;
            break;
        default:
            return -1;
    }
    
    return 0;
}