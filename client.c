#include <stdio.h>

#include <stdlib.h>

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <netinet/in.h>
#include <pthread.h>

#include "messages.h"

#define BUFSIZE 1024


struct threadargs {
    int sock;
    char* buf;
    pthread_t handler;
};

void* handle_server(void* a) {
    struct threadargs* args = (struct threadargs*)a;
    char* buf = args->buf;
    int sock = args->sock;
    ssize_t br;
    unsigned char msg_type;

    while(1) {
         br = recv(sock, buf, BUFSIZE, 0);
        if(br<0) {
            printf("Can't receive from server\n");
            break;
        }
	//begin to handle the buf that was revieved
 	msg_type = buf[0];
   		
	
	//###WLCM###
    	if(msg_type == 3){
   		printf("you have joined the chat server\n");
		printf("enter \\q to leave the chat\n");
   
    	}else if(msg_type == 4){
		char* toprint;
		char* message;
		char* username;
		unsigned char namelen = buf[1];
		unsigned short msglen = buf[2];//concern does this get the full short
		unsigned int* timeint;


		timeint = (unsigned int*)&buf[4];
		*timeint = htonl(*timeint);
	

		username = calloc(sizeof(char),namelen);
		message = calloc(sizeof(char), msglen);
		toprint = calloc(sizeof(char),10 + namelen +msglen);


		 for (int i = 0; i < namelen; i++){
                        username[i]=buf[8 + i];
                }
		for (int i = 0; i < msglen; i++){
                        message[i]=buf[8 + namelen + i];
                }
		
		strcat(toprint, username);
		strcat(toprint, ": ");
		strcat(toprint, message);

		printf("%u:%u:%u  %s\n",(*timeint / 3600), (*timeint % 3600 / 60), (*timeint % 60), toprint);

		free(username);
		free(message);
		free(toprint);

	}else if(msg_type == 6){
		char* username;
		char* toprint;
		unsigned char namelen = buf[1];
		username = calloc(sizeof(char),namelen);
		toprint = calloc(sizeof(char), 20 +namelen);

		for (int i = 0; i < namelen; i++){
                        username[i]=buf[2+i];
                }

		strcpy(toprint, username);
		strcat(toprint, " joined the chat.");

		printf("%s\n", toprint);

                free(username);
                free(toprint);


	}else if(msg_type == 7){
                char* username;
                char* toprint;
                unsigned char namelen = buf[1];
                username = calloc(sizeof(char),namelen);
                toprint = calloc(sizeof(char), 20 +namelen);

                for (int i = 0; i < namelen; i++){
                        username[i]=buf[2+i];
                }

                strcpy(toprint, username);
                strcat(toprint, " left the chat.");

                printf("%s\n", toprint);

                free(username);
                free(toprint);


       	}else{
	}
    }
    free(buf);
    args->buf=0;
    close(sock);
    
}
int main(int argc, char** argv) {
    struct threadargs args;
    unsigned short server_port;
    int err;
    

      // check that the input is long enough
    // the program needs to take in arguments.
    // first the port number then the filename
       if(argc<2) {
        printf("Usage: %s <port>\n",argv[0]);
        return -1;
    }



    // validate input
    char* eptr;
    //put the port number passed in into the server_port variable
    server_port = (unsigned short)strtol(argv[1], &eptr, 10);
    if(*eptr != 0) {
        printf("Invalid port: %s\n",server_port);
        return -1;
    }


    //struct sockaddr_in {
    	//short            sin_family;   // e.g. AF_INET
    	//unsigned short   sin_port;     // e.g. htons(3490)
    	//struct in_addr   sin_addr;     // see struct in_addr, below
    	//char             sin_zero[8];  // zero this if you want to
    
    // declare some bookkeeping vars
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    int sock;

    // Network info for where to connect to
    memset(&server_addr, 0, addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(0x7F000001);
    server_addr.sin_port   = htons(server_port);

    // Generate the socket
    //int socket(int domain, int type, int protocol);
    //AF_INET is the IPv4 internet protocols
    //SOCK_STREAM Provides sequenced, reliable, two-way, connection-based
    // byte streams. 
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0) {
        perror("Could not get socket from OS");
        return -1;
    }


    //Get the username and interact with the client
    printf("welcome to Oliver's chat program!\n");
    printf("What is your username that you would like to connect with?\n");

    char* username;
    username = malloc(100);
    scanf("%s",username);
    printf("Conecting...\n");

    // Actually make the connection to the server
    //int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    //socket
    	//Specifies the file descriptor associated with the socket.
    //address
	//Points to a sockaddr structure containing the peer address. The length and format
	//of the address depend on the address family of the socket.
    //address_len
	//Specifies the length of the sockaddr structure pointed to by the address argument.
    err = connect(sock, (struct sockaddr*)&server_addr, addr_len);
    if(err<0) {
        perror("Connect to the server failed");
        close(sock);
        return -1;
    }
    // The client is connected to the server
    
    //create the thread to handle the server
    args.buf = malloc(BUFSIZE);
    args.sock = sock;
    pthread_create(&args.handler, 0, &handle_server, &args);
    
    char* buf = 0;
    size_t buflen=0;
    ssize_t bs;


    struct array HELO = make_HELO(username);
    send(sock, HELO.data, HELO.size, 0);

    while(1) {
        ssize_t br = getline(&buf, &buflen, stdin);
        if(buf[0]=='\\' && buf[1]=='q') {
		struct array BBYE = make_BBYE();
            printf("Quiting...\n");
	    send(sock, BBYE.data, BBYE.size, 0);
            break;
        }
        buf[br-1] = 0;
	struct array MESG = make_MESG(buf, username);
        bs = send(sock, MESG.data, MESG.size, 0);
        if(bs<br-1) {
            printf("Socket unexpectedly broke...\n");
            break;
        }
    }
    close(sock);
    pthread_cancel(args.handler);
    pthread_join(args.handler,0);

    free(buf);
    free(username);
    free(args.buf);

    return 0;
}

    
#undef BUFSIZE
