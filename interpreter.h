#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "field.h"
#include "parser.h"

// Контекст интерпретатора
typedef struct {
    Field field;
    int field_initialized;
    int dino_placed;
    int error_occurred;
    char error_message[256];
    int display_enabled;
    int save_enabled;
    double display_interval;
} InterpreterContext;

// Функции интерпретатора
void interpreter_init(InterpreterContext* context);
int interpreter_execute_command(InterpreterContext* context, ParsedCommand* cmd);
void interpreter_set_display_options(InterpreterContext* context, int enabled, double interval);
void interpreter_set_save_option(InterpreterContext* context, int enabled);
const char* interpreter_get_error_message(InterpreterContext* context);

#endif