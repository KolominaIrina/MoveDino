#include "interpreter.h"
#include "commands.h"
#include "utils.h"
#include <unistd.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Инициализация контекста интерпретатора
void interpreter_init(InterpreterContext* context) {
    if (context == NULL) return;
    
    // Инициализация основных полей
    field_init(&context->field);
    context->field_initialized = 0;
    context->dino_placed = 0;
    context->error_occurred = 0;
    context->display_enabled = 1;
    context->save_enabled = 1;
    context->display_interval = 1.0;
    strcpy(context->error_message, "");
    strcpy(context->current_filename, "");
    context->exec_depth = 0;
    
    // Инициализация истории для UNDO
    context->history_size = 0;
    context->current_history_index = -1;
    
    // Инициализация предупреждений
    strcpy(context->warning_message, "");
    context->has_warning = 0;
}

// Установка параметров отображения
void interpreter_set_display_options(InterpreterContext* context, int enabled, double interval) {
    if (context == NULL) return;
    
    context->display_enabled = enabled;
    context->display_interval = interval;
}

// Установка параметра сохранения
void interpreter_set_save_option(InterpreterContext* context, int enabled) {
    if (context == NULL) return;
    
    context->save_enabled = enabled;
}

// Получение текста последней ошибки
const char* interpreter_get_error_message(InterpreterContext* context) {
    return (context != NULL) ? context->error_message : "Context is NULL";
}

// Очистка экрана консоли 
void clear_screen() {
#ifdef _WIN32
    system("cls");  // Windows
#else
    system("clear"); // Linux/Mac
#endif
}

// Задержка выполнения 
void wait_for_display(double seconds) {
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));     // Windows - миллисекунды
#else
    usleep((useconds_t)(seconds * 1000000));  // Linux/Mac - микросекунды
#endif
}

// Сохранение текущего состояния в историю для UNDO
void interpreter_save_state(InterpreterContext* context) {
    if (context == NULL || !context->field_initialized) return;
    
    // Сдвигаем все состояния, если достигли максимального
    if (context->history_size >= MAX_UNDO_LEVELS) {
        // Удаление самого первого состояния
        for (int i = 0; i < MAX_UNDO_LEVELS - 1; i++) {
            field_copy(&context->history[i], &context->history[i + 1]);
        }
        context->history_size = MAX_UNDO_LEVELS - 1;
    }
    
    // Сохранение текущего состояния
    field_copy(&context->history[context->history_size], &context->field);
    context->current_history_index = context->history_size;
    context->history_size++;
}

// Откат к предыдущему состоянию (UNDO)
int interpreter_undo(InterpreterContext* context) {
    if (context == NULL) return -1;
    
    // Проверка наличия состояний в истории
    if (context->history_size == 0) {
        printf("Error: No states in history to undo\n");
        return -2;
    }
    
    // Проверка, что не достигли начала истории
    if (context->current_history_index == 0) {
        printf("Error: Already at the earliest state\n");
        return -3;
    }
    
    // Возврат к предыдущему состоянию
    context->current_history_index--;
    field_copy(&context->field, &context->history[context->current_history_index]);
    
    printf("Undo successful. Restored state %d of %d\n", 
           context->current_history_index + 1, context->history_size);
    
    return 0;
}

// Установка предупреждающего сообщения
void interpreter_set_warning(InterpreterContext* context, const char* warning) {
    if (context != NULL && warning != NULL) {
        strncpy(context->warning_message, warning, sizeof(context->warning_message) - 1);
        context->warning_message[sizeof(context->warning_message) - 1] = '\0';
        context->has_warning = 1;
    }
}

// Отображение предупреждений
void interpreter_show_warnings(InterpreterContext* context) {
    if (context != NULL && context->has_warning) {
        printf("\n🚨 WARNING: %s\n\n", context->warning_message);
        context->has_warning = 0;  // Сбрасываем флаг после показа
    }
}

