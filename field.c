#include "field.h"
#include "utils.h"
#include <stdio.h>

// Инициализация поля значениями по умолчанию
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

// Получение текстового описания ошибки по коду
const char* field_get_error_message(int error_code) {
    switch (error_code) {
        case -1: return "Field not initialized";
        case -2: return "Coordinates out of field bounds";
        case -3: return "Impassable obstacle";
        case -4: return "Hole - fatal error";
        case -5: return "Jump blocked by obstacle";
        case -6: return "Cell not empty for object creation";
        case -7: return "No tree to cut";
        case -8: return "No stone to push";
        case -9: return "Stone push blocked";
        case -10: return "Stone hit tree - bounced back";
        default: return "Unknown error";
    }
}

// Установка размеров игрового поля
int field_set_size(Field* field, int width, int height) {
    // Проверка допустимых размеров
    if (width < MIN_SIZE || width > MAX_WIDTH || 
        height < MIN_SIZE || height > MAX_HEIGHT) {
        printf("Error: Invalid field size %dx%d. Valid range: %dx%d - %dx%d\n", 
               width, height, MIN_SIZE, MIN_SIZE, MAX_WIDTH, MAX_HEIGHT);
        return -1;
    }
    
    field->width = width;
    field->height = height;
    printf("Field set: %dx%d\n", width, height);
    return 0;
}

// Установка позиции динозавра на поле
int field_set_dino_position(Field* field, int x, int y) {
    // Проверка инициализации поля
    if (field->width == 0 || field->height == 0) {
        printf("Error: Field not initialized. Use SIZE command first\n");
        return -1;
    }
    
    // Проверка границ поля
    if (x < 0 || x >= field->width || y < 0 || y >= field->height) {
        printf("Error: Coordinates (%d, %d) out of field bounds %dx%d\n", 
               x, y, field->width, field->height);
        return -2;
    }
    
    // Если динозавр уже был где-то, очищаем старую позицию
    if (field->dino_x != -1 && field->dino_y != -1) {
        Cell* old_cell = field_get_cell(field, field->dino_x, field->dino_y);
        if (old_cell->type == CELL_DINO) {
            old_cell->type = CELL_EMPTY;
        }
    }
    
    // Устанавливаем новую позицию динозавра
    field->dino_x = x;
    field->dino_y = y;
    
    Cell* new_cell = field_get_cell(field, x, y);
    new_cell->type = CELL_DINO;
    
    printf("Dino placed at position (%d, %d)\n", x, y);
    return 0;
}

// Перемещение динозавра в указанном направлении
int field_move_dino(Field* field, int dx, int dy) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        printf("Error: Dino not placed. Use START command\n");
        return -1;
    }
    
    // Вычисление новой позиции с учетом торической геометрии
    int new_x = (field->dino_x + dx + field->width) % field->width;
    int new_y = (field->dino_y + dy + field->height) % field->height;
    
    Cell* target_cell = field_get_cell(field, new_x, new_y);
    
    // Проверка препятствий
    if (target_cell->type == CELL_HOLE) {
        printf("Fatal Error: Dino fell into a hole at cell (%d, %d)!\n", new_x, new_y);
        return -4;
    }
    if (target_cell->type == CELL_MOUNTAIN || 
        target_cell->type == CELL_TREE || 
        target_cell->type == CELL_STONE) {
        return -3;  // Возвращаем код ошибки, вывод будет в interpreter.c
    }
    
    // Перемещаем динозавра
    Cell* current_cell = field_get_cell(field, field->dino_x, field->dino_y);
    current_cell->type = CELL_EMPTY;
    
    field->dino_x = new_x;
    field->dino_y = new_y;
    target_cell->type = CELL_DINO;
    
    printf("Dino moved to (%d, %d)\n", new_x, new_y);
    return 0;
}

// Покраска текущей клетки, где находится динозавр
void field_paint_cell(Field* field, char color) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        printf("Error: Dino not placed. Use START command\n");
        return;
    }
    
    Cell* cell = field_get_cell(field, field->dino_x, field->dino_y);
    if (color >= 'a' && color <= 'z') {
        cell->color = color;
        printf("Cell (%d, %d) painted with color '%c'\n", field->dino_x, field->dino_y, color);
    } else {
        printf("Error: Invalid color '%c'. Valid colors are lowercase letters a-z\n", color);
    }
}

