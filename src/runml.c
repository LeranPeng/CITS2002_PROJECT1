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
#include "../includes/symbol_table.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <ml_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *ml_filename = argv[1];
    FILE *ml_file = fopen(ml_filename, "r");

    if (ml_file == NULL)
    {
        fprintf(stderr, ".ml file open error\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(stdout, ".ml file open successfully\n");
        char **result_pre_process;
        result_pre_process = pre_process(ml_file);
        // TODO: Delete Test Code
        // for (int j = 0; j < 10; j++)
        // {
        //     printf("Line %d: %s", j + 1, result_pre_process[j]);
        //     free(result_pre_process[j]);
        // }
        char *tokens = parser(result_pre_process);
        symbol_table_start(tokens);
        // TODO: Free space after all done
    }
}