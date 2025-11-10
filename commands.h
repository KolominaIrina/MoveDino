#ifndef COMMANDS_H
#define COMMANDS_H

// Перечисление типов команд
typedef enum {
    CMD_UNKNOWN,    // Неизвестная команда
    CMD_COMMENT,    // Комментарий
    CMD_SIZE,       // Установка размера поля
    CMD_START,      // Установка позиции динозавра
    CMD_MOVE,       // Перемещение динозавра
    CMD_PAINT,      // Покраска клетки
    CMD_DIG,        // Создание ямы
    CMD_MOUND,      // Создание горы
    CMD_JUMP,       // Прыжок динозавра
    CMD_GROW,       // Выращивание дерева
    CMD_CUT,        // Срубание дерева
    CMD_MAKE,       // Создание камня
    CMD_PUSH,       // Пинание камня
    CMD_EXEC,       // Выполнение файла
    CMD_LOAD,       // Загрузка поля из файла
    CMD_UNDO,       // Откат действия
    CMD_IF          // Условная команда
} CommandType;

// Перечисление направлений движения
typedef enum {
    DIR_UP,         // Вверх
    DIR_DOWN,       // Вниз
    DIR_LEFT,       // Влево
    DIR_RIGHT,      // Вправо
    DIR_UNKNOWN     // Неизвестное направление
} Direction;

// Функции для работы с командами
Direction parse_direction(const char* dir_str);
int get_direction_offset(Direction dir, int* dx, int* dy);

#endif