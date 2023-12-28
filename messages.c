#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "messages.h"


struct array make_HELO(char* username) {
    struct array ret; // I'll need to return something, declare here

    unsigned char msg_type = 2;
    unsigned char version  = 0;
    unsigned char namelen  = strlen(username);
    // username comes in as an argument

    // Need some memory to pack with the data... (set up the initial array)
    ret.size = 3+namelen;
    ret.data = calloc( ret.size , sizeof(char));
    ret.length = ret.size;

    ret.data[0] = msg_type;
    ret.data[1] = version;
    ret.data[2] = namelen;
    for(int i=0; i<namelen; i++) {
        ret.data[3+i] = username[i];
    }

    return ret;
}

struct array make_MESG(char* message, char* username) {
    struct array ret; // I'll need to return something, declare here

    unsigned char msg_type = 4;
    unsigned short  msglength  = strlen(message);
    unsigned char namelength = strlen(username);
    unsigned int timestamp = 0;
    // username comes in as an argument

    // Need some memory to pack with the data... (set up the initial array)
    ret.size = (8 + msglength + namelength);
    ret.data = calloc( ret.size , sizeof(char));
    ret.length = ret.size;

    ret.data[0] = msg_type;
    ret.data[1] = namelength;
    ret.data[2] = msglength;//concern does this stor both bytes of the short
    ret.data[4] = timestamp;

    //put the username in the buf
    for(int i=0; i<namelength; i++){
            ret.data[8+i] = username[i];
    }
    //put the message in the buf
    for(int i=0; i<msglength; i++) {
        ret.data[8 + namelength + i] = message[i];
    }

    return ret;
}


struct array make_BBYE() {
         struct array ret; // I'll need to return something, declare here

    unsigned char msg_type = 0;

    ret.size = 1;
    ret.data = calloc(ret.size, sizeof(char));
    ret.length = ret.size;


    ret.data[0] = msg_type;

    return ret;
}

struct array make_WLCM() {
         struct array ret; // I'll need to return something, declare here

    unsigned char msg_type = 3;
    unsigned char version  = 0;

    ret.size = 2;
    ret.data = calloc(ret.size, sizeof(char));
    ret.length = ret.size;


    ret.data[0] = msg_type;
    ret.data[1] = version;

    return ret;
}

struct array make_ERRR(char* errormsg) {
         struct array ret; // I'll need to return something, declare here

    unsigned char msg_type = 1;
    unsigned char msglen = strlen(errormsg);

    ret.size = 2 + msglen;
    ret.data = calloc(ret.size, sizeof(char));
    ret.length = ret.size;


    ret.data[0] = msg_type;
    ret.data[1] = msglen;

    for(int i=0; i<msglen; i++) {
        ret.data[2+i] = errormsg[i];
    }


    return ret;
}

struct array make_JOIN(char* username) {
         struct array ret; // I'll need to return something, declare here

    unsigned char msg_type = 6;
    unsigned char msglen = strlen(username);

    ret.size = 2 + msglen;
    ret.data = calloc(ret.size, sizeof(char));
    ret.length = ret.size;


    ret.data[0] = msg_type;
    ret.data[1] = msglen;

    for(int i=0; i<msglen; i++) {
        ret.data[2+i] = username[i];
    }


    return ret;
}

struct array make_LEAV(char* username) {
         struct array ret; // I'll need to return something, declare here

    unsigned char msg_type = 7;
    unsigned char msglen = strlen(username);

    ret.size = 2 + msglen;
    ret.data = calloc(ret.size, sizeof(char));
    ret.length = ret.size;


    ret.data[0] = msg_type;
    ret.data[1] = msglen;

    for(int i=0; i<msglen; i++) {
        ret.data[2+i] = username[i];
    }


    return ret;
}
