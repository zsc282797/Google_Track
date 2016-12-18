#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils.h"
#include "qtree.h"
#include <sys/select.h>


char *question_prompt = "Do you like %s? (y/n)\r\n";
char *neg_result = "Sorry, no users with similar interests joined yet\r\n";
char *pos_result1 = "friend recommendation for user %s:\r\n";
char *pos_result2 = "You have total %d potential friend(s)!!!\r\n";
char *goodbye_msg = "Good Bye %s \r\n";
char *offline_msg = "Sorry the user is offline \r\n";
char *mismatch_head = "Here are your best mismatches: \r\n";
char *not_found = "No completing personalities found. Please try again later \r\n";
char *not_complete = "Please completed the questions first \r\n";
char *err_cmd = "Invalid Command, Try again! \r\n";
//char *message_head = "Message from %s: ";


char answer[10];
Client * find_client(char* name, Client* top);

int validate_answer(char *answer){
    char *invalid_message = "ERROR: Answer must be one of 'y', 'n', 'q'.\n";

    if (strlen(answer) > 3){
        printf("%s", invalid_message);
        return 2;
    }

    if (answer[0] == 'q' || answer[0] == 'Q')
        //wrap_up();
        return -1;

    if (answer[0] == 'n' || answer[0] == 'N')
        return 0;

    if (answer[0] == 'y' || answer[0] == 'Y')
        return 1;

    printf("%s", invalid_message);
    return 2;
}







/*
 * Print a formatted error message to stderr.
 */
void error(char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
}
/*
* helper to get the size of the interests
*/
int len_interests(Node *interests){
    Node* curr;
    curr = interests;
    if (curr == NULL)
        return 0;
    else {
        int count = 0;
        while (curr!=NULL){
            count += 1;
            curr = curr->next;
        }

        return count;
    }
}
/*
 * Read and process commands
 */
