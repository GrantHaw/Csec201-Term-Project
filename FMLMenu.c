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
        //quit
        if (strcmp(first, "quit") == 0) {
            printf("Exiting FML.\n");
            break;
        }

        // upload
        if (strcmp(first, "upload") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: upload <local filename> <remote filename>\n");
            } else {
                printf("Valid command: upload %s %s\n", second, third);
            }
            continue;
        }

        // download
        if (strcmp(first, "download") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: download <remote filename> <local filename>\n");
            } else {
                printf("Valid command: download %s %s\n", second, third);
            }
            continue;
        }

    }

    return 0;
}