// Выполнение команды из условия IF
int interpreter_execute_if_command(InterpreterContext* context, ParsedCommand* cmd, int line_number) {
    if (context == NULL || cmd == NULL) {
        printf("Error: Null pointer in IF command execution\n");
        return -1;
    }
    
    // Проверка инициализации поля
    if (!context->field_initialized) {
        printf("Error: Field not initialized for IF command\n");
        return -2;
    }
    
    // Проверка условия IF
    int condition_met = field_check_cell_symbol(&context->field, cmd->x, cmd->y, cmd->color);
    
    if (condition_met) {
        printf("Condition met! Symbol '%c' found at (%d, %d). Executing: %s\n", 
               cmd->color, cmd->x, cmd->y, cmd->then_command);
        
        // Парсинг и выполнение команды из блока THEN
        ParsedCommand then_cmd;
        int parse_result = parse_line(cmd->then_command, &then_cmd);
        
        if (parse_result != 0) {
            printf("Error parsing THEN command: %s\n", cmd->then_command);
            return -3;
        }
        
        if (then_cmd.type == CMD_COMMENT) {
            printf("THEN command is a comment - skipped\n");
            return 0;
        }
        
        // Сохранение состояния перед выполнением команды THEN
        interpreter_save_state(context);
        
        // Выполнение команды THEN
        int result = interpreter_execute_command(context, &then_cmd, line_number);
        
        if (result < 0 && context->error_occurred) {
            printf("Error executing THEN command: %s\n", interpreter_get_error_message(context));
            return -4;
        }
        
        return result;
    } else {
        printf("Condition not met. Symbol '%c' not found at (%d, %d)\n", cmd->color, cmd->x, cmd->y);
        return 0;
    }
}

// Выполнение команд из файла
int interpreter_execute_file(InterpreterContext* context, const char* filename) {
    if (context == NULL || filename == NULL) {
        printf("Error: Null pointer when executing file\n");
        return -1;
    }
    
    // Проверка глубины вложенности (защита от бесконечной рекурсии)
    if (context->exec_depth >= 10) {
        printf("Error: Maximum EXEC depth exceeded (10 levels)\n");
        return -2;
    }
    
    // Сохранение текущего имени файла
    char old_filename[256];
    strcpy(old_filename, context->current_filename);
    strcpy(context->current_filename, filename);
    context->exec_depth++;
    
    printf("=== Executing file: %s (depth: %d) ===\n", filename, context->exec_depth);
    
    // Открытие файла
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open file '%s'\n", filename);
        // Восстанавление предыдущее состояние
        strcpy(context->current_filename, old_filename);
        context->exec_depth--;
        return -3;
    }
    
    // Чтение и выполнение команд из файла
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    int result = 0;
    
    while (read_line(file, line, sizeof(line)) != NULL && !context->error_occurred) {
        line_number++;
        
        ParsedCommand cmd;
        int parse_result = parse_line(line, &cmd);
        
        if (parse_result != 0) {
            printf("Error parsing line %d in %s: %s\n", line_number, filename, line);
            continue;
        }
        
        if (cmd.type == CMD_COMMENT) {
            continue;  // Пропуск комментариев
        }
        
        result = interpreter_execute_command(context, &cmd, line_number);
        if (result < 0 && context->error_occurred) {
            printf("Fatal error at line %d in %s: %s\n", line_number, filename, 
                   interpreter_get_error_message(context));
            break;
        }
    }
    
    fclose(file);
    printf("=== Finished executing: %s ===\n", filename);
    
    // Восстановление предыдущего имени файла
    strcpy(context->current_filename, old_filename);
    context->exec_depth--;
    
    return result;
}