int process_args(int cmd_argc, char **cmd_argv, QNode **root, Node *interests,
		 /*The skeleton is provided */
         struct client *current_client, struct client *head) {



        int len_int;
        char* test_greet = "Collecting your interests\n";
        char* complete = "Test complete.\n";
	    len_int = len_interests(interests);
    if (cmd_argc <= 0) {
		return 0;

	} else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
		/* Return an appropriate value to denote that the specified
		 * user is now need to be disconnected. */
		//printf("Client is being disconnected! \n");
		//printf("Removing client %s\n", inet_ntoa(current_client->ipaddr));

        sprintf(current_client->buf, goodbye_msg, current_client->name);
        write(current_client->fd, current_client->buf, strlen(current_client->buf));
        close(current_client->fd);


		return -1;

	} else if (strcmp(cmd_argv[0], "do_test") == 0 && cmd_argc == 1) {
		/* The specified user is ready to start answering questions. You
		 * need to make sure that the user answers each question only
		 * once.
		 */


		write(current_client->fd, test_greet, strlen(test_greet));
		char* name = current_client->name;
		printf("Debug: name is %s\n",name);

        printf("Debug: intersts list has %d question \n",len_int);
        current_client -> answers = malloc (sizeof(int) * len_int);

		Node *user_list = find_user(*root, name);
        //int idx = 0;
        if (!user_list){
            // find the list of friends to which the user should be attached
            //printf("We got null from user_list\n");

            QNode *prev, *curr;
            prev = curr = *root;

            // iterate over list of interests
            Node *i = interests;
            int ans;
            int idx = 0;

            while (i!=NULL){
                //printf(question_prompt, i->str);
                char question[128];
                sprintf(question, question_prompt, i->str);
                //char* question = strcat(question_prompt, i->str);
                write(current_client->fd, question, strlen(question));

                // read answer to prompt
                //scanf("%s", answer);
                fd_set fdlist;
                FD_ZERO(&fdlist);
                FD_SET(current_client->fd, &fdlist);


                char temp[128];
                int inbuf = 0;
                char *after = temp + inbuf;
                int room = BUFFER_SIZE - inbuf;

                int nbytes;
                int where;

                while(1){
                nbytes = read(current_client->fd,after,room);
                inbuf+=nbytes;
                after = temp + inbuf;
                room = BUFFER_SIZE - inbuf;

                where = find_network_newline(temp,inbuf);
                if (where > 0) {
                  temp[where] = '\0';



                  break;}



              }



                ans = validate_answer(temp);
                current_client -> answers[idx] = ans;
                idx++;
                // if answer if not valid, continue to loop
                if (ans == 2) {
                	char* invalid_message = "ERROR: Answer must be one of 'y', 'n', 'q'.\n";
                	write(current_client->fd, invalid_message, strlen(invalid_message));
                    continue;}
                if (ans == -1)
                	exit(1);
                prev = curr;
                curr = find_branch(curr, ans);


                i = i -> next;
            }
            current_client->completed = 1;
            write(current_client->fd, complete, strlen(complete));

            // add user to the end of the list
            user_list = prev->children[ans].fchild;
            prev->children[ans].fchild = add_user(user_list, name);
            return 0;
        }
        else {
            write(current_client->fd, test_done, strlen(test_done));
        }


	} else if (strcmp(cmd_argv[0], "get_all") == 0 && cmd_argc == 1) {
		/* Send the list of best mismatches related to the specified
		 * user. If the user has not taked the test yet, return the
		 * corresponding error value (different than 0 and -1).
		 */
        QNode *prev, *curr;
        prev = curr = *root;

        Node *user_list;
        int ans;

        printf("Looking at current_client's name: %s\n",current_client->name);
        if (!current_client->completed){
            write(current_client->fd, not_complete, strlen(not_complete));
        }

        for (int i=0; i<len_int; i++){
            printf("Answer[%d] = %d \n",i,current_client->answers[i]);
            ans = !current_client->answers[i];

            prev = curr;
            curr = find_branch(curr, ans);
        }//Switch to the correct branch
        user_list = prev->children[ans].fchild;
        if(!user_list){
          write(current_client->fd, not_found, strlen(not_found) );
        }

        else {
            Node* parent = user_list;
            write(current_client->fd, mismatch_head, strlen(mismatch_head));
            char temp[128];
            sprintf(temp, "%s\r\n", parent->str);
            write(current_client->fd, temp, strlen(temp));
            while (parent->next != NULL) {
                parent = parent->next;
                sprintf(temp, "%s\r\n", parent->str);
                write(current_client->fd, temp, strlen(temp));
            }
        }


        //print_qtree(*root,0);




	} else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc == 3) {
		/* Send the specified message stored in cmd_argv[2] to the user
		 * stored in cmd_argv[1].
		 */
        printf("Entered post switch of the command \n");
        //char* message = cmd_argv[2];
        printf("Message to deliver is %s",cmd_argv[2]);
        char* username = cmd_argv[1];

        int where = find_network_newline(username,strlen(username));
        if (where >0){ username[where] = '\0';}
        Client* receiver = find_client(username, head);
        printf("DEBUG: Search for:%s!\n",username );
        if (!receiver){//No user found

            write(current_client->fd, offline_msg, strlen(offline_msg));
            return 0;
        }
        else {
            char temp[128];

            sprintf(temp,"Message from %s: %s\r\n",current_client->name,
                cmd_argv[2]);
            write(receiver->fd, temp, strlen(temp));


            //write(receiver->fd, cmd_argv[2], strlen(cmd_argv[2]));
            return 0;
        }

	}
	else {
      int nbytes = write(current_client->fd, err_cmd, strlen(err_cmd));
      if (nbytes < 0){
        error("write");
      }
        //printf("cmd_argc = %d \n", cmd_argc);
        //printf("cmd_argv[%d] :%s \n",0,cmd_argv[0]);
        //printf("cmd_argv[%d] :%s \n",1,cmd_argv[1]);
        //printf("cmd_argv[%d] :%s \n",2,cmd_argv[2]);
		/* The input message is not properly formatted. */
		error("Incorrect syntax");
	}
	return 0;
}

/*
 * Tokenize the command stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv) {
    /*Provided by the instructor*/
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);
    while (  next_token != NULL   ) {

        /*if (cmd_argc == (INPUT_ARG_MAX_NUM - 1)) {
            cmd_argv[cmd_argc] = strtok(NULL, "");
            printf("cmd_argv[%d] is set to be: %s \n", cmd_argc, next_token);
            cmd_argc++;
            break;
        }*/
        cmd_argv[cmd_argc] = next_token;
        //printf("cmd_argv[%d] is set to be: %s \n", cmd_argc, next_token);
        cmd_argc++;
        if (cmd_argc ==INPUT_ARG_MAX_NUM - 1  ) {
        next_token = strtok(NULL, "");}
        else {next_token = strtok(NULL, DELIM);}
    }

    return cmd_argc;
}

Client * find_client(char* name, Client* top){
   /*Self written helper function*/
    Client *p;
    for (p = top; p ;p=p->next){
        printf("DEBUG: Looking:%s!\n",p->name);
        int where = find_network_newline(p->name,strlen(p->name));
        if (where>0){ p->name[where]= '\0';}

        if ( strcmp(p->name, name) == 0 ){

            return p;
        }
    }
    return NULL;
}

int find_network_newline (char *buf, int inbuf) {
	 /*Copied from the assginment guide*/
	 int i;
     for (i = 0; i < inbuf - 1; i++)
         if ((buf[i] == '\r') && (buf[i + 1] == '\n'))
           	return i;
	return -1;
}
