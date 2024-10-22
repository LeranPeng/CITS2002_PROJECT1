//  CITS2002 Project 1 2024
//  Platform: Linux

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_LINES 100                      // Maximum number of lines that can be processed
#define MAX_CHARS 256                      // Maximum characters per line
#define MAX_LENGTH (MAX_LINES * MAX_CHARS) // Maximum total length of all lines
#define MAX_SYMBOLS 50                     // Maximum number of symbols in the symbol table
#define MAX_IDENTIFIER_LENGTH 13           // Maximum length of identifiers

/*---Structure Definition START---*/
// Enumeration of different token types used in parsing
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
    TOKEN_NEGATIVE,
    TOKEN_UNKNOWN,
} TokenType;

// Token structure to hold token type and value
typedef struct Token
{
    TokenType type;
    char value[MAX_IDENTIFIER_LENGTH];
} Token;

// Array of string representations for token types
char *TypeName[15] = {
    "identifier",
    "number",
    "function",
    "return",
    "print",
    "assign",
    "plus",
    "minus",
    "multiply",
    "divide",
    "left-bracket",
    "right-bracket",
    "comma",
    "Tab",
    "feed"};

// Structure to represent symbols in the symbol table
typedef struct Symbol
{
    char name[MAX_IDENTIFIER_LENGTH];  // identifier name
    char type[16];                     // identifier type (variable-assign/variable/function/parameter)
    char scope[MAX_IDENTIFIER_LENGTH]; // scope (global/local)
} Symbol;

// Structure for the symbol table holding symbols and their count
typedef struct Symbol_Table
{
    Symbol table[MAX_SYMBOLS];
    int count;
} Symbol_Table;

// Structure for representing functions during translation
typedef struct Function
{
    char type[8]; // function type (float/void)
    char name[MAX_IDENTIFIER_LENGTH];
    char parameters[MAX_SYMBOLS][MAX_IDENTIFIER_LENGTH];
    int parameter_count;
} Function;
/*---Structure Definition END---*/

/*---Global Variable Declare START---*/
// save all tokens
char tokens[MAX_LENGTH];
// Current Token - During Parse
Token current_token = {TOKEN_UNKNOWN, ""};
// Previous Token - During Parse
Token previous_token;
// Point to Parser origin_input - During Parse
const char *origin_p = NULL;
// save all symbols
Symbol_Table symbol_table;
// Current Function - During symbol table check
char current_function[MAX_IDENTIFIER_LENGTH];
// Function Declare part code
char code_function_part[MAX_LENGTH] = "";
// Temp Function Declare part code
char code_function_part_buffer[MAX_LENGTH - MAX_CHARS] = "";
// Main part code
char code_main_part[MAX_LENGTH] = "";
// Current Function - During translating
Function translating_function = {"", "", {""}, 0};
/*---Global Variable Declare END---*/

// Preprocess input files, stripping comments and checking format
char **pre_process(FILE *fp)
{
    static char *result[MAX_LINES];
    char buffer[MAX_CHARS]; // buffer for read lines
    int line_number = 0;

    // read file contents
    while (fgets(buffer, MAX_CHARS, fp) != NULL && line_number < MAX_LINES)
    {
        for (int i = 0; i < strlen(buffer) - 1; i++)
        {
            if (buffer[i] == '#')
            {
                buffer[i] = '\n';
                buffer[i + 1] = '\0';
                break;
            }
        }
        if (strspn(buffer, " \t\r\n") == strlen(buffer)) // Determine if it is empty line
        {
            continue;
        }
        result[line_number] = (char *)malloc(strlen(buffer) + 1); // allocate memory
        if (result[line_number] == NULL)
        {
            fprintf(stderr, "!Error allocating memory");
        }
        strcpy(result[line_number], buffer); // Copy buffer content to result array
        line_number++;
    }
    // Check if the last character in the file is a newline
    if (line_number > 0 && result[line_number - 1][strlen(result[line_number - 1]) - 1] != '\n')
    {
        // Append newline to the last line
        result[line_number - 1] = (char *)realloc(result[line_number - 1], strlen(result[line_number - 1]) + 2);
        if (result[line_number - 1] == NULL)
        {
            fprintf(stderr, "!Error reallocating memory");
        }
        strcat(result[line_number - 1], "\n");
    }
    return result;
}

