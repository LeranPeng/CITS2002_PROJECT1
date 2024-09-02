//  CITS2002 Project 1 2024
//  Student1: 23909531 Leran Peng
//  Student2: 24022534 Runzhi Zhao
//  Platform: Apple

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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
    char line[256];

    fprintf(output, "#include <stdio.h>\n");
    fprintf(output, "#include <stdlib.h>\n");
    fprintf(output, "#include <ctype.h>\n");
    fprintf(output, "#include <string.h>\n");
    fprintf(output,"\n");

    fprintf(output, "int main() {\n");

    while (fgets(line, sizeof(line), input)) {

        char *trimmed_line = strtok(line, "\n");


        if (trimmed_line[0] == '#') {
            continue;
        }


        char *assign_op = strstr(trimmed_line, "<-");
        if (assign_op != NULL) {
            *assign_op = '\0';  // 将字符串从 '<--' 处分割
            char *identifier = trimmed_line;
            char *expression = assign_op + 3;
            fprintf(output, "    double %s = %s;\n", identifier, expression);
        }

        else if (strncmp(trimmed_line, "print", 5) == 0) {
            char expression[200];
            sscanf(trimmed_line, "print %[^\n]", expression);
            fprintf(output, "    printf(\"%%lf\\n\", %s);\n", expression);
        }

        else if (strncmp(trimmed_line, "function", 8) == 0) {
            char func_name[13];
            char params[100];
            sscanf(trimmed_line, "function %12s (%[^)])", func_name, params);
            fprintf(output, "double %s(%s) {\n", func_name, params);
        }

        else if (strncmp(trimmed_line, "return", 6) == 0) {
            char expression[200];
            sscanf(trimmed_line, "return %[^\n]", expression);
            fprintf(output, "    return %s;\n", expression);
        }

        else if (strchr(trimmed_line, '(') != NULL && strchr(trimmed_line, ')') != NULL) {
            fprintf(output, "    %s;\n", trimmed_line);
        }

        else if (trimmed_line[0] == '\0') {
            fprintf(output, "}\n");
        }
    }

    fprintf(output, "    return 0;\n");
    fprintf(output, "}\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_ml_file> <output_c_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    FILE *output = fopen(argv[2], "w");
    if (output == NULL) {
        perror("Error opening output file");
        fclose(input);
        exit(EXIT_FAILURE);
    }

    translate_ml_to_c(input, output);

    fflush(output);
    fclose(input);
    fclose(output);

    printf("Translation complete. Output written to %s\n", argv[2]);

    return 0;
}

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
