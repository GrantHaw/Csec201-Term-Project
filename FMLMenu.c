#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include "ll.h"

typedef void (*CanonicalizeFunc)(char*, char*);
typedef unsigned int (*Rotl32Func)(unsigned int, int);
typedef unsigned int (*NextHashFunc)(unsigned int, const unsigned char*, int);

HMODULE hDLL = NULL;
CanonicalizeFunc canonicalize = NULL;
Rotl32Func rotl32 = NULL;
NextHashFunc nextHash = NULL;

int loadBlockchainDLL() {
    hDLL = LoadLibrary(L"BlockchainDLL.dll");
    if (hDLL == NULL) {
        printf("ERROR: Could not load BlockchainDLL.dll\n");
        printf("Make sure the DLL file is in the same directory as the executable.\n");
        return 0;
    }

    canonicalize = (CanonicalizeFunc)GetProcAddress(hDLL, "canonicalize");
    if (canonicalize == NULL) {
        printf("ERROR: Could not find 'canonicalize' function in DLL\n");
        FreeLibrary(hDLL);
        return 0;
    }

    rotl32 = (Rotl32Func)GetProcAddress(hDLL, "rotl32");
    if (rotl32 == NULL) {
        printf("ERROR: Could not find 'rotl32' function in DLL\n");
        FreeLibrary(hDLL);
        return 0;
    }

    nextHash = (NextHashFunc)GetProcAddress(hDLL, "nextHash");
    if (nextHash == NULL) {
        printf("ERROR: Could not find 'nextHash' function in DLL\n");
        FreeLibrary(hDLL);
        return 0;
    }

    printf("Blockchain DLL loaded successfully.\n");
    return 1;
}

void unloadBlockchainDLL() {
    if (hDLL != NULL) {
        FreeLibrary(hDLL);
        hDLL = NULL;
        canonicalize = NULL;
        rotl32 = NULL;
        nextHash = NULL;
    }
}

int main() {
    char input[256];
    char originalInput[256];
    char *first, *second, *third;
    struct LinkedList commandHistory;

    if (!loadBlockchainDLL()) {
        printf("Failed to load blockchain functionality. Exiting.\n");
        return 1;
    }

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
            unloadBlockchainDLL();
            break;
        }

        // history
        if (strcmp(first, "history") == 0) {
            if (second != NULL) {
                printf("Syntax error: 'history' takes no parameters.\n");
            }
            else {
                printHistory(&commandHistory);

                validateBlockchain(&commandHistory);

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
            }
            else {
                printf("Valid command: validate\n");
                // validate blockchauin  
                validateBlockchain(&commandHistory);
                // add cmd
                addCommand(&commandHistory, originalInput);
            } 
            continue;
        }
        
        if (strcmp(first, "test_delete_node") == 0) {
            if (second == NULL) {
                printf("Syntax error: test_delete_node <node_position>\n");
            } else {
                int nodePos = atoi(second);
                printf("TEST: Attempting to delete node at position %d\n", nodePos);
                testDeleteNode(&commandHistory, nodePos);
                printf("Node deletion test completed. Run 'validate' to check integrity.\n");
            }
            continue;
        }

        if (strcmp(first, "test_modify_command") == 0) {
            if (second == NULL || third == NULL) {
                printf("Syntax error: test_modify_command <node_position> <new_command>\n");
            } else {
                int nodePos = atoi(second);
                printf("TEST: Attempting to modify command at position %d\n", nodePos);
                testModifyCommand(&commandHistory, nodePos, third);
                printf("Command modification test completed. Run 'validate' to check integrity.\n");
            }
            continue;
        }

        if (strcmp(first, "test_modify_two_commands") == 0) {
            printf("TEST: Modifying commands in positions 1 and 2\n");
            testModifyCommand(&commandHistory, 1, "HACKED_COMMAND_1");
            testModifyCommand(&commandHistory, 2, "HACKED_COMMAND_2");
            printf("Two-command modification test completed. Run 'validate' to check integrity.\n");
            continue;
        }

        if (strcmp(first, "test_modify_hash") == 0) {
            if (second == NULL) {
                printf("Syntax error: test_modify_hash <node_position>\n");
            } else {
                int nodePos = atoi(second);
                printf("TEST: Attempting to modify hash at position %d\n", nodePos);
                testModifyHash(&commandHistory, nodePos);
                printf("Hash modification test completed. Run 'validate' to check integrity.\n");
            }
            continue;
        }

        if (strcmp(first, "test_modify_two_hashes") == 0) {
            printf("TEST: Modifying hashes in positions 1 and 2\n");
            testModifyHash(&commandHistory, 1);
            testModifyHash(&commandHistory, 2);
            printf("Two-hash modification test completed. Run 'validate' to check integrity.\n");
            continue;
        }

        if (strcmp(first, "test_help") == 0) {
            printf("Available blockchain integrity tests:\n");
            printf("  test_delete_node <position>           - Delete node at specified position\n");
            printf("  test_modify_command <pos> <new_cmd>   - Change command at position\n");
            printf("  test_modify_two_commands              - Change commands at positions 1 and 2\n");
            printf("  test_modify_hash <position>           - Change hash at specified position\n");
            printf("  test_modify_two_hashes                - Change hashes at positions 1 and 2\n");
            printf("  validate                              - Check blockchain integrity\n");
            printf("  history                               - Show command history with auto-validation\n");
            continue;
        }

        // Unknown command
        printf("%s is not a valid FML command.\n", first);
    }

    return 0;
}