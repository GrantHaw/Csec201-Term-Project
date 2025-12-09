#pragma once
#include <winsock2.h>

struct node {
    char command[256];
    unsigned int hash; //32 bit
    struct node* next;
};

struct LinkedList {
    int size;
    struct node* head;
};

void initialize(struct LinkedList* list);
void addCommand(struct LinkedList* list, char* command);
void printHistory(struct LinkedList* list, SOCKET* clientSock);
void deleteList(struct LinkedList* list);

int validateBlockchain(struct LinkedList* list, SOCKET* clientSock);

//test stuff
void testModifyCommand(struct LinkedList* list, int nodePosition, char* newCommand);
void testModifyHash(struct LinkedList* list, int nodePosition);
void testDeleteNode(struct LinkedList* list, int nodePosition);