// Создание объекта в соседней клетке
int field_create_object(Field* field, int dx, int dy, CellType type) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        printf("Error: Dino not placed. Use START command\n");
        return -1;
    }
    
    // Вычисление координат целевой клетки
    int target_x = (field->dino_x + dx + field->width) % field->width;
    int target_y = (field->dino_y + dy + field->height) % field->height;
    
    Cell* target_cell = field_get_cell(field, target_x, target_y);
    
    // Можно создавать объекты только на пустых клетках
    if (target_cell->type != CELL_EMPTY) {
        return -6;
    }
    
    // Особый случай: гора на яме
    if (type == CELL_MOUNTAIN && target_cell->type == CELL_HOLE) {
    target_cell->type = CELL_EMPTY;
    // target_cell->color НЕ меняем - сохраняем цвет клетки!
    printf("Hole at cell (%d, %d) filled with mountain\n", target_x, target_y);
    return 0;
}
    
    // Создание объекта
    target_cell->type = type;
    
    const char* obj_name = "";
    switch (type) {
        case CELL_HOLE: obj_name = "hole"; break;
        case CELL_MOUNTAIN: obj_name = "mountain"; break;
        case CELL_TREE: obj_name = "tree"; break;
        case CELL_STONE: obj_name = "stone"; break;
        default: obj_name = "object"; break;
    }
    printf("Created %s at cell (%d, %d)\n", obj_name, target_x, target_y);
    
    return 0;
}

// Срубание дерева в соседней клетке
int field_cut_tree(Field* field, int dx, int dy) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        printf("Error: Dino not placed. Use START command\n");
        return -1;
    }
    
    int target_x = (field->dino_x + dx + field->width) % field->width;
    int target_y = (field->dino_y + dy + field->height) % field->height;
    
    Cell* target_cell = field_get_cell(field, target_x, target_y);
    
    // Проверка, что в клетке есть дерево
    if (target_cell->type != CELL_TREE) {
        return -7;  // Нет дерева для срубания
    }
    
    // Срубаем дерево - клетка становится пустой, но сохраняет цвет
    target_cell->type = CELL_EMPTY;
    // Цвет сохраняется автоматически
    
    printf("Tree cut at cell (%d, %d)\n", target_x, target_y);
    return 0;
}

// Пинание камня в соседней клетке
int field_push_stone(Field* field, int dx, int dy) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        printf("Error: Dino not placed. Use START command\n");
        return -1;
    }
    
    // Координаты камня
    int stone_x = (field->dino_x + dx + field->width) % field->width;
    int stone_y = (field->dino_y + dy + field->height) % field->height;
    
    Cell* stone_cell = field_get_cell(field, stone_x, stone_y);
    
    // Проверка, что в клетке есть камень
    if (stone_cell->type != CELL_STONE) {
        return -8;  // Нет камня для пинания
    }
    
    // Вычисляем направление пинания (противоположно положению динозавра)
    int push_dx = dx;
    int push_dy = dy;
    
    // Координаты целевой клетки для камня
    int new_x = (stone_x + push_dx + field->width) % field->width;
    int new_y = (stone_y + push_dy + field->height) % field->height;
    
    Cell* target_cell = field_get_cell(field, new_x, new_y);
    
    // Проверяем, куда катим камень
    if (target_cell->type == CELL_TREE) {
        return -10;  // Камень ударился о дерево
    }
    if (target_cell->type == CELL_MOUNTAIN || target_cell->type == CELL_STONE) {
        return -9;  // Препятствие
    }
    
    stone_cell->type = CELL_EMPTY;
    
    // Особый случай: камень в яму
    if (target_cell->type == CELL_HOLE) {
    target_cell->type = CELL_EMPTY;
    printf("Stone filled hole at cell (%d, %d)\n", new_x, new_y);
    } else {
    target_cell->type = CELL_STONE;
    printf("Stone pushed to (%d, %d)\n", new_x, new_y);
    }
    
    return 0;
}

// Прыжок динозавра на указанное расстояние
int field_jump_dino(Field* field, int dx, int dy, int distance) {
    if (field->dino_x == -1 || field->dino_y == -1) {
        printf("Error: Dino not placed. Use START command\n");
        return -1;
    }
    
    if (distance <= 0) {
        printf("Error: Jump distance must be positive\n");
        return -5;
    }
    
    int current_x = field->dino_x;
    int current_y = field->dino_y;
    int blocked_at_x = -1, blocked_at_y = -1;
    
    // Проверяем путь прыжка на наличие препятствий
    for (int i = 1; i <= distance; i++) {
        int check_x = (current_x + dx * i + field->width) % field->width;
        int check_y = (current_y + dy * i + field->height) % field->height;
        
        Cell* check_cell = field_get_cell(field, check_x, check_y);
        
        // Если встречаем гору, дерево или камень - останавливаемся перед ней
        if (check_cell->type == CELL_MOUNTAIN || 
            check_cell->type == CELL_TREE || 
            check_cell->type == CELL_STONE) {
            blocked_at_x = check_x;
            blocked_at_y = check_y;
            distance = i - 1;  // Прыгаем до препятствия
            break;
        }
        
        // Если приземляемся на яму - ошибка
        if (i == distance && check_cell->type == CELL_HOLE) {
            printf("Fatal Error: Dino landed in a hole at cell (%d, %d)!\n", 
                   check_x, check_y);
            return -4;
        }
    }
    
    if (distance == 0) {
        return -5;  // Прыжок заблокирован прямо перед динозавром
    }
    
    // Выполняем прыжок
    int new_x = (current_x + dx * distance + field->width) % field->width;
    int new_y = (current_y + dy * distance + field->height) % field->height;
    
    Cell* current_cell = field_get_cell(field, current_x, current_y);
    Cell* target_cell = field_get_cell(field, new_x, new_y);
    
    current_cell->type = CELL_EMPTY;
    field->dino_x = new_x;
    field->dino_y = new_y;
    target_cell->type = CELL_DINO;
    
    printf("Dino jumped %d cells to position (%d, %d)\n", distance, new_x, new_y);
    
    // Возвращаем информацию о блокировке для предупреждения
    if (blocked_at_x != -1) {
        return -100 - blocked_at_x * 1000 - blocked_at_y;  // Код с координатами препятствия
    }
    
    return 0;
}

