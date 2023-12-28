#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct array {
    int length;
    int size;
    char* data;
};


struct array make_HELO(char* username);

struct array make_MESG(char* message, char* username);

struct array make_BBYE();

struct array make_WLCM();

struct array make_ERRR(char* errormsg);

struct array make_JOIN(char* username);

struct array make_LEAV(char* username);
