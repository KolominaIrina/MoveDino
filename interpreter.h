#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "field.h"
#include "parser.h"

#define MAX_UNDO_LEVELS 50  // Максимальное количество уровней отката

// Контекст интерпретатора - хранит состояние выполнения программы
typedef struct {
    Field field;                    // Текущее состояние игрового поля
    int field_initialized;          // Флаг инициализации поля
    int dino_placed;                // Флаг размещения динозавра
    int error_occurred;             // Флаг возникновения ошибки
    char error_message[256];        // Текст последней ошибки
    int display_enabled;            // Включена ли визуализация
    int save_enabled;               // Включено ли сохранение
    double display_interval;        // Интервал между отображениями
    
    // ДЛЯ UNDO - система отката действий
    Field history[MAX_UNDO_LEVELS]; // История состояний поля
    int history_size;               // Текущий размер истории
    int current_history_index;      // Текущий индекс в истории
    
    // ДЛЯ ПРЕДУПРЕЖДЕНИЙ
    char warning_message[256];      // Текст предупреждения
    int has_warning;                // Флаг наличия предупреждения
    
    // ДЛЯ ВЛОЖЕННЫХ ФАЙЛОВ
    char current_filename[256];     // Текущий исполняемый файл
    int exec_depth;                 // Глубина вложенности EXEC
    
} InterpreterContext;

// Прототипы функций интерпретатора
void interpreter_init(InterpreterContext* context);
int interpreter_execute_command(InterpreterContext* context, ParsedCommand* cmd, int line_number);
int interpreter_execute_file(InterpreterContext* context, const char* filename);
int interpreter_execute_if_command(InterpreterContext* context, ParsedCommand* cmd, int line_number);
void interpreter_save_state(InterpreterContext* context);
int interpreter_undo(InterpreterContext* context);
void interpreter_set_display_options(InterpreterContext* context, int enabled, double interval);
void interpreter_set_save_option(InterpreterContext* context, int enabled);
const char* interpreter_get_error_message(InterpreterContext* context);

// Функции для работы с предупреждениями
void interpreter_set_warning(InterpreterContext* context, const char* warning);
void interpreter_show_warnings(InterpreterContext* context);

#endif