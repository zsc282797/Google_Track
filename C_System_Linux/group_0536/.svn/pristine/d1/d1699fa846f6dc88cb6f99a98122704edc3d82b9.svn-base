#include "qtree.h"

QNode *add_next_level (QNode *current, Node *list_node) {
	int str_len;
	
	str_len = strlen (list_node->str);
	current = (QNode *) calloc (1, sizeof(QNode));

	current->question =  (char *) calloc (str_len +1, sizeof(char ));
	strncpy ( current->question, list_node->str, str_len );
	current->question [str_len] = '\0';  
	current->node_type = REGULAR;
	
	if (list_node->next == NULL) {
		current->node_type = LEAF;
		return current;
	}
	
	current->children[0].qchild = add_next_level ( current->children[0].qchild, list_node->next);
	current->children[1].qchild = add_next_level ( current->children[1].qchild, list_node->next);

	return current;
}

void print_qtree (QNode *parent, int level) {
	int i;
	for (i=0; i<level; i++)
		printf("\t");
	
	printf ("%s type:%d\n", parent->question, parent->node_type);
	if(parent->node_type == REGULAR) {
		print_qtree (parent->children[0].qchild, level+1);
		print_qtree (parent->children[1].qchild, level+1);
	}
	else { //leaf node
		for (i=0; i<(level+1); i++)
			printf("\t");
		print_users (parent->children[0].fchild);
		for (i=0; i<(level+1); i++)
			printf("\t");
		print_users (parent->children[1].fchild);
	}
}

void print_users (Node *parent) {
	if (parent == NULL)
		printf("NULL\n");
	else {
		printf("%s, ", parent->str);
		while (parent->next != NULL) {
			parent = parent->next;
			printf("%s, ", parent->str);
		}
		printf ("\n");
	}
}

// traverse tree: check whether current is a leaf
int is_leaf(QNode *current){
	if(current->node_type == LEAF){
		return 1;
	} else {
		return 0;
	}
}

// helper function for add_users
Node *create_user(char *name){
	Node *user = (Node *)malloc(sizeof(Node));
	user->str = (char *)malloc(sizeof(char) * (strlen(name) + 1));

	strncpy(user->str, name, strlen(name));
	user->str[strlen(name)] = '\0';

	user->next = NULL;

	return user;
}

// if the current qnode is a leaf, return itself and define its fchild
QNode *add_users(char *name, QNode *current, int answer) {
	if (current->node_type == LEAF){
		Node *user = create_user(name);

		if(current->children[answer].fchild == NULL) {
			current->children[answer].fchild = user;
		} else {

			Node *curr = current->children[answer].fchild;
			while(curr->next != NULL) {
				curr = curr->next;
			}
			curr->next = user;
		}
		return current;
	}
	return current->children[answer].qchild;
}

// print user name list without the user himself/herself
void print_friend_list(Node *head, char *name){
	if((strcmp(head->str, name) == 0) && (head->next == NULL)){
		printf("Sorry, no users with similar interests joined yet\n\n");
	} else {
		Node *curr = head;
		int num_friends = 0;
		printf("friend recommendations for user %s:\n", name);
		while(curr->next != NULL) {
			if(strcmp(curr->str, name) != 0) {
				if (strcmp(curr->next->str, name) == 0) {
					break;
				}
				printf("%s, ", curr->str);
				num_friends++;
			}
			curr = curr->next;
		}

		if (curr->next == NULL) {
			printf("%s\n", curr->str);
			num_friends++;
		} else if (curr->next->next == NULL) {
			printf("%s\n", curr->str);
			num_friends++;
		} else {
			printf("%s, ", curr->str);
			num_friends++;
			curr = curr->next->next;
			while(curr->next != NULL) {
				printf("%s, ", curr->str);
				num_friends++;
				curr = curr->next;
			}
			printf("%s\n", curr->str);
			num_friends++;

		}
		printf ("You have total %d potential friend(s)!!!\n\n", num_friends);
	}
}

// helper function for search_user
int contain_user(Node *head, char *name) {
	Node *curr = head;
	while(curr != NULL) {
		if (strcmp(curr->str, name) == 0) {
			return 1;
		}
		curr = curr->next;
	}
	return 0;
}

Node *search_user(char *name, QNode *current){
	if (current->node_type == LEAF){
		// check left
		if (contain_user(current->children[0].fchild, name)){
			return current->children[0].fchild;
		}
			
		// check right
		if (contain_user(current->children[1].fchild, name)){
			return current->children[1].fchild;
		}
		
		return NULL;

	}

	Node *left = search_user(name, current->children[0].qchild);
	Node *right = search_user(name, current->children[1].qchild);

	if (left != NULL) {
		return left;
	}
	if (right != NULL) {
		return right;
	}

	return NULL;
}

void free_qtree(QNode *current) {
    
    if (current->node_type == LEAF) {
		if (current->children[0].fchild != NULL){
			free_list(current->children[0].fchild);
            current->children[0].fchild = NULL;
		}
		if (current->children[1].fchild != NULL){
			free_list(current->children[1].fchild);
            current->children[1].fchild = NULL;
		}
		
        free(current->question);
		free(current);
		current = NULL;

	} else {
		free_qtree(current->children[0].qchild);
        free_qtree(current->children[1].qchild);
        free(current->question);
        free(current);
        
	}
}
