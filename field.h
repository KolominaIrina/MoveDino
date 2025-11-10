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

// Прототипы функций
void field_init(Field* field);
int field_set_size(Field* field, int width, int height);
int field_set_dino_position(Field* field, int x, int y);
int field_move_dino(Field* field, int dx, int dy);
void field_paint_cell(Field* field, char color);
int field_create_object(Field* field, int dx, int dy, CellType type);
int field_cut_tree(Field* field, int dx, int dy);
int field_push_stone(Field* field, int dx, int dy);
int field_jump_dino(Field* field, int dx, int dy, int distance);
Cell* field_get_cell(Field* field, int x, int y);
int field_check_cell_symbol(Field* field, int x, int y, char symbol);
void field_print(Field* field, FILE* output);
void field_display(Field* field);
const char* field_get_error_message(int error_code);
void field_copy(Field* dest, const Field* src);
int field_load_from_file(Field* field, const char* filename);

#endif