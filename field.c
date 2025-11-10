#include "field.h"
#include "utils.h"

void field_init(Field* field) {
    field->width = 0;
    field->height = 0;
    field->dino_x = -1;
    field->dino_y = -1;
    
    // Инициализация всех клеток как пустых
    for (int i = 0; i < MAX_WIDTH; i++) {
        for (int j = 0; j < MAX_HEIGHT; j++) {
            field->grid[i][j].type = CELL_EMPTY;
            field->grid[i][j].color = '\0';
        }
    }
}

int field_set_size(Field* field, int width, int height) {
    if (width < MIN_SIZE || width > MAX_WIDTH || 
        height < MIN_SIZE || height > MAX_HEIGHT) {
        return -1;  // Некорректный размер
    }
    
    field->width = width;
    field->height = height;
    return 0;
}

int field_set_dino_position(Field* field, int x, int y) {
    if (field->width == 0 || field->height == 0) {
        return -1;  // Поле не инициализировано
    }
    
    if (x < 0 || x >= field->width || y < 0 || y >= field->height) {
        return -2;  // Координаты вне поля
    }
    
    // Если динозавр уже был где-то, очищаем старую позицию
    if (field->dino_x != -1 && field->dino_y != -1) {
        Cell* old_cell = field_get_cell(field, field->dino_x, field->dino_y);
        if (old_cell->type == CELL_DINO) {
            old_cell->type = CELL_EMPTY;
        }
    }
    
    field->dino_x = x;
    field->dino_y = y;
    
    Cell* new_cell = field_get_cell(field, x, y);
    new_cell->type = CELL_DINO;
    
    return 0;
}

int field_move_dino(Field* field, int dx, int dy) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        return -1;  // Динозавр не размещен
    }
    
    int new_x = (field->dino_x + dx + field->width) % field->width;
    int new_y = (field->dino_y + dy + field->height) % field->height;
    
    Cell* target_cell = field_get_cell(field, new_x, new_y);
    
    // Проверка препятствий
    if (target_cell->type == CELL_HOLE) {
        return -2;  // Яма - фатальная ошибка
    }
    if (target_cell->type == CELL_MOUNTAIN || 
        target_cell->type == CELL_TREE || 
        target_cell->type == CELL_STONE) {
        return -3;  // Непроходимое препятствие
    }
    
    // Перемещаем динозавра
    Cell* current_cell = field_get_cell(field, field->dino_x, field->dino_y);
    current_cell->type = CELL_EMPTY;
    
    field->dino_x = new_x;
    field->dino_y = new_y;
    target_cell->type = CELL_DINO;
    
    return 0;
}

void field_paint_cell(Field* field, char color) {
    if (field->dino_x == -1 || field->dino_y == -1) return;
    
    Cell* cell = field_get_cell(field, field->dino_x, field->dino_y);
    if (color >= 'a' && color <= 'z') {
        cell->color = color;
    }
}

int field_create_object(Field* field, int dx, int dy, CellType type) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        return -1;  // Динозавр не размещен
    }
    
    int target_x = (field->dino_x + dx + field->width) % field->width;
    int target_y = (field->dino_y + dy + field->height) % field->height;
    
    Cell* target_cell = field_get_cell(field, target_x, target_y);
    
    // Можно создавать объекты только на пустых клетках
    if (target_cell->type != CELL_EMPTY) {
        return -2;
    }
    
    // Особый случай: гора на яме
    if (type == CELL_MOUNTAIN && target_cell->type == CELL_HOLE) {
        target_cell->type = CELL_EMPTY;
        target_cell->color = '\0';
        return 0;
    }
    
    target_cell->type = type;
    return 0;
}

int field_jump_dino(Field* field, int dx, int dy, int distance) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        return -1;  // Динозавр не размещен
    }
    
    int current_x = field->dino_x;
    int current_y = field->dino_y;
    
    // Проверяем путь прыжка
    for (int i = 1; i <= distance; i++) {
        int check_x = (current_x + dx * i + field->width) % field->width;
        int check_y = (current_y + dy * i + field->height) % field->height;
        
        Cell* check_cell = field_get_cell(field, check_x, check_y);
        
        // Если встречаем гору, дерево или камень - останавливаемся перед ней
        if (check_cell->type == CELL_MOUNTAIN || 
            check_cell->type == CELL_TREE || 
            check_cell->type == CELL_STONE) {
            distance = i - 1;  // Прыгаем до препятствия
            break;
        }
        
        // Если приземляемся на яму - ошибка
        if (i == distance && check_cell->type == CELL_HOLE) {
            return -2;
        }
    }
    
    if (distance == 0) return -3;  // Прыжок заблокирован
    
    // Выполняем прыжок
    int new_x = (current_x + dx * distance + field->width) % field->width;
    int new_y = (current_y + dy * distance + field->height) % field->height;
    
    Cell* current_cell = field_get_cell(field, current_x, current_y);
    Cell* target_cell = field_get_cell(field, new_x, new_y);
    
    current_cell->type = CELL_EMPTY;
    field->dino_x = new_x;
    field->dino_y = new_y;
    target_cell->type = CELL_DINO;
    
    return 0;
}

Cell* field_get_cell(Field* field, int x, int y) {
    // Торическая геометрия - координаты заворачиваются
    x = (x + field->width) % field->width;
    y = (y + field->height) % field->height;
    return &field->grid[x][y];
}

void field_print(Field* field, FILE* output) {
    for (int y = 0; y < field->height; y++) {
        for (int x = 0; x < field->width; x++) {
            Cell* cell = &field->grid[x][y];
            if (cell->color != '\0') {
                fprintf(output, "%c", cell->color);
            } else {
                fprintf(output, "%c", (char)cell->type);
            }
        }
        fprintf(output, "\n");
    }
}

void field_display(Field* field) {
    printf("\n");
    for (int y = 0; y < field->height; y++) {
        for (int x = 0; x < field->width; x++) {
            Cell* cell = &field->grid[x][y];
            if (cell->color != '\0') {
                printf("%c", cell->color);
            } else {
                printf("%c", (char)cell->type);
            }
        }
        printf("\n");
    }
    printf("Dino at: (%d, %d)\n\n", field->dino_x, field->dino_y);
}