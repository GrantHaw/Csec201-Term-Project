#include <stdio.h>
#include <string.h>
#include "ll.h"

int main() {
    char input[256];
    char originalInput[256];
    char *first, *second, *third;
    struct LinkedList commandHistory;

    initialize(&commandHistory);

    printf("FML Command Parser (type quit to exit)\n");

    while (1) {
        printf("FML> ");
        fgets(input, 256, stdin);

        if (input[strlen(input)-1] == '\n') {
            input[strlen(input)-1] = '\0';
        }

        strcpy(originalInput, input);

        first = strtok(input, " ");
        second = strtok(NULL, " ");
        third = strtok(NULL, " ");

        if (first == NULL) {
            continue;
        }
        //quit
        if (strcmp(first, "quit") == 0) {
            printf("Exiting FML.\n");

            deleteList(&commandHistory);
            break;
        }

        // history
        if (strcmp(first, "history") == 0) {
            if (second != NULL) {
                printf("Syntax error: 'history' takes no parameters.\n");
            }
            else {
                printHistory(&commandHistory);

                addCommand(&commandHistory, originalInput);
            }
            continue;
        }

        // upload
        if (strcmp(first, "upload") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: upload <local filename> <remote filename>\n");
            } else {
                printf("Valid command: upload %s %s\n", second, third);
                addCommand(&commandHistory, originalInput);
            }
            continue;
        }

        // download
        if (strcmp(first, "download") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: download <remote filename> <local filename>\n");
            } else {
                printf("Valid command: download %s %s\n", second, third);
                addCommand(&commandHistory, originalInput);
            }
            continue;
        }

        // delete
        if (strcmp(first, "delete") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: delete local|remote <filename>\n");
            } else if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
                printf("'%s' is not recognized. Valid options are 'local' and 'remote'.\n", second);
            } else {
                printf("Valid command: delete %s %s\n", second, third);
                addCommand(&commandHistory, originalInput);
            }
            continue;
        }

        // change
        if (strcmp(first, "change") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: change local|remote <filepath>\n");
            } else if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
                printf("'%s' is not recognized. Valid options are 'local' and 'remote'.\n", second);
            } else {
                printf("Valid command: change %s %s\n", second, third);
                addCommand(&commandHistory, originalInput);
            }
            continue;
        }

        // show
        if (strcmp(first, "show") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: show local|remote path|files|folders\n");
            } else if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
                printf("'%s' is not recognized. Valid options for show are 'local' and 'remote'.\n", second);
            } else if (strcmp(third, "path") != 0 && strcmp(third, "files") != 0 && strcmp(third, "folders") != 0) {
                printf("'%s' is not recognized. Valid options are 'path', 'files', and 'folders'.\n", third);
            } else {
                printf("Valid command: show %s %s\n", second, third);
                addCommand(&commandHistory, originalInput);
            }
            continue;
        }

        

        // validate
        if (strcmp(first, "validate") == 0) {
            if (second != NULL) {
                printf("Syntax error: 'validate' takes no parameters.\n");
            } else {
                printf("Valid command: validate\n");
                addCommand(&commandHistory, originalInput);
            }
            continue;
        }

        // Unknown command
        printf("%s is not a valid FML command.\n", first);
    }

    return 0;
}