// Parser
Token get_next_token(const char **origin_p)
{
    // skip whitespace
    while (**origin_p && isspace(**origin_p))
    {
        Token token = {TOKEN_UNKNOWN, ""}; // 默认类型为 TOKEN_OTHER
        if (**origin_p == '\n')
        {
            token.type = TOKEN_FEED;
        }
        else if (**origin_p == '\t')
        {
            token.type = TOKEN_TAB;
        }
        (*origin_p)++;
        if (token.type != TOKEN_UNKNOWN)
        {
            return token;
        }
    }

    // Parse numbers (including floating point and negative number)
    if (isdigit(**origin_p))
    {
        Token token = {TOKEN_NUMBER, ""};
        int i = 0;
        int has_dot = 0;
        // Parse the digits and optional decimal point
        while (isdigit(**origin_p) || (!has_dot && **origin_p == '.'))
        {
            if (**origin_p == '.')
            {
                has_dot = 1;
            }
            token.value[i++] = *(*origin_p)++;
        }
        token.value[i] = '\0';
        return token;
    }

    // Parse identifiers
    if (isalpha(**origin_p))
    {
        Token token = {TOKEN_IDENTIFIER, ""};
        int i = 0;
        while (isalnum(**origin_p))
        {
            if (i >= 12)
            {
                fprintf(stderr, "!Syntax error: Variable length is more than 12 characters!");
                exit(1);
            }
            token.value[i++] = *(*origin_p)++;
        }
        token.value[i] = '\0';
        if (strcmp(token.value, "function") == 0)
            return (Token){TOKEN_FUNCTION, "function"};
        if (strcmp(token.value, "return") == 0)
            return (Token){TOKEN_RETURN, "return"};
        if (strcmp(token.value, "print") == 0)
            return (Token){TOKEN_PRINT, "print"};
        return token;
    }

    // Parse operators and punctuators
    switch (*(*origin_p)++)
    {
    case '<':
        if (**origin_p == '-')
        {
            (*origin_p)++;
            return (Token){TOKEN_ASSIGN, "<-"};
        }
        return (Token){TOKEN_UNKNOWN, ""};
    case '+':
        return (Token){TOKEN_PLUS, "+"};
    case '-':
        if (previous_token.type != TOKEN_NUMBER &&
            previous_token.type != TOKEN_IDENTIFIER &&
            previous_token.type != TOKEN_RPAREN)
        {
            return (Token){TOKEN_NEGATIVE, "-"};
        }
        return (Token){TOKEN_MINUS, "-"};
    case '*':
        return (Token){TOKEN_MULTIPLY, "*"};
    case '/':
        return (Token){TOKEN_DIVIDE, "/"};
    case '(':
        return (Token){TOKEN_LPAREN, "("};
    case ')':
        return (Token){TOKEN_RPAREN, ")"};
    case ',':
        return (Token){TOKEN_COMMA, ","};
    default:
        return (Token){TOKEN_UNKNOWN, ""};
    }
}

// Move to the next token in the input
void next_token()
{
    previous_token = current_token;
    current_token = get_next_token(&origin_p);
    if (current_token.type != TOKEN_UNKNOWN)
    {
        sprintf(tokens + strlen(tokens), "[%d,%s]", current_token.type, current_token.value);
        if (current_token.type == TOKEN_FEED)
        {
            sprintf(tokens + strlen(tokens), "\n");
        }
    }
}

// Ensure the current token matches the expected token type and move to the next token
void match(TokenType expected)
{
    if (current_token.type == expected)
    {
        next_token();
    }
    else
    {
        fprintf(stderr, "!Syntax error: expected %s\n", TypeName[expected]);
        exit(1);
    }
}

// Recursively match zero or more tokens of a given type
void match_zero_or_more(TokenType expected)
{
    if (current_token.type == expected)
    {
        next_token();
        match_zero_or_more(expected);
    }
}

void expression();              // declare a function in advance
void function_call_parameter(); // declare a function in advance

int factor()
{
    if (current_token.type == TOKEN_NEGATIVE)
    {
        next_token();
        if (factor() == -1) // Recursively handle the factor after the negative sign
        {
            fprintf(stderr, "!Syntax error in expression (after negative sign)\n");
            exit(1);
        }
    }
    else if (current_token.type == TOKEN_IDENTIFIER)
    {
        next_token();
        // function call
        if (current_token.type == TOKEN_LPAREN)
        {
            next_token();
            function_call_parameter();
        }
    }
    else if (current_token.type == TOKEN_NUMBER)
    {
        next_token();
    }
    else if (current_token.type == TOKEN_LPAREN)
    {
        next_token();
        expression();
        match(TOKEN_RPAREN);
    }
    else
    {
        return -1;
    }
    return 0;
}

