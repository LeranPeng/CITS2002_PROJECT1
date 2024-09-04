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

typedef struct Token
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

char tokens[MAX_LENGTH];
// current Token
Token current_token;
// point to origin_input
const char *origin_p = NULL;

Token get_next_token(const char **origin_p)
{
    // skip space
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

void next_token()
{
    current_token = get_next_token(&origin_p);
    if (current_token.type != TOKEN_UNKNOWN)
    {
        sprintf(tokens + strlen(tokens), "[%d,%s]", current_token.type, current_token.value);
        if (current_token.type == TOKEN_FEED)
        {
            sprintf(tokens + strlen(tokens), "\n");
        }
    }
    // TODO: Delete Test Code
    // printf("currentToken:%d-%s\n", current_token.type, current_token.value);
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

void expression();
void function_call_parameter();

int factor()
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
    }
    else if (current_token.type == TOKEN_NUMBER)
    {
        next_token();
    }
    else if (current_token.type == TOKEN_LPAREN)
    {
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
        printf("Syntax error in expression\n");
        exit(1);
    }
    if (current_token.type == TOKEN_MULTIPLY || current_token.type == TOKEN_DIVIDE)
    {
        next_token();
        term(0);
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
        printf("Syntax error in statement\n");
        exit(1);
    }
    match(TOKEN_FEED);
}

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
    printf("Parsing completed successfully!\n");
    return tokens;
}
