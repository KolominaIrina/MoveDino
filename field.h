#ifndef FIELD_H
#define FIELD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define MIN_SIZE 10

// Типы клеток поля
typedef enum {
    CELL_EMPTY = '_',
    CELL_DINO = '#',
    CELL_HOLE = '%',
    CELL_MOUNTAIN = '^',
    CELL_TREE = '&',
    CELL_STONE = '@'
} CellType;

// Структура для представления клетки
typedef struct {
    CellType type;
    char color;  // '\0' если нет цвета, иначе строчная латинская буква
} Cell;

// Структура для представления игрового поля
typedef struct {
    Cell grid[MAX_WIDTH][MAX_HEIGHT];
    int width;
    int height;
    int dino_x;
    int dino_y;
} Field;

// Инициализация поля
void field_init(Field* field);

// Установка размеров поля
int field_set_size(Field* field, int width, int height);

// Установка позиции динозавра
int field_set_dino_position(Field* field, int x, int y);

// Перемещение динозавра
int field_move_dino(Field* field, int dx, int dy);

// Покраска текущей клетки
void field_paint_cell(Field* field, char color);

// Создание объекта в соседней клетке
int field_create_object(Field* field, int dx, int dy, CellType type);

// Прыжок динозавра
int field_jump_dino(Field* field, int dx, int dy, int distance);

// Получение клетки по координатам (с учетом торической геометрии)
Cell* field_get_cell(Field* field, int x, int y);

// Вывод поля в файл
void field_print(Field* field, FILE* output);

// Вывод поля в консоль
void field_display(Field* field);

#endif