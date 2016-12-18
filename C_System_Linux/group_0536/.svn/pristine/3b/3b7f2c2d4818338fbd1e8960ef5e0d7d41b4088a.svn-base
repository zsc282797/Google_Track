#ifndef QTREE_H
#define QTREE_H
#include "questions.h"

typedef enum {
    REGULAR, LEAF
} NodeType;

union Child {
	struct str_node *fchild;
	struct QNode *qchild;
} Child;

typedef struct QNode {
	char *question;
	NodeType node_type;
	union Child children[2];
} QNode;

QNode *add_next_level (QNode *current, Node * list_node);

void print_qtree (QNode *parent, int level);
void print_users (Node *parent);

Node *add_user(Node *head, char *user);
QNode *find_branch(QNode *current, int answer);
Node *find_user(QNode *current, char *name);
void free_qtree(QNode *root);

#endif