// Проверка символа в указанной клетке
int field_check_cell_symbol(Field* field, int x, int y, char symbol) {
    // Проверка границ
    if (x < 0 || x >= field->width || y < 0 || y >= field->height) {
        return 0;
    }
    
    Cell* cell = field_get_cell(field, x, y);
    
    // Проверяем основной тип клетки
    if ((char)cell->type == symbol) {
        return 1;
    }
    
    // Проверяем цвет
    if (cell->color == symbol) {
        return 1;
    }
    
    return 0;
}

// Получение клетки по координатам (с учетом торической геометрии)
Cell* field_get_cell(Field* field, int x, int y) {
    // Торическая геометрия - координаты заворачиваются
    if (field->width == 0 || field->height == 0) {
        return NULL;
    }
    x = (x + field->width) % field->width;
    y = (y + field->height) % field->height;
    return &field->grid[x][y];
}

// Вывод поля в файл
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

// Вывод поля в консоль
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
    printf("Dino at position: (%d, %d)\n\n", field->dino_x, field->dino_y);
}

// Копирование состояния поля
void field_copy(Field* dest, const Field* src) {
    if (dest == NULL || src == NULL) return;
    
    // Копируем все поля структуры
    dest->width = src->width;
    dest->height = src->height;
    dest->dino_x = src->dino_x;
    dest->dino_y = src->dino_y;
    
    // Копируем сетку клеток
    for (int i = 0; i < MAX_WIDTH; i++) {
        for (int j = 0; j < MAX_HEIGHT; j++) {
            dest->grid[i][j] = src->grid[i][j];
        }
    }
}

// Загрузка поля из файла
int field_load_from_file(Field* field, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open file '%s'\n", filename);
        return -1;
    }
    
    char line[MAX_WIDTH + 2];  // +2 для \n и \0
    int height = 0;
    int width = 0;
    
    // Читаем файл для определения размеров
    while (fgets(line, sizeof(line), file)) {
        int line_length = strlen(line);
        if (line[line_length - 1] == '\n') {
            line_length--;
        }
        
        if (line_length > width) {
            width = line_length;
        }
        height++;
    }
    
    // Проверка допустимых размеров
    if (width < MIN_SIZE || width > MAX_WIDTH || height < MIN_SIZE || height > MAX_HEIGHT) {
        printf("Error: Invalid field size in file: %dx%d\n", width, height);
        fclose(file);
        return -1;
    }
    
    // Возвращаемся к началу файла
    fseek(file, 0, SEEK_SET);
    
    // Инициализируем поле
    field_init(field);
    field->width = width;
    field->height = height;
    
    // Загружаем данные из файла
    int y = 0;
    while (fgets(line, sizeof(line), file) && y < height) {
        int line_length = strlen(line);
        if (line[line_length - 1] == '\n') {
            line_length--;
        }
        
        for (int x = 0; x < line_length && x < width; x++) {
            Cell* cell = field_get_cell(field, x, y);
            
            if (line[x] >= 'a' && line[x] <= 'z') {
                // Цветная клетка
                cell->type = CELL_EMPTY;
                cell->color = line[x];
            } else {
                // Объект
                cell->color = '\0';
                switch (line[x]) {
                    case '_': cell->type = CELL_EMPTY; break;
                    case '#': 
                        cell->type = CELL_DINO; 
                        field->dino_x = x;
                        field->dino_y = y;
                        break;
                    case '%': cell->type = CELL_HOLE; break;
                    case '^': cell->type = CELL_MOUNTAIN; break;
                    case '&': cell->type = CELL_TREE; break;
                    case '@': cell->type = CELL_STONE; break;
                    default: cell->type = CELL_EMPTY; break;
                }
            }
        }
        y++;
    }
    
    fclose(file);
    printf("Field loaded from '%s': %dx%d\n", filename, width, height);
    return 0;
}