#ifndef PORT
	#define PORT 57759
#endif
#define MAX_NAME 128
#define BUFFER_SIZE 128

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>
#include "utils.h"


static int listenfd;
int port = PORT;



Client *top;
Client *tail;


int howmany = 0;  //Total number of connected client


char* msg_greeting = "What is your user name? \r\n";
char* msg_noname = "Please enter a valid name! \r\n";
char* msg_getcmd = "Go ahead and enter user commands > \r\n";
static Client* addclient(int fd, struct in_addr addr);
static void removeclient(int fd);

void whatsup(struct client *p, QNode* root, Node* interests);

int main(int argc, char **argv)
{


  tail = top;
	QNode *root = NULL;
	Node * interests = NULL;

	if (argc < 2) {
        printf ("To run the program ./categorizer <name of input file>\n");
        return 1;
    }

    interests = get_list_from_file ( argv[1]);

    if (interests == NULL)
        return 1;

	root = add_next_level (root,  interests);

	//* abvoe are tree setup


    //int c;
    struct client *p;
    extern void bindandlisten(), newconnection(), whatsup(struct client *p, QNode* root, Node* interests);



    bindandlisten();  /* aborts on error */
   
    /* the only way the server exits is by being killed */
    while (1) {
	fd_set fdlist;
	int maxfd = listenfd;
	FD_ZERO(&fdlist);
	FD_SET(listenfd, &fdlist);
	for (p = top; p; p = p->next) {
	    FD_SET(p->fd, &fdlist);
	    if (p->fd > maxfd)
		maxfd = p->fd;
	}
	if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
	    perror("select");
	} else {
	    for (p = top; p; p = p->next)
		if (FD_ISSET(p->fd, &fdlist))
		    break;
		/*
		 * it's not very likely that more than one client will drop at
		 * once, so it's not a big loss that we process only one each
		 * select(); we'll get it later...
		 */
	    if (p)
		whatsup(p, root, interests);  /* might remove p from list, so can't be in the loop */
	    if (FD_ISSET(listenfd, &fdlist))
		newconnection(root, interests);
	}
    }
  
    return(0);
}


void bindandlisten()  /* bind and listen, abort on error */
{
    struct sockaddr_in r;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket");
	exit(1);
    }

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(port);
    printf("Listening on %d\n", PORT);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
	perror("bind");
	exit(1);
    }

    if (listen(listenfd, 5)) {
	perror("listen");
	exit(1);
    }
}


void newconnection(QNode* root, Node* interests)

{
    int fd;
    int nbytes;
    struct sockaddr_in r;
    socklen_t socklen = sizeof r;
    //int len, i, c;
    //int c;
    //char buf[sizeof msg_greeting + 40];

    //int where = -1;
    //int cmd_argc;
    //char** cmd_argv = malloc( sizeof (char*) * INPUT_ARG_MAX_NUM);
    //int retval;
    //for (int i = 0; i<=INPUT_ARG_MAX_NUM;i++){
    //	cmd_argv[i] = malloc( sizeof(char) * BUFFER_SIZE );
    //}


    if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
	perror("accept");
    } else {
	printf("connection from %s\n", inet_ntoa(r.sin_addr));
	Client* current_client;
	current_client = addclient(fd, r.sin_addr);



	nbytes = write(current_client->fd, msg_greeting, strlen (msg_greeting) );


	current_client->state = 0; //Set name is not read
	while (current_client->state == 0){
		nbytes = read(current_client->fd, current_client->name, sizeof current_client->name - 1);
		if (nbytes < 0){
			error("read");
		}
		if (strlen (current_client->name) > 0)
			{current_client->state = 1;
			int where = find_network_newline(current_client->name,strlen(current_client->name));
			if (where > 0){current_client->name[where] = '\0'; current_client->name[where+1] = '\0'; }
			printf("DEBUG: Username read is:%s!\n",current_client->name);	
			break; }

			/*The name adding loop*/
	}
	sprintf(current_client->buf ,"Welcome. %s\r\n",current_client->name);
	printf("DEBUG: User count is %d\n", howmany);
	nbytes = write(current_client->fd,current_client->buf,strlen (current_client->buf));
	nbytes = write(current_client->fd,msg_getcmd,strlen (msg_getcmd));





	/* wait for and get response -- look for first non-whitespace char */
	/* (buf is a good enough size for reads -- it probably will all fit
	 * into one read() for any non-trivial size.) */

	}
}


void whatsup(struct client *p, QNode* root, Node* interests)  /* select() said activity; check it out */
{
    	char temp[128];
			int inbuf = 0;
			char *after = temp + inbuf;
      int room = BUFFER_SIZE - inbuf;
    	int cmd_argc;
    	int nbytes;
			int where;




   		char** cmd_argv = malloc( sizeof (char*) * (INPUT_ARG_MAX_NUM+1));
    	int retval;
    	for (int i = 0; i<=INPUT_ARG_MAX_NUM+1;i++){
    		cmd_argv[i] = malloc( sizeof(char) * BUFFER_SIZE );
    	}


nbytes = read(p->fd,after,room);
			if (nbytes == 0){
			 //being interrupted to exit
			 sprintf(temp,"Good Bye %s \r\n" ,p->name);
			 //broadcast(temp, strlen(temp));
			 removeclient(p->fd);

		 }

			while(1){


			inbuf+=nbytes;
			after = temp + inbuf;
			room = BUFFER_SIZE - inbuf;

			where = find_network_newline(temp,inbuf);
			if (where > 0) {
				temp[where] = '\0';

				strcpy(p->buf, temp);

				break;}
    nbytes = read(p->fd,after,room);


		}


	if(nbytes > 0){

			cmd_argc = tokenize(p->buf, cmd_argv);
			retval = process_args(cmd_argc, cmd_argv, &root, interests, p, top);
			if(retval == -1){
				//disconect
					removeclient(p->fd);}
			if (retval ==0){
					printf("Success runing \n");
			}
		}
		else if (nbytes == 0){
			//being interrupted to exit
			sprintf(temp,"Good Bye %s \r\n" ,p->name);
			//broadcast(temp, strlen(temp));
			removeclient(p->fd);

		}
		else {
			perror("read");
		}




}


static Client* addclient(int fd, struct in_addr addr)
{
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
			fprintf(stderr, "out of memory!\n");  /* highly unlikely to happen */
			exit(1);
    }
    printf("Adding client %s\n", inet_ntoa(addr));
    fflush(stdout);

   	p -> fd = fd;
    p -> ipaddr = addr;
		p->prev = NULL;
		p->next = NULL;

		if (top == NULL){
    	top = p;

    	tail = p;
    }
  	else {

  		tail->next = p;
  		p->prev = tail;
  		tail = tail->next;

  	}


    howmany++;


    return p;
}



static void removeclient(int fd)
{ 
    struct client **p;
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
	;
    if (*p) {
	struct client *t = (*p)->next;
	printf("Removing client %s\n", inet_ntoa((*p)->ipaddr));
	fflush(stdout);
	free(*p);
	*p = t;
	howmany--;
    } else {
	fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
	fflush(stderr);
    }
}