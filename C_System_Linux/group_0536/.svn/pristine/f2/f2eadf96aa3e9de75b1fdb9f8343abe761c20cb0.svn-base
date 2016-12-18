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

// find the branch that answer corresponds to, starting from
// node current
QNode *find_branch(QNode *current, int answer){
    return current->children[answer].qchild;   
}

// add user to the end of the user list and return the list
Node *add_user(Node *head, char *user){
    // create a new user node    
    Node *new = malloc(sizeof(Node));
    if (!new){
        perror("malloc");
        exit(1);
    }
    
    char *newstr = malloc((strlen(user)+1)*sizeof(char));
    if (!newstr){
        perror("malloc");
        exit(1);
    }
    strcpy(newstr, user);
        
    new->str = newstr;
    new->next = NULL;
    
    // list is empty
    if (!head){
        head = new;
    
    } else {
        // find the end of the list
        Node *current;
        for (current = head; current->next; current = current->next);
        
        current->next = new;
    }
    
    return head;
}

// find a user in the tree
// return NULL if the user was not found
Node *find_user(QNode *current, char *name){
    // current is an inner node
    if (current->node_type == REGULAR){
        
        Node *head;
        // look for the node (recursively) in the 0 subtree
        head = find_user(current->children[0].qchild, name);
        if (head)
            return head;
            
        // look for the node (recursively) in the 1 subtree
        head = find_user(current->children[1].qchild, name);
        if (head)
            return head;
            
    // current is a leaf node
    } else {
        // look for the user in the 0 child       
        Node *head = current->children[0].fchild;
        
        while (head != NULL) {
            if (strcmp(head->str, name) == 0)
                return current->children[0].fchild;
            head = head->next;
        }

        // look for the user in the 1 child
        head = current->children[1].fchild;

        while (head != NULL) {
            if (strcmp(head->str, name) == 0)
                return current->children[1].fchild;
            head = head->next;
        }
    }
    
    return NULL;
}

// free tree
void free_qtree(QNode *current){
    if (current){
        if (current->node_type == LEAF){         
            // free lists of users
            free_list(current->children[0].fchild);
            free_list(current->children[1].fchild);
                      
        } else {
            // free left subtree
            free_qtree(current->children[0].qchild);
            // free right subtree
            free_qtree(current->children[1].qchild);
        }
        
        free(current->question); // free question
        free(current);
    }
}
