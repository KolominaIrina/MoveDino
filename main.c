#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "field.h"
#include "parser.h"
#include "interpreter.h"
#include "utils.h"

void print_usage(const char* program_name) {
    printf("Usage: %s input.txt output.txt [options]\n", program_name);
    printf("Options:\n");
    printf("  --interval N    Set display interval in seconds (default: 1.0)\n");
    printf("  --no-display    Disable console visualization\n");
    printf("  --no-save       Disable saving final state to output file\n");
    printf("  --help          Show this help message\n");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Обработка аргументов командной строки
    char* input_filename = argv[1];
    char* output_filename = argv[2];
    
    int display_enabled = 1;
    int save_enabled = 1;
    double display_interval = 1.0;
    
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--no-display") == 0) {
            display_enabled = 0;
        } else if (strcmp(argv[i], "--no-save") == 0) {
            save_enabled = 0;
        } else if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
            display_interval = atof(argv[++i]);
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            printf("Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // Проверка существования входного файла
    if (!file_exists(input_filename)) {
        printf("Error: Input file '%s' not found\n", input_filename);
        return 1;
    }
    
    // Инициализация интерпретатора
    InterpreterContext context;
    interpreter_init(&context);
    interpreter_set_display_options(&context, display_enabled, display_interval);
    interpreter_set_save_option(&context, save_enabled);
    
    // Открытие входного файла
    FILE* input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Cannot open input file '%s'\n", input_filename);
        return 1;
    }
    
    // Чтение и выполнение команд
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    
    while (read_line(input_file, line, sizeof(line)) != NULL) {
        line_number++;
        
        ParsedCommand cmd;
        int parse_result = parse_line(line, &cmd);
        
        if (parse_result != 0) {
            printf("Error parsing line %d: %s\n", line_number, line);
            continue;
        }
        
        if (cmd.type == CMD_COMMENT) {
            continue;  // Пропускаем комментарии
        }
        
        int exec_result = interpreter_execute_command(&context, &cmd);
        if (exec_result < 0 && context.error_occurred) {
            printf("Fatal error at line %d: %s\n", line_number, interpreter_get_error_message(&context));
            break;
        }
    }
    
    fclose(input_file);
    
    // Сохранение результата
    if (save_enabled && !context.error_occurred) {
        FILE* output_file = fopen(output_filename, "w");
        if (output_file != NULL) {
            field_print(&context.field, output_file);
            fclose(output_file);
            printf("Final state saved to '%s'\n", output_filename);
        } else {
            printf("Error: Cannot create output file '%s'\n", output_filename);
        }
    }
    
    if (context.error_occurred) {
        return 1;
    }
    
    return 0;
}