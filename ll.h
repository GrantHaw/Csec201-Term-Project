#pragma once


struct node {
	char command[256];

	struct node* next;
};

struct LinkedList {
	int size;
	struct node* head;
};

void initialize(struct LinkedList* list);

void addCommand(struct LinkedList* list, char * command);

void printHistory(struct LinkedList* list);

void deleteList(struct LinkedList* list);