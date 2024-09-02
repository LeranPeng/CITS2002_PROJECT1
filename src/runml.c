//  CITS2002 Project 1 2024
//  Student1: 23909531 Leran Peng
//  Student2: 24022534 Runzhi Zhao
//  Platform: Apple

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "../includes/pre_process.h"
#include "../includes/parser.h"

#define MAX_LINE_LENGTH 256
#define MAX_IDENTIFIERS 50

typedef struct {
    char name[13];
    int initialized;
} Identifier;

Identifier identifiers[MAX_IDENTIFIERS];
int identifier_count = 0;


// Function to trim leading and trailing whitespace from a string
char *trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end+1) = 0;

    return str;
}

// Function to find or add an identifier
int find_or_add_identifier(const char *name) {
    for (int i = 0; i < identifier_count; i++) {
        if (strcmp(identifiers[i].name, name) == 0) {
            return i;
        }
    }
    if (identifier_count < MAX_IDENTIFIERS) {
        strncpy(identifiers[identifier_count].name, name, 12);
        identifiers[identifier_count].name[12] = '\0';
        identifiers[identifier_count].initialized = 0;
        return identifier_count++;
    }
    return -1; // Error: too many identifiers
}

// Function to handle expressions
void translate_expression(char *expression, FILE *output) {
    char *token = strtok(expression, " ");
    while (token != NULL) {
        int id = find_or_add_identifier(token);
        if (id != -1 && isalpha(token[0])) {
            if (!identifiers[id].initialized) {
                fprintf(output, "    double %s = 0.0;\n", identifiers[id].name);
                identifiers[id].initialized = 1;
            }
            fprintf(output, "%s ", identifiers[id].name);
        } else {
            fprintf(output, "%s ", token);
        }
        token = strtok(NULL, " ");
    }
}

// Function to translate ML code to C11
void translate_ml_to_c(FILE *input, FILE *output) {
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), input)) {
        char *trimmed_line = trim_whitespace(line);

        // Check for function definition
        if (strncmp(trimmed_line, "function", 8) == 0) {
            char func_name[13];
            char params[100];
            sscanf(trimmed_line, "function %12s (%[^)])", func_name, params);

            fprintf(output, "double %s(%s) {\n", func_name, params);
        }
            
        // Check for assignment statement
        else if (strstr(trimmed_line, "<--") != NULL) {
            char identifier[13];
            char expression[200];
            sscanf(trimmed_line, "%12s <-- %[^\n]", identifier, expression);

            int id = find_or_add_identifier(identifier);
            if (id != -1 && !identifiers[id].initialized) {
                fprintf(output, "    double %s = 0.0;\n", identifiers[id].name);
                identifiers[id].initialized = 1;
            }

            fprintf(output, "    %s = ", identifier);
            translate_expression(expression, output);
            fprintf(output, ";\n");
        }
        // Check for print statement
        else if (strncmp(trimmed_line, "print", 5) == 0) {
            char expression[200];
            sscanf(trimmed_line, "print %[^\n]", expression);

            fprintf(output, "    printf(\"%%lf\\n\", ");
            translate_expression(expression, output);
            fprintf(output, ");\n");
        }
        // Check for return statement
        else if (strncmp(trimmed_line, "return", 6) == 0) {
            char expression[200];
            sscanf(trimmed_line, "return %[^\n]", expression);

            fprintf(output, "    return ");
            translate_expression(expression, output);
            fprintf(output, ";\n");
        }
        // Check for function call
        else if (strchr(trimmed_line, '(') != NULL && strchr(trimmed_line, ')') != NULL) {
            fprintf(output, "    %s;\n", trimmed_line);
        }
        // Close function definition
        else if (trimmed_line[0] == '\0') {
            fprintf(output, "}\n");
        }
    }
}





int main(int argc, char *argv[])
{
    if (argc != 3) {
        print_usage(argv[0]);
        exit(EXIT_FAILURES);
    }

    
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        fprintf(stderr, "Error: Cannot open input file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    FILE *output = fopen(argv[2], "w");
    if (output == NULL) {
        fprintf(stderr, "Error: Cannot open output file %s\n", argv[2]);
        fclose(input);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, ".ml file open successfully\n");
    translate_ml_to_c(input, output);
    fclose(input);   
    fprintf(stdout, "Translation complete. Output written to %s\n", argv[2]);

    fclose(output); // should be changed. compile the output in c11.

    exit(EXIT_SUCCESS);


        //char **result_pre_process;
        //result_pre_process = pre_process(ml_file);
        // TODO: Delete Test Code
        // for (int j = 0; j < 10; j++)
        // {
        //     printf("Line %d: %s", j + 1, result_pre_process[j]);
        //     free(result_pre_process[j]);
        // }
        //parser(result_pre_process);
        // TODO: Free space after all done
    }
}