void term()
{
    if (factor() == -1)
    {
        fprintf(stderr, "!Syntax error in expression\n");
        exit(1);
    }
    if (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE)
    {
        next_token();
        term();
    }
}

void expression()
{
    term();
    if (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS)
    {
        next_token();
        expression();
    }
}

void function_call_parameter()
{
    if (current_token.type == TOKEN_RPAREN)
    {
        next_token();
    }
    else
    {
        expression();
        while (current_token.type == TOKEN_COMMA)
        {
            next_token();
            expression();
        }
        match(TOKEN_RPAREN);
    }
}

// Parse a statement
void statement()
{
    if (current_token.type == TOKEN_IDENTIFIER)
    {
        next_token();
        // function call
        if (current_token.type == TOKEN_LPAREN)
        {
            next_token();
            function_call_parameter();
        }
        else
        {
            match(TOKEN_ASSIGN);
            expression();
        }
    }
    else if (current_token.type == TOKEN_PRINT || current_token.type == TOKEN_RETURN)
    {
        next_token();
        expression();
    }
    else
    {
        fprintf(stderr, "!Syntax error in statement\n");
        exit(1);
    }
    match(TOKEN_FEED);
}

// Parse function or statement
void program_item()
{
    if (current_token.type == TOKEN_FUNCTION)
    {
        next_token();
        match(TOKEN_IDENTIFIER);
        match_zero_or_more(TOKEN_IDENTIFIER);
        match(TOKEN_FEED);
        while (current_token.type == TOKEN_TAB)
        {
            next_token();
            statement();
        }
    }
    else
    {
        statement();
    }
}

// Parse an array of input lines into tokens
char *parser(char **input_lines)
{
    // array splicing
    char splicing_result[MAX_LENGTH] = "";
    for (int i = 0; i < MAX_LINES && input_lines[i] != NULL; i++)
    {
        sprintf(splicing_result + strlen(splicing_result), "%s", input_lines[i]);
    }

    origin_p = splicing_result;
    next_token();
    while (current_token.type != TOKEN_UNKNOWN)
    {
        program_item();
    }
    return tokens;
}

// Symbol table(check conflicts)
int is_conflict(const char *name, const char *type, const char *scope)
{
    if (strstr(name, "arg") != NULL)
    {
        return 1;
    }
    for (int i = 0; i < symbol_table.count; i++)
    {
        if (strcmp(symbol_table.table[i].name, name) == 0)
        {
            const char *existing_type = symbol_table.table[i].type;
            const char *existing_scope = symbol_table.table[i].scope;
            // Function redefinition
            if (strcmp(existing_type, "function") == 0 || strcmp(type, "function") == 0)
            {
                if (strcmp(existing_type, "function") == 0 && strcmp(type, "function") == 0)
                {
                    fprintf(stderr, "!Error: Function '%s' is redefined.\n", name);
                }
                else
                {
                    fprintf(stderr, "!Error: '%s' already defined as '%s' cannot be redefined as '%s'.\n",
                            name, existing_type, type);
                }
                return 1;
            }
            // Parameter redefinition
            if (strcmp(existing_type, "parameter") == 0 && strcmp(type, "parameter") == 0 &&
                strcmp(existing_scope, scope) == 0)
            {
                fprintf(stderr, "!Error: Parameter '%s' is redefined in scope '%s'.\n", name, scope);
                return 1;
            }
            // Variable-assign overrides variable in the same scope
            if (strcmp(existing_scope, scope) == 0)
            {
                if (strcmp(existing_type, "variable") == 0 && strcmp(type, "variable-assign") == 0)
                {
                    strcpy(symbol_table.table[i].type, "variable-assign");
                }
                return 1;
            }
            // Global variable overrides
            if ((strcmp(existing_scope, "global") == 0 && strcmp(type, "variable") == 0) ||
                (strcmp(existing_type, "variable") == 0 && strcmp(scope, "global") == 0))
            {
                strcpy(symbol_table.table[i].scope, "global");
                return 1;
            }
        }
    }
    return 0;
}

