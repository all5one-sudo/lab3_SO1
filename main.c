#include <stdio.h>
#include "memory.h"

int main()
{
    printf("\033[0;34mMalloc First Fit Test\033[0m\n");
    malloc_control(FIRST_FIT);
    char *char1 = NULL;
    char1 = malloc(32);
    strcpy(char1, "str 1");
    check_heap(char1);

    printf("\n\033[0;34mMalloc Best Fit Test\033[0m\n");
    malloc_control(BEST_FIT);
    char *char2 = NULL;
    char2 = malloc(16);
    strcpy(char2, "str 2");
    check_heap(char2);

    printf("\n\033[0;34mCalloc Test\033[0m\n");
    char *char3 = NULL;
    char3 = calloc(64, sizeof(char));
    strcpy(char3, "str 3");
    check_heap(char3);

    printf("\n\033[0;34mRealloc Test\033[0m\n");
    char *char4 = NULL;
    char4 = realloc(char4, 128);
    strcpy(char4, "str 4");
    check_heap(char4);

    free(char1);
    free(char2);
    free(char3);
    free(char4);
    return 0;
}