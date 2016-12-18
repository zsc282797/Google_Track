#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "questions.h"

Node * get_list_from_file (char *input_file_name) {
	Node * head = NULL;
	Node * tail = NULL;
	char line[MAX_LINE];
	FILE * inputFP;
	
    //read lines and add to the list
    inputFP = fopen(input_file_name, "r");
    if (inputFP == NULL) {
    	fprintf(stderr, "ERROR: The file %s does not open successfully.\n", input_file_name);
    	return NULL;
    }

    while (fgets(line, MAX_LINE, inputFP) != NULL) {
    	line[strcspn (line, "\r\n")] = '\0';

    	Node * current = (Node *) malloc(sizeof(Node));
    	current->str = (char *) malloc(sizeof(char) * (strlen(line) + 1));

    	strncpy(current->str, line, strlen(line));
    	current->str[strlen(line)] = '\0';
    	current->next = NULL;

    	if(head == NULL) {
    		head = current;
    		tail = current;
    	} else {
    		tail->next = current;
    		tail = current;
    	}
    	
    }

    fclose(inputFP);

	return head;
}

void print_list (Node *head) {
	Node * current = head;

	while(1){
		if (current == NULL){
			break;
		}
		printf("%s\n", current->str);
		current = current->next;
	}
	
}

void free_list (Node *head) {
	Node * current = NULL;
	Node * temp = head;

	while(temp != NULL){
		current = temp;
		temp = temp->next;
		free(current->str);
		free(current);
	}

	head = NULL;
}