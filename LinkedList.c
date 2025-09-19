#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ll.h"

void initialize(struct LinkedList* list) {
	list->size = 0;
	list->head = NULL;
}

void addCommand(struct LinkedList* list, char* command) {
	struct node* newNode = (struct node*)malloc(sizeof(struct node));
	strcpy(newNode->command, command);

	newNode->next = list->head;
	list->head = newNode;
	list->size++;
}

void printHistory(struct LinkedList* list) {
	struct node* temp = list->head;
	int commandNumber = list->size;

	if (temp == NULL) {
		printf("No commands in history :(\n");
		return;
	}

	printf("Command History:\n");
	while (temp != NULL) {
		printf("%d: %s\n", commandNumber, temp->command);
		temp = temp->next;
		commandNumber--;
	}
}

void deleteList(struct LinkedList * list) {
	struct node* current =list->head;
	struct node* next;

	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}
	list->head = NULL;
	list->size = 0;
}

