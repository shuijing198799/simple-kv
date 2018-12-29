#ifndef __STR_H
#define __STR_H

#include <stdio.h>
#include <stdbool.h>
#include "tools.h"
#include "zmalloc.h"

#define STR_HEAD(s) ((struct strinfo *)((s)-(sizeof(struct strinfo))))

//expand str memeory dynamically, 
//if the memory is large than 1M,add 1M  every time, 
//otherwise * 2 erveytime.
#define MAX_DOUBLE_MEMORY 1024 * 1024

//the struct of str
//cap is the allocated memory
//len is the length of str
//buf is the buf
struct strinfo {
	size_t len;
	size_t cap;
	char buf[];
};

//create a new str with length
char* newstr(const char* init);
//empty a str with length
void cleanstr(char* s);
//free str memory
void freestr(char*s);
//copy function
char* strnumcpy(char* s,const char* n, size_t len);
//init a empty str
char* emptystr();
//init str via len
char* newstrlen(const char* init, size_t len);

char* strnumcat(char* s, const char* n, size_t len);

//parse command to args
char **parseCommand(const char *line, int* argc);

//tests
void parse_test();
void str_test();

//set str length
static inline void setlen(const char* s, int newlen) {
	STR_HEAD(s)->len = newlen;
}


//set str length
static inline void setcap(const char* s, int newcap) {
	STR_HEAD(s)->cap = newcap;
}

//get str length
static inline size_t getlen(const char* s) {
	return STR_HEAD(s)->len;
}


//test asset and print info
static inline void test_cond(const char* s,bool c) {
	if(c) {
		printf("test %s passed\n",s);
	} else {
		printf("test %s failed\n",s);
	}
}

#endif