// Основная функция выполнения команды
int interpreter_execute_command(InterpreterContext* context, ParsedCommand* cmd, int line_number) {
    if (context == NULL || cmd == NULL) {
        printf("Error: Null pointer when executing command\n");
        return -1;
    }
    
    // Проверка наличия предыдущей ошибки
    if (context->error_occurred) {
        return -2;
    }
    
    // Вывод информации о выполняемой команде
    if (strlen(context->current_filename) > 0) {
        printf("Executing %s line %d: ", context->current_filename, line_number);
    } else {
        printf("Executing line %d: ", line_number);
    }
    
    // Сохранение состояние перед выполнением команды 
    if (cmd->type != CMD_UNDO && cmd->type != CMD_LOAD && cmd->type != CMD_EXEC && 
        cmd->type != CMD_COMMENT && context->field_initialized) {
        interpreter_save_state(context);
    }
    
    int result = 0;
    int dx, dy;
    
    // Различные типы комманд
    switch (cmd->type) {
        case CMD_COMMENT: // Пропуск комментариев без выполнения
            printf("Comment - skipped\n");
            break;
            
        case CMD_SIZE: // Установка размера игрового поля
            printf("SIZE %d %d\n", cmd->x, cmd->y);
            if (context->field_initialized) {
                printf("Error: SIZE command can only be used once\n");
                context->error_occurred = 1;
                strcpy(context->error_message, "SIZE command already used");
                return -1;
            }
            
            result = field_set_size(&context->field, cmd->x, cmd->y);
            if (result != 0) {
                context->error_occurred = 1;
                strcpy(context->error_message, "Invalid field size");
                return -1;
            }
            context->field_initialized = 1;
            break;
            
        case CMD_START: // Установка начальной позиции динозавра
            printf("START %d %d\n", cmd->x, cmd->y);
            if (!context->field_initialized) {
                printf("Error: Field not initialized. Use SIZE first\n");
                context->error_occurred = 1;
                strcpy(context->error_message, "FIELD must be initialized before START");
                return -1;
            }
            
            result = field_set_dino_position(&context->field, cmd->x, cmd->y);
            if (result != 0) {
                context->error_occurred = 1;
                strcpy(context->error_message, "Invalid dino start position");
                return -1;
            }
            context->dino_placed = 1;
            break;
            
        case CMD_MOVE: // Перемещение динозавра в указанном направлении
            printf("MOVE %s\n", cmd->direction);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                context->error_occurred = 1;
                strcpy(context->error_message, "Dino not placed. Use START command first");
                return -1;
            }
            
            Direction move_dir = parse_direction(cmd->direction);
            if (move_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for MOVE\n", cmd->direction);
                context->error_occurred = 1;
                strcpy(context->error_message, "Invalid direction for MOVE");
                return -1;
            }
            
            get_direction_offset(move_dir, &dx, &dy);
            result = field_move_dino(&context->field, dx, dy);
            
            if (result == -4) {
                context->error_occurred = 1;
                strcpy(context->error_message, "Dino fell into a hole!");
                return -1;
            } else if (result == -3) {
                int new_x = (context->field.dino_x + dx + context->field.width) % context->field.width;
                int new_y = (context->field.dino_y + dy + context->field.height) % context->field.height;
                char warning[100];
                snprintf(warning, sizeof(warning), "Movement blocked by obstacle at cell (%d, %d)", new_x, new_y);
                interpreter_set_warning(context, warning);
            } else if (result < 0) {
                printf("Movement error: %s\n", field_get_error_message(result));
            }
            break;
            
        case CMD_PAINT: // Закрашивание текущей клетки указанным цветом
            printf("PAINT %c\n", cmd->color);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                context->error_occurred = 1;
                strcpy(context->error_message, "Dino not placed. Use START command first");
                return -1;
            }
            
            if (cmd->color < 'a' || cmd->color > 'z') {
                printf("Error: Invalid color '%c'. Must be lowercase letter a-z\n", cmd->color);
                context->error_occurred = 1;
                strcpy(context->error_message, "Invalid color. Must be lowercase letter a-z");
                return -1;
            }
            
            field_paint_cell(&context->field, cmd->color);
            break;
            
        case CMD_DIG: // Создание ямы в указанном направлении
            printf("DIG %s\n", cmd->direction);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                context->error_occurred = 1;
                strcpy(context->error_message, "Dino not placed. Use START command first");
                return -1;
            }
            
            Direction dig_dir = parse_direction(cmd->direction);
            if (dig_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for DIG\n", cmd->direction);
                context->error_occurred = 1;
                strcpy(context->error_message, "Invalid direction for DIG");
                return -1;
            }
            
            get_direction_offset(dig_dir, &dx, &dy);
            result = field_create_object(&context->field, dx, dy, CELL_HOLE);
            if (result == -6) {
                int target_x = (context->field.dino_x + dx + context->field.width) % context->field.width;
                int target_y = (context->field.dino_y + dy + context->field.height) % context->field.height;
                char warning[100];
                snprintf(warning, sizeof(warning), "Cannot dig hole at cell (%d, %d) - cell is occupied", target_x, target_y);
                interpreter_set_warning(context, warning);
            }
            break;
            
        case CMD_MOUND: // Создание горы в указанном направлении
            printf("MOUND %s\n", cmd->direction);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                return -1;
            }
            
            Direction mound_dir = parse_direction(cmd->direction);
            if (mound_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for MOUND\n", cmd->direction);
                return -1;
            }
            
            get_direction_offset(mound_dir, &dx, &dy);
            result = field_create_object(&context->field, dx, dy, CELL_MOUNTAIN);
            if (result == -6) {
                int target_x = (context->field.dino_x + dx + context->field.width) % context->field.width;
                int target_y = (context->field.dino_y + dy + context->field.height) % context->field.height;
                char warning[100];
                snprintf(warning, sizeof(warning), "Cannot create mountain at cell (%d, %d) - cell is occupied", target_x, target_y);
                interpreter_set_warning(context, warning);
            }
            break;
            
        case CMD_JUMP: // Прыжок динозавра на указанное расстояние в указанном направлении
            printf("JUMP %s %d\n", cmd->direction, cmd->n);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                context->error_occurred = 1;
                strcpy(context->error_message, "Dino not placed. Use START command first");
                return -1;
            }
            
            Direction jump_dir = parse_direction(cmd->direction);
            if (jump_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for JUMP\n", cmd->direction);
                context->error_occurred = 1;
                strcpy(context->error_message, "Invalid direction for JUMP");
                return -1;
            }
            
            if (cmd->n <= 0) {
                printf("Error: Jump distance must be positive\n");
                context->error_occurred = 1;
                strcpy(context->error_message, "Jump distance must be positive");
                return -1;
            }
            
            get_direction_offset(jump_dir, &dx, &dy);
            result = field_jump_dino(&context->field, dx, dy, cmd->n);
            
            if (result == -4) {
                context->error_occurred = 1;
                strcpy(context->error_message, "Dino fell into a hole during jump!");
                return -1;
            } else if (result == -5) {
                interpreter_set_warning(context, "Jump blocked - obstacle right in front of dino");
            } else if (result < -100) {
                // Извлекаем координаты препятствия из кода ошибки
                int error_code = -result - 100;
                int blocked_x = error_code / 1000;
                int blocked_y = error_code % 1000;
                char warning[100];
                snprintf(warning, sizeof(warning), "Jump blocked by obstacle at cell (%d, %d)", blocked_x, blocked_y);
                interpreter_set_warning(context, warning);
            } else if (result < 0) {
                printf("Jump error: %s\n", field_get_error_message(result));
            }
            break;
            
        case CMD_GROW: // Выращивание дерева в указанном направлении
            printf("GROW %s\n", cmd->direction);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                return -1;
            }
            
            Direction grow_dir = parse_direction(cmd->direction);
            if (grow_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for GROW\n", cmd->direction);
                return -1;
            }
            
            get_direction_offset(grow_dir, &dx, &dy);
            result = field_create_object(&context->field, dx, dy, CELL_TREE);
            if (result == -6) {
                int target_x = (context->field.dino_x + dx + context->field.width) % context->field.width;
                int target_y = (context->field.dino_y + dy + context->field.height) % context->field.height;
                char warning[100];
                snprintf(warning, sizeof(warning), "Cannot grow tree at cell (%d, %d) - cell is occupied", target_x, target_y);
                interpreter_set_warning(context, warning);
            }
            break;
            
        case CMD_CUT: // Срубание дерева в указанном направлении
            printf("CUT %s\n", cmd->direction);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                return -1;
            }
            
            Direction cut_dir = parse_direction(cmd->direction);
            if (cut_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for CUT\n", cmd->direction);
                return -1;
            }
            
            get_direction_offset(cut_dir, &dx, &dy);
            result = field_cut_tree(&context->field, dx, dy);
            if (result == -7) {
                int target_x = (context->field.dino_x + dx + context->field.width) % context->field.width;
                int target_y = (context->field.dino_y + dy + context->field.height) % context->field.height;
                char warning[100];
                snprintf(warning, sizeof(warning), "Cannot cut at cell (%d, %d) - no tree found", target_x, target_y);
                interpreter_set_warning(context, warning);
            } else if (result < 0) {
                printf("Cut error: %s\n", field_get_error_message(result));
            }
            break;
            
        case CMD_MAKE: // Создание камня в указанном направлении
            printf("MAKE %s\n", cmd->direction);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                return -1;
            }
            
            Direction make_dir = parse_direction(cmd->direction);
            if (make_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for MAKE\n", cmd->direction);
                return -1;
            }
            
            get_direction_offset(make_dir, &dx, &dy);
            result = field_create_object(&context->field, dx, dy, CELL_STONE);
            if (result == -6) {
                int target_x = (context->field.dino_x + dx + context->field.width) % context->field.width;
                int target_y = (context->field.dino_y + dy + context->field.height) % context->field.height;
                char warning[100];
                snprintf(warning, sizeof(warning), "Cannot create stone at cell (%d, %d) - cell is occupied", target_x, target_y);
                interpreter_set_warning(context, warning);
            }
            break;
            
        case CMD_PUSH: // Толкание камня в указанном направлении
            printf("PUSH %s\n", cmd->direction);
            if (!context->dino_placed) {
                printf("Error: Dino not placed. Use START command first\n");
                return -1;
            }
            
            Direction push_dir = parse_direction(cmd->direction);
            if (push_dir == DIR_UNKNOWN) {
                printf("Error: Invalid direction '%s' for PUSH\n", cmd->direction);
                return -1;
            }
            
            get_direction_offset(push_dir, &dx, &dy);
            result = field_push_stone(&context->field, dx, dy);
            if (result == -8) {
                int target_x = (context->field.dino_x + dx + context->field.width) % context->field.width;
                int target_y = (context->field.dino_y + dy + context->field.height) % context->field.height;
                char warning[100];
                snprintf(warning, sizeof(warning), "Cannot push at cell (%d, %d) - no stone found", target_x, target_y);
                interpreter_set_warning(context, warning);
            } else if (result == -9) {
                interpreter_set_warning(context, "Stone push blocked by obstacle");
            } else if (result == -10) {
                interpreter_set_warning(context, "Stone hit tree and bounced back!");
            } else if (result < 0) {
                printf("Push error: %s\n", field_get_error_message(result));
            }
            break;
            
        case CMD_UNDO: // Откат последнего действия
            printf("UNDO\n");
            result = interpreter_undo(context);
            if (result != 0) {
                printf("Undo failed\n");
            }
            break;
            
        case CMD_EXEC: // Выполнение команд из внешнего файла
            printf("EXEC %s\n", cmd->filename);
            result = interpreter_execute_file(context, cmd->filename);
            if (result != 0) {
                printf("EXEC failed for file: %s\n", cmd->filename);
            }
            break;
            
        case CMD_LOAD: // Загрузка состояния поля из файла
            printf("LOAD %s\n", cmd->filename);
            if (context->field_initialized) {
                printf("Error: LOAD can only be used as first command\n");
                return -1;
            }
            
            result = field_load_from_file(&context->field, cmd->filename);
            if (result == 0) {
                context->field_initialized = 1;
                if (context->field.dino_x != -1 && context->field.dino_y != -1) {
                    context->dino_placed = 1;
                }
            } else {
                printf("LOAD failed for file: %s\n", cmd->filename);
            }
            break;
            
        case CMD_IF: // Условное выполнение команды
            printf("IF CELL %d %d IS %c THEN %s\n", cmd->x, cmd->y, cmd->color, cmd->then_command);
            result = interpreter_execute_if_command(context, cmd, line_number);
            break;
            
        default:
            printf("Command %d not fully implemented yet\n", cmd->type);
            break;
    }
    
    // Отображение состояния после команды (если включена визуализация)
    if (context->display_enabled && !context->error_occurred) {
        clear_screen();
        field_display(&context->field);
        interpreter_show_warnings(context);  // Предупреждения после поля
        wait_for_display(context->display_interval);
    } else if (context->has_warning) {
        // Если визуализация отключена, всё равно показываются предупреждения
        interpreter_show_warnings(context);
    }
    
    return result;
}