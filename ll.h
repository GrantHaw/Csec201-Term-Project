#pragma once

struct node {
	char command[256];
	
	//32 bit hash
	unsigned int hash;
					
	// next node
	struct node* next;
};

struct LinkedList {
	int size;
	struct node* head;
};

//func prototypes
void initialize(struct LinkedList* list);

void addCommand(struct LinkedList* list, char* command);

void printHistory(struct LinkedList* list);

void deleteList(struct LinkedList* list);

void canonicalize(char* input, char* output);

unsigned int rotl32(unsigned int x, int r);

unsigned int nextHash(unsigned int prevHash, const unsigned char* bytes, int length);

int validateBlockchain(struct LinkedList* list);