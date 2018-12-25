#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "str.h"
#include "zmalloc.h"

char* newstr(const char* init) {
	size_t len = (init == NULL) ? 0 : strlen(init);
	int silen = sizeof(struct strinfo);
	void* si = zmalloc(silen + len + 1);
	((struct strinfo*)(si))->len = len;
	((struct strinfo*)(si))->cap = len;
	char* s = (char*)si + silen;
	memcpy(s,init,len);
	s[len] = '\0';
	return s;
}

char* newstrlen(const char* init, size_t len) {
	if(strlen(init) < len) return NULL;
	int silen = sizeof(struct strinfo);
	void* si = zmalloc(silen + len + 1);
	((struct strinfo*)(si))->len = len;
	((struct strinfo*)(si))->cap = len;
	char* s = (char*)si + silen;
	memcpy(s,init,len);
	s[len] = '\0';
	return s;
}

char* emptystr() {
	int silen = sizeof(struct strinfo);
	void* si = zmalloc(silen + 1);
	((struct strinfo*)(si))->len = 0;
	((struct strinfo*)(si))->cap = 4;
	char* s = (char*)si + silen;
	s[0] = '\0';
	return s;
}

void cleanstr(char* s) {
	setlen(s,0);
	s[0] = '\0';
}

void freestr(char* s) {
	if(s == NULL)
		return;
	zfree(s - sizeof(struct strinfo));
}

char* strnumcpy(char* s, const char* n, size_t newlen) {
	size_t silen = sizeof(struct strinfo),newcap;
	void *si, *newsi;
	if(STR_HEAD(s)->cap < newlen) 
	{
		if(newlen > MAX_DOUBLE_MEMORY) {
			newcap = newlen + MAX_DOUBLE_MEMORY;
		} else {
			newcap = newlen * 2;
		}
		si = (char*)s - silen;
		newsi = zrealloc(si,newcap + silen + 1);
		if(newsi == NULL) 
			return NULL;
		s = (char*)newsi + silen;
		setcap(s,newcap);
	}
	memcpy(s,n,newlen + 1);
	setlen(s,newlen);
	return s;
}

char* strnumcat(char* s, const char* n, size_t addlen) {
	size_t silen = sizeof(struct strinfo),newcap,newlen;
	void *si, *newsi;
	newlen = getlen(s) + addlen;
	if(STR_HEAD(s)->cap < newlen) 
	{
		if(newlen > MAX_DOUBLE_MEMORY) {
			newcap = newlen + MAX_DOUBLE_MEMORY;
		} else {
			newcap = newlen * 2;
		}
		si = (char*)s - silen;
		newsi = zrealloc(si,newcap + silen + 1);
		if(newsi == NULL) 
			return NULL;
		s = (char*)newsi + silen;
		setcap(s,newcap);
	}
	memcpy(s + getlen(s),n,addlen);
	s[newlen] = '\0';
	setlen(s,newlen);
	return s;
}

char **parseCommand(const char *line, int* argc) {
    const char *p = line;
    char *current = NULL;
    char **vector = NULL;

    *argc = 0;
    while(1) {
        /* skip blanks */
        while(*p && isspace(*p)) p++;
        if (*p) {
            /* get a token */
            int inq=0;  /* set to 1 if we are in "quotes" */
            int insq=0; /* set to 1 if we are in 'single quotes' */
            int done=0;

            if (current == NULL) current = emptystr();
            while(!done) {
                if (inq) {
                    if (*p == '\\' && *(p+1)) {
                        char c;

                        p++;
                        switch(*p) {
                        case 'n': c = '\n'; break;
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case 'b': c = '\b'; break;
                        case 'a': c = '\a'; break;
                        default: c = *p; break;
                        }
                        current = strnumcat(current,&c,1);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = strnumcat(current,p,1);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p+1) == '\'') {
                        p++;
                        current = strnumcat(current,"'",1);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = strnumcat(current,p,1);
                    }
                } else {
                    switch(*p) {
                    case ' ':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\0':
                        done=1;
                        break;
                    case '"':
                        inq=1;
                        break;
                    case '\'':
                        insq=1;
                        break;
                    default:
                        current = strnumcat(current,p,1);
                        break;
                    }
                }
                if (*p) p++;
            }
            /* add the token to the vector */
            vector = realloc(vector,((*argc)+1)*sizeof(char*));
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        } else {
            /* Even on empty input string return something not NULL. */
            if (vector == NULL) vector = zmalloc(sizeof(void*));
            return vector;
        }
    }

err:
    while((*argc)--)
        freestr(vector[*argc]);
    zfree(vector);
    if (current) freestr(current);
    *argc = 0;
    return NULL;
}

void str_test() {
/*	char* a = newstr("jfowjeife");
	printf("%s len %lu capacity is %lu\n",a,(unsigned long)getlen(a),STR_HEAD(a)->cap);
	a = strnumcpy(a,"jsojwoejofwo",12);
	printf("%s len %lu capacity is %lu\n",a,(unsigned long)getlen(a),STR_HEAD(a)->cap);*/
	char* a = emptystr();
	const char* tmp = "sefewefe\r\n";
	for(int i = 0; i < 4; i++) {
		a = strnumcat(a,tmp,strlen(tmp));
		printf("%s len %lu capacity is %lu len is %lu\n",a,(unsigned long)getlen(a),STR_HEAD(a)->cap,strlen(a));
	}
}

void parse_test() {
	int argc;
	char* test = "get sjogjei gwjoeijgioj gwjoeijgioj gwejoij\n";
	char** result = parseCommand(test, &argc);
	for(int i = 0; i < argc; i++) {
		printf("%s ",result[i]);
	}
	printf("\n");
	test = "123456";
	result = parseCommand(test, &argc);
	for(int i = 0; i < argc; i++) {
		printf("%s ",result[i]);
	}
	printf("\n");
}
