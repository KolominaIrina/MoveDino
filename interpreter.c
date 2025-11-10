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

void interpreter_init(InterpreterContext* context) {
    if (context == NULL) return;
    
    field_init(&context->field);
    context->field_initialized = 0;
    context->dino_placed = 0;
    context->error_occurred = 0;
    context->display_enabled = 1;
    context->save_enabled = 1;
    context->display_interval = 1.0;
    strcpy(context->error_message, "");
}

void interpreter_set_display_options(InterpreterContext* context, int enabled, double interval) {
    if (context == NULL) return;
    
    context->display_enabled = enabled;
    context->display_interval = interval;
}

void interpreter_set_save_option(InterpreterContext* context, int enabled) {
    if (context == NULL) return;
    
    context->save_enabled = enabled;
}

const char* interpreter_get_error_message(InterpreterContext* context) {
    return (context != NULL) ? context->error_message : "Context is NULL";
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void wait_for_display(double seconds) {
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#else
    usleep((useconds_t)(seconds * 1000000));
#endif
}

int interpreter_execute_command(InterpreterContext* context, ParsedCommand* cmd) {
    if (context == NULL || cmd == NULL) {
        return -1;
    }
    
    if (context->error_occurred) {
        return -2;  // Уже произошла ошибка
    }
    
    int result = 0;
    
    switch (cmd->type) {
        case CMD_COMMENT:
            // Игнорируем комментарии
            break;
            
        case CMD_SIZE:
            if (context->field_initialized) {
                strcpy(context->error_message, "Error: SIZE command can only be used once");
                context->error_occurred = 1;
                return -1;
            }
            
            result = field_set_size(&context->field, cmd->x, cmd->y);
            if (result != 0) {
                strcpy(context->error_message, "Error: Invalid field size");
                context->error_occurred = 1;
                return -1;
            }
            context->field_initialized = 1;
            break;
            
        case CMD_START:
            if (!context->field_initialized) {
                strcpy(context->error_message, "Error: FIELD must be initialized before START");
                context->error_occurred = 1;
                return -1;
            }
            
            result = field_set_dino_position(&context->field, cmd->x, cmd->y);
            if (result != 0) {
                strcpy(context->error_message, "Error: Invalid dino start position");
                context->error_occurred = 1;
                return -1;
            }
            context->dino_placed = 1;
            break;
            
        case CMD_MOVE:
            if (!context->dino_placed) {
                strcpy(context->error_message, "Error: Dino not placed. Use START command first");
                context->error_occurred = 1;
                return -1;
            }
            
            Direction move_dir = parse_direction(cmd->direction);
            if (move_dir == DIR_UNKNOWN) {
                strcpy(context->error_message, "Error: Invalid direction for MOVE");
                context->error_occurred = 1;
                return -1;
            }
            
            int dx, dy;
            get_direction_offset(move_dir, &dx, &dy);
            result = field_move_dino(&context->field, dx, dy);
            
            if (result == -2) {
                strcpy(context->error_message, "Fatal Error: Dino fell into a hole!");
                context->error_occurred = 1;
                return -1;
            } else if (result == -3) {
                printf("Warning: Cannot move to occupied cell\n");
            }
            break;
            
        case CMD_PAINT:
            if (!context->dino_placed) {
                strcpy(context->error_message, "Error: Dino not placed. Use START command first");
                context->error_occurred = 1;
                return -1;
            }
            
            if (cmd->color < 'a' || cmd->color > 'z') {
                strcpy(context->error_message, "Error: Invalid color. Must be lowercase letter a-z");
                context->error_occurred = 1;
                return -1;
            }
            
            field_paint_cell(&context->field, cmd->color);
            break;
            
        case CMD_DIG:
            if (!context->dino_placed) {
                strcpy(context->error_message, "Error: Dino not placed. Use START command first");
                context->error_occurred = 1;
                return -1;
            }
            
            Direction dig_dir = parse_direction(cmd->direction);
            if (dig_dir == DIR_UNKNOWN) {
                strcpy(context->error_message, "Error: Invalid direction for DIG");
                context->error_occurred = 1;
                return -1;
            }
            
            get_direction_offset(dig_dir, &dx, &dy);
            result = field_create_object(&context->field, dx, dy, CELL_HOLE);
            if (result != 0) {
                printf("Warning: Cannot dig here - cell is not empty\n");
            }
            break;
            
        case CMD_JUMP:
            if (!context->dino_placed) {
                strcpy(context->error_message, "Error: Dino not placed. Use START command first");
                context->error_occurred = 1;
                return -1;
            }
            
            Direction jump_dir = parse_direction(cmd->direction);
            if (jump_dir == DIR_UNKNOWN) {
                strcpy(context->error_message, "Error: Invalid direction for JUMP");
                context->error_occurred = 1;
                return -1;
            }
            
            if (cmd->n <= 0) {
                strcpy(context->error_message, "Error: Jump distance must be positive");
                context->error_occurred = 1;
                return -1;
            }
            
            get_direction_offset(jump_dir, &dx, &dy);
            result = field_jump_dino(&context->field, dx, dy, cmd->n);
            
            if (result == -2) {
                strcpy(context->error_message, "Fatal Error: Dino fell into a hole during jump!");
                context->error_occurred = 1;
                return -1;
            } else if (result == -3) {
                printf("Warning: Jump blocked by obstacle\n");
            }
            break;
            
        default:
            // Для остальных команд - заглушки
            printf("Info: Command %d not fully implemented yet\n", cmd->type);
            break;
    }
    
    // Отображение состояния после команды
    if (context->display_enabled && !context->error_occurred) {
        clear_screen();
        field_display(&context->field);
        wait_for_display(context->display_interval);
    }
    
    return result;
}