// add a new symbol to the symbol table
void add_symbol(const char *name, const char *type, const char *scope)
{
    if (!is_conflict(name, type, scope))
    {
        if (symbol_table.count < MAX_SYMBOLS)
        {
            strncpy(symbol_table.table[symbol_table.count].name, name, sizeof(symbol_table.table[symbol_table.count].name) - 1);
            strncpy(symbol_table.table[symbol_table.count].type, type, sizeof(symbol_table.table[symbol_table.count].type) - 1);
            strncpy(symbol_table.table[symbol_table.count].scope, scope, sizeof(symbol_table.table[symbol_table.count].scope) - 1);
            symbol_table.count++;
        }
        else
        {
            fprintf(stderr, "!Error: Symbol table is full.\n");
            exit(1);
        }
    }
}

// Split a string by a comma
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

/**  reilkay implements' version of the strtok_r function
 * with a reentrant mechanism that explicitly utilizes saveptr to save state.
 */
char *strtok_rk(char *str, const char *delim, char **saveptr)
{
    char *token;
    // If the input string is NULL, continue using the previously saved state
    if (str == NULL)
    {
        str = *saveptr;
    }
    // Skip the leading delimiters in the string
    str += strspn(str, delim);
    // If there are no more characters, return NULL
    if (*str == '\0')
    {
        return NULL;
    }
    // Find the next delimiter
    token = str;
    str = strpbrk(token, delim);
    if (str)
    {
        // If a delimiter is found, replace it with a string terminator '\0'
        *str = '\0';
        *saveptr = str + 1;
    }
    else
    {
        // If no delimiter is found, this is the last token
        *saveptr = NULL;
    }
    return token;
}

/**  reilkay implements' version of the strcat function
 * it returns a static value without modifying the original value passed in.
 */
char *strcat_static_rk(const char *str1, const char *str2)
{
    static char result[MAX_IDENTIFIER_LENGTH + 10]; // Static buffer to hold the result
    // Ensure the buffer is empty
    result[0] = '\0';
    strncpy(result, str1, MAX_IDENTIFIER_LENGTH + 9);
    // Concatenate the second string, ensuring no buffer overflow
    strncat(result, str2, MAX_IDENTIFIER_LENGTH + 9 - strlen(result));
    return result;
}

// Add prefix to avoid c language keywords (such as double||float).
char *add_prefix(char *str)
{
    return strcat_static_rk("_from_ml_", str);
}

// process each line during symbol table building
void process_line(char *line)
{
    char *token_ptr;
    char *token = strtok_rk(line, "[]", &token_ptr);
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
            token = strtok_rk(NULL, "[]", &token_ptr);
            splitString(token, token_type, token_value);
            add_symbol(token_value, "function", "global");
            strcpy(current_function, token_value);
            token = strtok_rk(NULL, "[]", &token_ptr);
            while (strcmp(token, "14,") != 0)
            {
                splitString(token, token_type, token_value);
                add_symbol(token_value, "parameter", current_function);
                token = strtok_rk(NULL, "[]", &token_ptr);
            }
        }
        else if (atoi(token_type) == TOKEN_IDENTIFIER)
        {
            splitString(token, token_type, token_value);
            token = strtok_rk(NULL, "[]", &token_ptr);
            char token_type_tmp[3];
            char token_value_tmp[MAX_IDENTIFIER_LENGTH];
            splitString(token, token_type_tmp, token_value_tmp);
            if (atoi(token_type_tmp) != TOKEN_LPAREN)
            {
                if (atoi(token_type_tmp) == TOKEN_ASSIGN)
                {
                    add_symbol(token_value, "variable-assign", current_function);
                }
                else
                {
                    add_symbol(token_value, "variable", current_function);
                }
            }
        }
        token = strtok_rk(NULL, "[]", &token_ptr);
    }
}

// Building the symbol table from input lines
void symbol_table_start(char *input_lines)
{
    symbol_table.count = 0;
    // Process every line, build symbol table
    char *line_ptr;
    char *line = strtok_rk(input_lines, "\n", &line_ptr);
    while (line != NULL)
    {
        process_line(line);
        line = strtok_rk(NULL, "\n", &line_ptr);
    }
}

