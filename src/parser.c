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
    TOKEN_UNKNOWN,
} TokenType;

typedef struct
{
    TokenType type;
    char value[256];
} Token;

char *TypeName[14] = {
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
    "comma"};

Token get_next_token(const char **origin_p)
{
    // skip space
    while (**origin_p && isspace(**origin_p))
        (*origin_p)++;

    if (isdigit(**origin_p))
    {
        Token token = {TOKEN_NUMBER, ""};
        int i = 0;
        while (isdigit(**origin_p))
        {
            token.value[i++] = *(*origin_p)++;
        }
        return token;
    }

    if (isalpha(**origin_p))
    {
        Token token = {TOKEN_IDENTIFIER, ""};
        int i = 0;
        while (isalnum(**origin_p))
        {
            token.value[i++] = *(*origin_p)++;
        }
        if (strcmp(token.value, "function") == 0)
            return (Token){TOKEN_FUNCTION, "function"};
        if (strcmp(token.value, "return") == 0)
            return (Token){TOKEN_RETURN, "return"};
        if (strcmp(token.value, "print") == 0)
            return (Token){TOKEN_PRINT, "print"};
        return token;
    }

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

char origin[MAX_LENGTH] = "";
// point to origin
const char *origin_p = NULL;
Token current_token;

void next_token()
{
    current_token = get_next_token(&origin_p);
    // TODO: Delete Test Code
    printf("currentToken:%s\n", current_token.value);
}

void match(TokenType expected)
{
    if (current_token.type == expected)
    {
        next_token();
    }
    else
    {
        printf("Syntax error: expected %s\n", TypeName[expected]);
        exit(1);
    }
}

void match_zero_or_more(TokenType expected)
{
    if (current_token.type == expected)
    {
        next_token();
        match_zero_or_more(expected);
    }
}

void match_zero_or_more_2(TokenType expected1, TokenType expected2)
{
    if (current_token.type == expected1)
    {
        next_token();
        if (current_token.type == expected2)
        {
            next_token();
            match_zero_or_more_2(expected1, expected2);
        }
        else
        {
            printf("Syntax error in parameters: expect %s\n", TypeName[expected2]);
            exit(1);
        }
    }
}

void expression(int can_be_empty);
void function_call();

int factor()
{
    if (current_token.type == TOKEN_IDENTIFIER)
    {
        next_token();
        // function call
        if (current_token.type == TOKEN_LPAREN)
        {
            next_token();
            function_call();
        }
    }
    else if (current_token.type == TOKEN_NUMBER)
    {
        next_token();
    }
    else if (current_token.type == TOKEN_LPAREN)
    {
        expression(0);
        match(TOKEN_RPAREN);
    }
    else
    {
        return -1;
    }
    return 0;
}

void term(int can_be_empty)
{
    if (factor() == -1 && !can_be_empty)
    {
        printf("Syntax error in expression\n");
        exit(1);
    }
    if (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE)
    {
        next_token();
        term(0);
    }
}

void expression(int can_be_empty)
{
    term(can_be_empty);
    if (current_token.type == TOKEN_PLUS || current_token.type == TOKEN_MINUS)
    {
        next_token();
        expression(0);
    }
}

void function_call()
{
    expression(1);
    if (current_token.type == TOKEN_IDENTIFIER)
    {
        next_token();
        match_zero_or_more_2(TOKEN_COMMA, TOKEN_IDENTIFIER);
    }
    match(TOKEN_RPAREN);
}

void statement()
{
    if (current_token.type == TOKEN_IDENTIFIER)
    {
        next_token();
        // function call
        if (current_token.type == TOKEN_LPAREN)
        {
            next_token();
            function_call();
        }
        else
        {
            match(TOKEN_ASSIGN);
            expression(0);
        }
    }
    else if (current_token.type == TOKEN_PRINT || current_token.type == TOKEN_RETURN)
    {
        next_token();
        expression(0);
    }
    else
    {
        printf("Syntax error in statement\n");
        exit(1);
    }
}

void program_item()
{
    if (current_token.type == TOKEN_FUNCTION)
    {
        next_token();
        match(TOKEN_IDENTIFIER);
        match_zero_or_more(TOKEN_IDENTIFIER);
        statement();
    }
    else
    {
        statement();
    }
}

void parser(char **input_lines)
{
    // array splicing
    for (int i = 0; i < MAX_LINES && input_lines[i] != NULL; i++)
    {
        sprintf(origin + strlen(origin), "%s ", input_lines[i]);
    }

    // delete last space
    size_t len = strlen(origin);
    if (len > 0)
    {
        origin[len - 1] = '\0';
    }
    origin_p = origin;
    next_token();
    while (current_token.type != TOKEN_UNKNOWN)
    {
        program_item();
    }
    printf("Parsing completed successfully!\n");
}
