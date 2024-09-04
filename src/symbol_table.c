#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../includes/const.h"

typedef enum
{
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_FUNCTION,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_TAB,
    TOKEN_FEED,
    TOKEN_UNKNOWN,
} TokenType;

typedef struct Symbol
{
    char name[MAX_IDENTIFIER_LENGTH]; // identifier name
    char type[16];                    // identifier type (variable/function/parameter)
    // char data_type[8];                // data type (int/float)
    char scope[MAX_IDENTIFIER_LENGTH]; // scope (global/local)
} Symbol;

typedef struct Symbol_Table
{
    Symbol table[MAX_SYMBOLS];
    int count;
} Symbol_Table;

Symbol_Table symbol_table;
char current_function[MAX_IDENTIFIER_LENGTH];

int is_conflict(const char *name, const char *type)
{
    for (int i = 0; i < symbol_table.count; i++)
    {
        if (strcmp(symbol_table.table[i].name, name) == 0)
        {
            if (strcmp(symbol_table.table[i].type, type) != 0 &&
                (strcmp(symbol_table.table[i].type, "function") == 0 || strcmp(type, "function") == 0))
            {
                printf("Error: Symbol table is full.\n");
                exit(1);
            }
            return 1;
        }
    }
    return 0;
}

void add_symbol(const char *name, const char *type, const char *scope)
{
    if (!is_conflict(name, type))
    {
        if (symbol_table.count < MAX_SYMBOLS)
        {
            strncpy(symbol_table.table[symbol_table.count].name, name, sizeof(symbol_table.table[symbol_table.count].name) - 1);
            strncpy(symbol_table.table[symbol_table.count].type, type, sizeof(symbol_table.table[symbol_table.count].type) - 1);
            // strncpy(symbol_table.table[symbol_table.count].data_type, data_type, sizeof(symbol_table.table[symbol_table.count].data_type) - 1);
            strncpy(symbol_table.table[symbol_table.count].scope, scope, sizeof(symbol_table.table[symbol_table.count].scope) - 1);
            symbol_table.count++;
        }
        else
        {
            printf("Error: Symbol table is full.\n");
            exit(1);
        }
    }
}

void print_symbol_table()
{
    printf("%-10s %-10s %-10s\n", "Name", "Type", "Scope");
    printf("----------------------------------------\n");
    for (int i = 0; i < symbol_table.count; i++)
    {
        printf("%-10s %-10s %-10s\n", symbol_table.table[i].name, symbol_table.table[i].type,
               symbol_table.table[i].scope);
    }
}

void splitString(const char *str, char *before_comma, char *after_comma)
{
    // find comma position
    const char *comma_pos = strchr(str, ',');

    if (comma_pos != NULL)
    {
        // get content before comma
        size_t before_len = comma_pos - str;
        strncpy(before_comma, str, before_len);
        before_comma[before_len] = '\0';

        // get content after comma
        strcpy(after_comma, comma_pos + 1);
    }
    else
    {
        strcpy(before_comma, str);
        after_comma[0] = '\0';
    }
}

void process_line(char *line)
{
    // TODO: Test Code
    // printf("%s\n", line);

    char *token_ptr;
    char *token = strtok_r(line, "[]", &token_ptr);
    // if first char is '\t', it's Internal Function Statements.
    if (strcmp(token, "13,") != 0)
    {
        strcpy(current_function, "global");
    }
    while (token != NULL)
    {
        char token_type[3];
        char token_value[MAX_IDENTIFIER_LENGTH];
        splitString(token, token_type, token_value);
        if (atoi(token_type) == TOKEN_FUNCTION)
        {
            token = strtok_r(NULL, "[]", &token_ptr);
            splitString(token, token_type, token_value);
            add_symbol(token_value, "function", "global");
            strcpy(current_function, token_value);
            token = strtok_r(NULL, "[]", &token_ptr);
            while (strcmp(token, "14,") != 0)
            {
                splitString(token, token_type, token_value);
                add_symbol(token_value, "parameter", current_function);
                token = strtok_r(NULL, "[]", &token_ptr);
            }
        }
        else if (atoi(token_type) == TOKEN_IDENTIFIER)
        {
            splitString(token, token_type, token_value);
            token = strtok_r(NULL, "[]", &token_ptr);
            char token_type_tmp[3];
            char token_value_tmp[MAX_IDENTIFIER_LENGTH];
            splitString(token, token_type_tmp, token_value_tmp);
            if (atoi(token_type_tmp) != TOKEN_LPAREN)
            {
                add_symbol(token_value, "identifier", current_function);
            }
        }
        token = strtok_r(NULL, "[]", &token_ptr);
    }
}

void symbol_table_start(char *input_lines)
{
    int num_lines = 0;
    symbol_table.count = 0;
    // Process every line, build symbol table
    char *line_ptr;
    char *line = strtok_r(input_lines, "\n", &line_ptr);
    while (line != NULL)
    {
        process_line(line);
        line = strtok_r(NULL, "\n", &line_ptr);
        num_lines++;
    }

    // TODO: Test Code
    // print_symbol_table();
    printf("Symbols check completed successfully!\n");
}