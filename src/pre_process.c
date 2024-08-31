#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/const.h"

char **pre_process(FILE *fp)
{
    static char *result[MAX_LINES];
    char buffer[MAX_CHARS]; // 用于读取每行的缓冲区
    int line_number = 0;

    // 逐行读取文件内容
    while (fgets(buffer, MAX_CHARS, fp) != NULL && line_number < MAX_LINES)
    {
        for (int i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] == '#')
            {
                buffer[i] = '\0';
                break;
            }
        }
        if (strlen(buffer) == 0)
        {
            continue;
        }
        result[line_number] = (char *)malloc(strlen(buffer) + 1); // 为每行分配内存
        if (result[line_number] == NULL)
        {
            perror("Error allocating memory");
        }
        strcpy(result[line_number], buffer); // 将缓冲区内容复制到字符串数组中
        line_number++;
    }
    return result;
}