// Translate tokens to C code
char *token_to_c_code(char *token_value, int token_type)
{
    static char return_value[MAX_IDENTIFIER_LENGTH + 10] = "";
    switch (token_type)
    {
    case TOKEN_IDENTIFIER:
        strcpy(return_value, add_prefix(token_value));
        break;
    case TOKEN_NUMBER:
        strcpy(return_value, token_value);
        break;
    case TOKEN_RETURN:
        strcpy(return_value, "return ");
        break;
    case TOKEN_ASSIGN:
        strcpy(return_value, " = ");
        break;
    case TOKEN_PLUS:
        strcpy(return_value, " + ");
        break;
    case TOKEN_MINUS:
        strcpy(return_value, " - ");
        break;
    case TOKEN_MULTIPLY:
        strcpy(return_value, " * ");
        break;
    case TOKEN_DIVIDE:
        strcpy(return_value, " / ");
        break;
    case TOKEN_LPAREN:
        strcpy(return_value, "(");
        break;
    case TOKEN_RPAREN:
        strcpy(return_value, ")");
        break;
    case TOKEN_COMMA:
        strcpy(return_value, ", ");
        break;
    case TOKEN_FEED:
        strcpy(return_value, ";\n");
        break;
    case TOKEN_NEGATIVE:
        strcpy(return_value, "-");
        break;
    default:
        break;
    }
    return return_value;
}

// Translate each line to C code based on parsed tokens
void translate_line(char *line)
{
    char *token_ptr;
    char *token = strtok_rk(line, "[]", &token_ptr);

    if (token != NULL)
    {
        char token_type[3];
        char token_value[MAX_IDENTIFIER_LENGTH];
        splitString(token, token_type, token_value);

        // deal with the end part of function
        if (atoi(token_type) != TOKEN_TAB && strcmp(translating_function.name, "") != 0)
        {
            // Output function declaration to code_function_part
            sprintf(code_function_part + strlen(code_function_part), "%s %s(", translating_function.type, add_prefix(translating_function.name));
            for (int i = 0; i < translating_function.parameter_count; i++)
            {
                sprintf(code_function_part + strlen(code_function_part), "%sfloat %s", i > 0 ? ", " : "", add_prefix(translating_function.parameters[i]));
            }
            sprintf(code_function_part + strlen(code_function_part), ")\n{\n");
            // Process symbol table, local variable declaration
            for (int i = 0; i < symbol_table.count; i++)
            {
                if (strcmp(symbol_table.table[i].scope, translating_function.name) == 0 &&
                    (strcmp(symbol_table.table[i].type, "variable") == 0 ||
                     strcmp(symbol_table.table[i].type, "variable-assign") == 0))
                {
                    // Add variable declaration to code_function_part
                    sprintf(code_function_part + strlen(code_function_part), "\tfloat %s = 0.000000;\n", add_prefix(symbol_table.table[i].name));
                }
            }
            sprintf(code_function_part + strlen(code_function_part), "%s}\n", code_function_part_buffer);
            strcpy(code_function_part_buffer, "");
            memset(&translating_function, 0, sizeof(translating_function)); // clear info in translation process
        }

        // deal with the definition of the function
        if (atoi(token_type) == TOKEN_FUNCTION)
        {
            strcpy(translating_function.type, "void"); // First define the return value as void
            token = strtok_rk(NULL, "[]", &token_ptr);
            splitString(token, token_type, token_value);
            strcpy(translating_function.name, token_value); // get function name
            token = strtok_rk(NULL, "[]", &token_ptr);
            splitString(token, token_type, token_value);
            // Handling function parameters
            int i = 0;
            while (atoi(token_type) != TOKEN_FEED)
            {
                strcpy(translating_function.parameters[i++], token_value);
                translating_function.parameter_count = i;
                token = strtok_rk(NULL, "[]", &token_ptr);
                splitString(token, token_type, token_value);
            }
        }
        // Handling function bodies or other statements
        else
        {
            char *code_part = code_main_part;
            // Handling function bodies
            if (atoi(token_type) == TOKEN_TAB)
            {
                code_part = code_function_part_buffer; // Redirecting output to function_part_buffer
                token = strtok_rk(NULL, "[]", &token_ptr);
            }
            sprintf(code_part + strlen(code_part), "\t"); // Output Tab to beautify the code structure
            do
            {
                splitString(token, token_type, token_value);
                if (atoi(token_type) == TOKEN_PRINT)
                {
                    sprintf(code_part + strlen(code_part), "print_iof(");
                    token = strtok_rk(NULL, "[]", &token_ptr);
                    splitString(token, token_type, token_value);
                    while (atoi(token_type) != TOKEN_FEED)
                    {
                        sprintf(code_part + strlen(code_part), "%s",
                                token_to_c_code(token_value, atoi(token_type)));
                        token = strtok_rk(NULL, "[]", &token_ptr);
                        splitString(token, token_type, token_value);
                    }
                    sprintf(code_part + strlen(code_part), ")");
                }
                else
                {
                    if (atoi(token_type) == TOKEN_RETURN && code_part == code_function_part_buffer)
                    {
                        strcpy(translating_function.type, "float"); // Modify the return type to float
                    }
                    sprintf(code_part + strlen(code_part), "%s",
                            token_to_c_code(token_value, atoi(token_type)));
                    token = strtok_rk(NULL, "[]", &token_ptr);
                }
            } while (token != NULL);
        }
    }
}

