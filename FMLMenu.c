#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>

int main() {
    char input[256];
    char *first, *second, *third;

    printf("FML Command Parser (type quit to exit)\n");

    while (1) {
        printf("FML> ");
        fgets(input, 256, stdin);

        if (input[strlen(input)-1] == '\n') {
            input[strlen(input)-1] = '\0';
        }

        first = strtok(input, " ");
        second = strtok(NULL, " ");
        third = strtok(NULL, " ");

        if (first == NULL) {
            continue;
        }
    }

    return 0;
}
