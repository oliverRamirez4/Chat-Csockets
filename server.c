#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>

#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <pthread.h>

#include <time.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "messages.h"

#define BUFSIZE 1024


//This is the SERVER 


struct clientsock {
    int sock;
    pthread_t thread;
    struct clientsock* nxt;
    struct clientsock** startptr;
    char* username;
};

pthread_mutex_t ClientListLock;


struct clientsock* addClient(struct clientsock** startptr, struct clientsock** cl, int sock) {
    if(*cl==0) {
        *cl=malloc(sizeof(struct clientsock));
        (*cl)->sock = sock;
        (*cl)->nxt  = 0;
        (*cl)->startptr = startptr;
        return *cl;
    }
    return addClient(startptr, &(*cl)->nxt, sock);
}
void rmClient(struct clientsock** cl, int sock) {
    if(*cl==0) { return; } // end of list without match
    struct clientsock* me = *cl;
    if( me->sock==sock) {
        *cl = (*cl)->nxt;
        free(me);
        return;
    }
}

void broadcast(struct clientsock** cl, char* buf, ssize_t len) {
    struct clientsock* cs = *cl;
    ssize_t bs;
    while(cs!=0) {
        int csock = cs->sock;
        cs = cs->nxt;
        bs = send(csock, buf, len, 0);
        if(bs!=len) {
            pthread_mutex_lock(&ClientListLock);
            rmClient(cs->startptr, csock);
            pthread_mutex_unlock(&ClientListLock);
        }
    }
}

void* handle_client(void* args) {
    struct clientsock* cl = (struct clientsock*)args;
    char* buf = malloc(1024);
    ssize_t br;
    unsigned char msg_type;
    char* username;
    username = malloc(1024);
    printf("Client thread started\n");
    while(1) {
        br = recv(cl->sock, buf, 1024, 0);
        printf("Client data received %d\n",br);
        if(br<1) {
            pthread_mutex_lock(&ClientListLock);
            rmClient(cl->startptr, cl->sock);
            pthread_mutex_unlock(&ClientListLock);
            break;
        }
	
	 //Begin to handle the buf that was recieved
	 
	 //get the message type
	 msg_type = buf[0];
	 
	 //handle the buf based on the message type.

 	 //###HELO###
	 if(msg_type==2) {
           printf("recieved a HELO.\n");
	   
	   
	   unsigned char versionmask = buf[1];
           unsigned char len = buf[2];
	   for(int i = 0;i<(int)len; i++){
	  username[i] = buf[3+i];
	   }
	   

           printf("%s logged in.\n",username);

	   //send a WLCM message back
	   struct array WLCM = make_WLCM();
	   send(cl->sock, WLCM.data, WLCM.size, 0);

	   // send a join message to everybody
	   struct array JOIN = make_JOIN(username);
	   broadcast(cl->startptr, JOIN.data, JOIN.size);
 




	 //###BBYE###
	 }else if(msg_type == 0) {
	    printf("A client has left the server\n");

	    struct array LEAV = make_LEAV(username);
           broadcast(cl->startptr, LEAV.data, LEAV.size);
	    pthread_mutex_lock(&ClientListLock);
            rmClient(cl->startptr, cl->sock);
            pthread_mutex_unlock(&ClientListLock);
	    break;
	
		
	//###MESG###
 	 }else if(msg_type == 4){

	     //managing time
	     unsigned int sec;
	     sec = (unsigned int)time (NULL);
	     //make it the amount of seconds in the day so far
   	     sec = sec %86400;

	     unsigned int *timeint;
       	     timeint = &sec;	     

	    *timeint = htonl(*timeint);

	     uint8_t* bufhelper;
	    bufhelper =(uint8_t*)timeint;

		buf[4] = bufhelper[0];
		buf[5] = bufhelper[1];
		buf[6] = bufhelper[2];
		buf[7] = bufhelper[3];
	
	       //All the server does with a message is change the time stamp
	   //then broadcast back to all of the clients.
	   broadcast(cl->startptr, buf, br);

    } else {
        printf("invalid message number, don't know what to do\n");
    }


       // broadcast(cl->startptr, buf, strlen(buf));
    }
    free(buf);
    free(username);
    printf("Client connection lost...\n");
}



int main(int argc, char** argv) {
    // declared variables
    unsigned short server_port;// the server port number
    struct stat stats;
    int err;

       // Check that we got enough input
    // you need to pass the port when calling the program
    if(argc<2) {
        printf("Usage: %s <port>\n",argv[0]);
        return -1;
    }

    // Did we get a port number
    //
    char* eptr;

    //loads the port number into server_port 
    server_port = (unsigned short)strtol(argv[1], &eptr, 10);
    
    //return if there is an invalid port
    if(*eptr != 0 || server_port<1024 || server_port>65535) {
        printf("Invalid port: %s\n",server_port);
        return -1;
    }
    
    // Declare bookkeeping for end points
    // The sockaddr_in struct has these variables
   	 // short sin_family    //e.g. AF_INET
   	 // unsigned short sin_port    //e.g. 9346
   	 // struct in _addr sin_addr
   	 // char sin_zero[8]
    struct clientsock* clients = 0; // empty linked list
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    int sock;

    int ret = -1;

    // Filling in the information for the socket
    // copies the character 0 to each charcter of the server_addr for its length
    memset(&server_addr, 0, addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(0x7F000001);
    server_addr.sin_port   = htons(server_port);
    //now the server_addr is set up with the port given by the user


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

    // Actually connect to the port
    
    //bind is complicated! 
    //need to call socket before bind which we already did 
    //int bind(int socket, struct sockaddr *address, int address_len);
    //the middle variable specifies which socket we are binding to
    //the length passes the length in
    err = bind(sock, (struct sockaddr*)&server_addr, addr_len);
    if(err<0) {
        perror("Binding to the socket failed");
        close(sock);
        return -1;
    }

    // Make my program listen for new connections
    // via the socket
    // int listen(int socket, int backlog);
    // backlog is the maximum length of the queue for instructions
    err = listen(sock, 10);
    if(err<0) {
        perror("Can't listen on the socket");
        close(sock);
        return -1;
    }
    // The server is now ready for clients
    // create the buff that stores the data from clients
    char* buf = malloc(BUFSIZE);
    unsigned char msg_type;
    int clientcount = 0;

   while(1) {
        addr_len = sizeof(struct sockaddr_in);
        // Setup a socket for just this client
        int csock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
        if(csock<0) {
            break;
        }
        pthread_mutex_lock(&ClientListLock);
        struct clientsock* cc = addClient(&clients, &clients, csock);
        pthread_mutex_unlock(&ClientListLock);
        broadcast(&clients, "New client", 11);
        pthread_create(&cc->thread, NULL, &handle_client, cc);
    }

    close(sock);

    return 0;

}
    
#undef BUFSIZE  