void translate_start(char *input_lines, float *args, int args_count)
{
    // Predefined fixed output parts
    char code_header_part[MAX_LENGTH] = "#include<stdio.h>\n"
                                        "void print_iof(float value)\n"
                                        "{\n"
                                        "    if(value == (int)value)\n"
                                        "    {\n"
                                        "        printf(\"%d\\n\", (int)value);\n"
                                        "    }\n"
                                        "    else\n"
                                        "    {\n"
                                        "        printf(\"%.6f\\n\", value);\n"
                                        "    }\n"
                                        "}\n";

    strcpy(code_main_part, "int main()\n"
                           "{\n");

    // Process args table, predefined global variables
    for (int i = 0; i < args_count; i++)
    {
        // Add variable declaration to header part
        sprintf(code_header_part + strlen(code_header_part), "float %s%d = %.6f;\n", add_prefix("arg"), i, args[i]);
    }

    // Process symbol table, predefined global variables
    for (int i = 0; i < symbol_table.count; i++)
    {
        if (strcmp(symbol_table.table[i].scope, "global") == 0 &&
            (strcmp(symbol_table.table[i].type, "variable") == 0 ||
             strcmp(symbol_table.table[i].type, "variable-assign") == 0))
        {
            // Add variable declaration to header part
            sprintf(code_header_part + strlen(code_header_part), "float %s = 0.000000;\n", add_prefix(symbol_table.table[i].name));
        }
    }

    // Process every line, translate code
    char *line_ptr;
    char *line = strtok_rk(input_lines, "\n", &line_ptr);
    while (line != NULL)
    {
        translate_line(line);
        line = strtok_rk(NULL, "\n", &line_ptr);
    }

    // Add end to main part
    sprintf(code_main_part + strlen(code_main_part), "}\n");

    // Open file to write
    FILE *file = fopen("./output_code.c", "w");
    if (file == NULL)
    {
        fprintf(stderr, "!Error: Can not open file to write.\n");
    }

    // Write header part
    fputs(code_header_part, file);
    fputs("\n", file);
    fputs(code_function_part, file);
    fputs("\n", file);
    fputs(code_main_part, file);
    fputs("\n", file);

    fclose(file);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "!Usage: %s <input_ml_file> [arguments_to_be_passed]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Process arguments
    int args_count = argc - 2;
    float args[MAX_SYMBOLS];
    for (int i = 0; i < args_count; i++)
    {
        args[i] = atof(argv[i + 2]); // begin with argv[2]
    }

    // Check ML file
    FILE *input = fopen(argv[1], "r");
    if (input == NULL)
    {
        fprintf(stderr, "!.ml file open error\n");
        exit(EXIT_FAILURE);
    }

    // Pre-process
    char **result_pre_process;
    result_pre_process = pre_process(input);
    fclose(input);

    // Parsing program syntax errors
    char *tokens = parser(result_pre_process);
    fprintf(stdout, "@Parsing completed successfully!\n");

    // Check the program symbol table
    char tokens_cp1[MAX_LENGTH];
    strcpy(tokens_cp1, tokens);
    symbol_table_start(tokens_cp1);
    fprintf(stdout, "@Symbols check completed successfully!\n");

    // Start translate to C code
    char tokens_cp2[MAX_LENGTH];
    strcpy(tokens_cp2, tokens);
    translate_start(tokens_cp2, args, args_count);
    fprintf(stdout, "@Translate completed successfully!\n");

    // Run gcc to compile the C code
    if (system("cc -std=c11 -o output_program output_code.c") != 0)
    {
        fprintf(stderr, "!Compilation failed.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "@Compilation succeeded.\n");

    // Run program binary
    if (system("./output_program") != 0)
    {
        fprintf(stderr, "\n!Program execution failed.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(stdout, "\n");
    }

    // Free the memory allocated by pre_process
    for (int i = 0; i < MAX_LINES && result_pre_process[i] != NULL; i++)
    {
        free(result_pre_process[i]);
    }

    // Free space, delete files after all done
    remove("output_code.c");
    remove("output_program");
    return 0;
}
