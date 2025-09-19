#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ll.h"

void initialize(struct LinkedList* list) {
	list->size = 0;
	list->head = NULL;
}

void addCommand()