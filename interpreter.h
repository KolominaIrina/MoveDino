#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "field.h"
#include "parser.h"

#define MAX_UNDO_LEVELS 20  // Максимальное количество уровней отката

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
    
    // UNDO - система отката действий
    Field history[MAX_UNDO_LEVELS]; // История состояний поля
    int history_size;               // Текущий размер истории
    int current_history_index;      // Текущий индекс в истории
    
    // Предупреждения
    char warning_message[256];      // Текст предупреждения
    int has_warning;                // Флаг наличия предупреждения
    
    // Вложенные файлы
    char current_filename[256];     // Текущий исполняемый файл
    int exec_depth;                 // Глубина вложенности EXEC (защита от бесконечной рекурсии)
    
} InterpreterContext;

// Функции интерпретатора
void interpreter_init(InterpreterContext* context); // инициализация контекста интерпретатора (обнуление, выделение памяти)
int interpreter_execute_command(InterpreterContext* context, ParsedCommand* cmd, int line_number); // выполнение одной команды (cmd - распознанная команда, line_number - для кодов ошибок)
int interpreter_execute_file(InterpreterContext* context, const char* filename); // выполнение всех команд из файла (команда EXEC)
int interpreter_execute_if_command(InterpreterContext* context, ParsedCommand* cmd, int line_number); // обработка условной команды IF
void interpreter_save_state(InterpreterContext* context);
int interpreter_undo(InterpreterContext* context);
void interpreter_set_display_options(InterpreterContext* context, int enabled, double interval); // настройки отображения
void interpreter_set_save_option(InterpreterContext* context, int enabled); // вкл/выкл сохранения результата в файл
const char* interpreter_get_error_message(InterpreterContext* context);

// Функции для работы с предупреждениями
void interpreter_set_warning(InterpreterContext* context, const char* warning);
void interpreter_show_warnings(InterpreterContext* context);

#endif