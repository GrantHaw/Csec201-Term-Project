#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ll.h"

#define SEED 0x5A

// External function pointers loaded from DLL
extern void (*canonicalize)(char*, char*);
extern unsigned int (*rotl32)(unsigned int, int);
extern unsigned int (*nextHash)(unsigned int, const unsigned char*, int);

void initialize(struct LinkedList* list) {
    list->size = 0;
    list->head = NULL;
}

void addCommand(struct LinkedList* list, char* command) {
    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    char canonicalCommand[256];
    unsigned int prevHash;
    
    canonicalize(command, canonicalCommand);
    strcpy(newNode->command, canonicalCommand);
    
    if (list->head != NULL) {
        prevHash = list->head->hash;
    } else {
        prevHash = SEED;
    }
    
    newNode->hash = nextHash(prevHash, (const unsigned char*)canonicalCommand, strlen(canonicalCommand));
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

void printHistory(struct LinkedList* list) {
    struct node* temp = list->head;
    int commandNumber = list->size;
    
    if (temp == NULL) {
        printf("No command history available.\n");
        return;
    }
    
    printf("Command History:\n");
    while (temp != NULL) {
        printf("%d: %s [Hash: 0x%08X]\n", commandNumber, temp->command, temp->hash);
        temp = temp->next;
        commandNumber--;
    }
}

int validateBlockchain(struct LinkedList* list) {
    if (list->head == NULL) {
        printf("Blockchain is empty - valid by default.\n");
        return 1;
    }
    
    struct node* nodes[1000];
    int count = 0;
    struct node* current = list->head;
    
    while (current != NULL && count < 1000) {
        nodes[count] = current;
        current = current->next;
        count++;
    }
    
    unsigned int prevHash = SEED;
    int alterationDetected = 0;
    int i;
    
    for (i = count - 1; i >= 0; i--) {
        unsigned int recomputedHash = nextHash(prevHash, (const unsigned char*)nodes[i]->command, strlen(nodes[i]->command));
        
        if (nodes[i]->hash != recomputedHash) {
            printf("Command history validation FAILED.\n");
            printf("First invalid node detected: \"%s\"\n", nodes[i]->command);
            printf("Expected hash: 0x%08X\n", recomputedHash);
            printf("Actual hash:   0x%08X\n", nodes[i]->hash);
            alterationDetected = 1;
            break;
        }
        
        prevHash = nodes[i]->hash;
    }
    
    if (!alterationDetected) {
        printf("Command history validation PASSED - no alterations detected.\n");
        return 1;
    }
    
    return 0;
}

void deleteList(struct LinkedList* list) {
    struct node* current = list->head;
    struct node* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    list->head = NULL;
    list->size = 0;
}

void testModifyCommand(struct LinkedList* list, int nodePosition, char* newCommand) {
    if (list->head == NULL) {
        printf("Cannot test - blockchain is empty\n");
        return;
    }
    
    struct node* current = list->head;
    int count = 1;
    
    while (current != NULL && count < nodePosition) {
        current = current->next;
        count++;
    }
    
    if (current != NULL) {
        printf("TEST: Changing command from \"%s\" to \"%s\"\n", current->command, newCommand);
        strcpy(current->command, newCommand);
    } else {
        printf("Node %d not found\n", nodePosition);
    }
}

void testModifyHash(struct LinkedList* list, int nodePosition) {
    if (list->head == NULL) {
        printf("Cannot test - blockchain is empty\n");
        return;
    }
    
    struct node* current = list->head;
    int count = 1;
    
    while (current != NULL && count < nodePosition) {
        current = current->next;
        count++;
    }
    
    if (current != NULL) {
        printf("TEST: Changing hash from 0x%08X to 0x%08X\n", current->hash, current->hash + 1);
        current->hash = current->hash + 1;
    } else {
        printf("Node %d not found\n", nodePosition);
    }
}

void testDeleteNode(struct LinkedList* list, int nodePosition) {
    if (list->head == NULL) {
        printf("Cannot test - blockchain is empty\n");
        return;
    }
    
    if (nodePosition == 1) {
        struct node* temp = list->head;
        printf("TEST: Deleting head node \"%s\"\n", temp->command);
        list->head = list->head->next;
        free(temp);
        list->size--;
        return;
    }
    
    struct node* current = list->head;
    int count = 1;
    
    while (current != NULL && current->next != NULL && count < nodePosition - 1) {
        current = current->next;
        count++;
    }
    
    if (current != NULL && current->next != NULL) {
        struct node* nodeToDelete = current->next;
        printf("TEST: Deleting node \"%s\"\n", nodeToDelete->command);
        current->next = nodeToDelete->next;
        free(nodeToDelete);
        list->size--;
    } else {
        printf("Node %d not found\n", nodePosition);
    }
}