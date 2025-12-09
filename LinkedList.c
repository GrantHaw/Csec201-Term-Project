#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "ll.h"

#define SEED 0x5A

// these come from the dll (defined in FMLServer.c)
typedef void (*CanonicalizeFunc)(char*, char*);
typedef unsigned int (*Rotl32Func)(unsigned int, int);
typedef unsigned int (*NextHashFunc)(unsigned int, const unsigned char*, int);

extern CanonicalizeFunc pfnCanonicalize;
extern Rotl32Func pfnRotl32;
extern NextHashFunc pfnNextHash;

void initialize(struct LinkedList* list) {
    list->size = 0;
    list->head = NULL;
}

void addCommand(struct LinkedList* list, char* command) {
    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    char canonical[256];
    unsigned int prev;

    pfnCanonicalize(command, canonical);
    strcpy(newNode->command, canonical);

    if (list->head != NULL) {
        prev = list->head->hash;
    }
    else {
        prev = SEED;
    }

    newNode->hash = pfnNextHash(prev, (const unsigned char*)canonical, (int)strlen(canonical));
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

void printHistory(struct LinkedList* list, SOCKET* clientSock) {
    struct node* temp = list->head;
    int num = list->size;
    int bw = 0; //bytes written
    char output[5000] = "";
    SOCKET sock = *clientSock;

    if (temp == NULL) {
        bw = sprintf(output, "History is empty\n");
        send(sock, output, (int)strlen(output), 0);
        return;
    }

    bw = sprintf(output, "------\n");

    while (temp != NULL) {
        bw += sprintf(&(output[bw]), "%d: %p  Hash: 0x%08X  Cmd: %s\n",
            num, temp, temp->hash, temp->command);
        temp = temp->next;
        num--;
    }
    bw += sprintf(&(output[bw]), "------\n");

    //do validation inline
    struct node* nodes[1000];
    int count = 0;
    temp = list->head;
    while (temp != NULL && count < 1000) {
        nodes[count] = temp;
        temp = temp->next;
        count++;
    }

    unsigned int prevHash = SEED;
    int ok = 1;
    int i;
    for (i = count - 1; i >= 0; i--) {
        unsigned int check = pfnNextHash(prevHash, (const unsigned char*)nodes[i]->command, (int)strlen(nodes[i]->command));
        if (nodes[i]->hash != check) {
            ok = 0;
            break;
        }
        prevHash = nodes[i]->hash;
    }

    if (ok) {
        bw += sprintf(&(output[bw]), "blockchain integrity ok\n");
    }
    else {
        bw += sprintf(&(output[bw]), "WARNING: blockchain compromised!\n");
    }

    send(sock, output, (int)strlen(output), 0);
}

int validateBlockchain(struct LinkedList* list, SOCKET* clientSock) {
    SOCKET sock = *clientSock;
    char err[1000] = "";
    int bw = 0;
    int i;

    if (list->head == NULL) {
        return 1; //empty is valid i guess
    }

    //put nodes in array so we can go backwards
    struct node* nodes[1000];
    int count = 0;
    struct node* curr = list->head;
    while (curr != NULL && count < 1000) {
        nodes[count] = curr;
        curr = curr->next;
        count++;
    }

    unsigned int prev = SEED;
    for (i = count - 1; i >= 0; i--) {
        unsigned int expected = pfnNextHash(prev, (const unsigned char*)nodes[i]->command, (int)strlen(nodes[i]->command));

        if (nodes[i]->hash != expected) {
            //found bad node
            bw = sprintf(err, "ERROR> alteration found:\n");
            bw += sprintf(&(err[bw]), "  Node: %p\n", nodes[i]);
            bw += sprintf(&(err[bw]), "  Hash: 0x%08X (expected 0x%08X)\n", nodes[i]->hash, expected);
            bw += sprintf(&(err[bw]), "  Cmd: %s\n", nodes[i]->command);
            send(sock, err, (int)strlen(err), 0);
            return 0;
        }
        prev = nodes[i]->hash;
    }

    return 1;
}

void deleteList(struct LinkedList* list) {
    struct node* curr = list->head;
    struct node* next;

    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    list->head = NULL;
    list->size = 0;
}

//test funcs - prob dont need these anymore but keeping just in case
void testModifyCommand(struct LinkedList* list, int pos, char* newCmd) {
    if (list->head == NULL) {
        printf("list empty\n");
        return;
    }

    struct node* curr = list->head;
    int c = 1;
    while (curr != NULL && c < pos) {
        curr = curr->next;
        c++;
    }

    if (curr != NULL) {
        printf("changing \"%s\" to \"%s\"\n", curr->command, newCmd);
        strcpy(curr->command, newCmd);
    }
}

void testModifyHash(struct LinkedList* list, int pos) {
    if (list->head == NULL) return;

    struct node* curr = list->head;
    int c = 1;
    while (curr != NULL && c < pos) {
        curr = curr->next;
        c++;
    }
    if (curr != NULL) {
        curr->hash = curr->hash + 1;
    }
}

void testDeleteNode(struct LinkedList* list, int pos) {
    if (list->head == NULL) return;

    if (pos == 1) {
        struct node* temp = list->head;
        list->head = list->head->next;
        free(temp);
        list->size--;
        return;
    }

    struct node* curr = list->head;
    int c = 1;
    while (curr != NULL && curr->next != NULL && c < pos - 1) {
        curr = curr->next;
        c++;
    }

    if (curr != NULL && curr->next != NULL) {
        struct node* del = curr->next;
        curr->next = del->next;
        free(del);
        list->size--;
    }
}