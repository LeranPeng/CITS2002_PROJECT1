//  CITS2002 Project 1 2024
//  Student1: 23909531 Leran Peng
//  Student2: 24022534 Runzhi Zhao
//  Platform: Apple

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "includes/pre_process.h"
#include "includes/symbol_table.h"
#include "includes/parser.h"
#include "symbol_table.c"
#include "pre_process.c"


char c_code[MAX_LENGTH];
char function_definitions[MAX_LENGTH];


int is_integer(const char *num_str) {
    char *end_ptr;
    double num = strtod(num_str, &end_ptr);
    return (num == (int)num);
}


void generate_c_code(const char *token_type, const char *token_value, int is_in_function) {
    if (strcmp(token_type, "2") == 0) { // TOKEN_NUMBER
        // 如果是数字类型，判断是整数还是小数，并相应地格式化
        if (is_integer(token_value)) {
            sprintf(c_code + strlen(c_code), "%d", atoi(token_value));  // 整数
        } else {
            sprintf(c_code + strlen(c_code), "%.6f", atof(token_value));  // 小数
        }
    } else if (strcmp(token_type, "0") == 0) { // TOKEN_IDENTIFIER
        sprintf(c_code + strlen(c_code), "%s", token_value);
    } else if (strcmp(token_type, "5") == 0) { // TOKEN_ASSIGN
        sprintf(c_code + strlen(c_code), " = ");
    } else if (strcmp(token_type, "6") == 0) { // TOKEN_PLUS
        sprintf(c_code + strlen(c_code), " + ");
    } else if (strcmp(token_type, "7") == 0) { // TOKEN_MINUS
        sprintf(c_code + strlen(c_code), " - ");
    } else if (strcmp(token_type, "8") == 0) { // TOKEN_MULTIPLY
        sprintf(c_code + strlen(c_code), " * ");
    } else if (strcmp(token_type, "9") == 0) { // TOKEN_DIVIDE
        sprintf(c_code + strlen(c_code), " / ");
    } else if (strcmp(token_type, "11") == 0) { // TOKEN_LPAREN
        sprintf(c_code + strlen(c_code), "(");
    } else if (strcmp(token_type, "12") == 0) { // TOKEN_RPAREN
        sprintf(c_code + strlen(c_code), ")");
    } else if (strcmp(token_type, "3") == 0) { // TOKEN_RETURN
        if (is_in_function) {
            sprintf(function_definitions + strlen(function_definitions), "return ");
        } else {
            sprintf(c_code + strlen(c_code), "return ");
        }
    } else if (strcmp(token_type, "4") == 0) { // TOKEN_PRINT
        // print 函数，处理整数和浮点数的不同打印
        sprintf(c_code + strlen(c_code), "if (is_integer(");
        sprintf(c_code + strlen(c_code), "%s)) { printf(\"%%d\", ", token_value);
        sprintf(c_code + strlen(c_code), "%s); } else { printf(\"%%.6f\", ", token_value);
        sprintf(c_code + strlen(c_code), "%s); }", token_value);
    } else if (strcmp(token_type, "13") == 0) { // TOKEN_COMMA
        sprintf(c_code + strlen(c_code), ", ");
    } else if (strcmp(token_type, "14") == 0) { // TOKEN_TAB
        if (is_in_function) {
            sprintf(function_definitions + strlen(function_definitions), "    "); // translate tab to indentation
        } else {
            sprintf(c_code + strlen(c_code), "    "); // translate tab to indentation
        }
    }
}


void process_line_for_c(char *line, int is_in_function)
{
    char *token_ptr;
    char *token = strtok_r(line, "[]", &token_ptr);

    while (token != NULL)
    {
        char token_type[3];
        char token_value[MAX_IDENTIFIER_LENGTH];
        splitString(token, token_type, token_value);

        generate_c_code(token_type, token_value, is_in_function);

        token = strtok_r(NULL, "[]", &token_ptr);
    }
}



char *translate_to_c(char *input_lines)
{
    // 初始化生成的 C 代码
    strcpy(c_code, "#include <stdio.h>\n\n");

    // 函数定义部分单独生成
    strcpy(function_definitions, "");

    char *line_ptr;
    char *line = strtok_r(input_lines, "\n", &line_ptr);
    int is_in_function = 0;


    // 遍历每一行
    while (line != NULL)
    {
        // 判断是否是函数定义
        if (strstr(line, "function") != NULL)
        {
            is_in_function = 1;
            sprintf(function_definitions + strlen(function_definitions), "void ");
        }

        // 处理每一行 token
        process_line_for_c(line, is_in_function);

        // 如果是函数结束，停止处理函数内容
        if (is_in_function && strstr(line, "return") != NULL)
        {
            sprintf(function_definitions + strlen(function_definitions), ";\n}\n");
            is_in_function = 0;
        }

        if (!is_in_function) {
            sprintf(c_code + strlen(c_code), ";\n"); //add comma
        }

        line = strtok_r(NULL, "\n", &line_ptr);
    }

    // 结束 C 代码
    sprintf(c_code + strlen(c_code), "int main() {\n");
    sprintf(c_code + strlen(c_code), "    // Main program\n");
    sprintf(c_code + strlen(c_code), "    return 0;\n}\n");

    // 将函数定义部分拼接到 C 代码的头部
    strcat(function_definitions, c_code);

    printf("C Code Translation Completed!\n");
    return function_definitions;
}



int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input_ml_file> [arguments_to_be_passed]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        fprintf(stderr, ".ml file open error\n");
        exit(EXIT_FAILURE);
    }

    char **result_pre_process = pre_process(input);

    char *tokens = parser(result_pre_process);
    symbol_table_start(tokens);

    FILE *output = fopen(argv[2], "w");
    if (output == NULL)
    {
        perror("Error opening output file");
        fclose(input);
        exit(EXIT_FAILURE);
    }

    fflush(output);
    fclose(input);
    fclose(output);

    printf("Translation complete. Output written to %s\n", argv[2]);

    // TODO: Free space after all done
    return 0;
}
