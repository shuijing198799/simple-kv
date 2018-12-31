#include "str.h"

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
	char* test = newstr("test");

	//get length of str and test whether the str is equal to my imagination
	test_cond("create str and test the length and detial of str",
			getlen(test) == 4 && strcmp(test,"test\0") == 0);

	//test strnumcpy
	char* newstr = "newtest";
	test = strnumcpy(test,newstr,strlen(newstr));
	test_cond("test strcmp with length",
			getlen(test) == 7 && strcmp(test,"newtest\0") == 0);

	//test clean str
	cleanstr(test);
	test_cond("test clean str",
			getlen(test) == 0 && strcmp(test,"\0") == 0);

	//test strcat with len
	test = strnumcat(test,"test",strlen("test"));
	test_cond("test strcat with len",
				getlen(test) == 4 && strcmp(test,"test\0") == 0);

	//test create a empty str.
	test = emptystr();
	test_cond("test empty str",
			getlen(test) == 0 && strcmp(test,"\0") == 0);

	//test parsecommand
	parse_test();

}

void parse_test() {
	int argc;
	char* test = "get sj*gji gwjijgioj g12oe~&jgioj gwejoij\n";
	char** result = parseCommand(test, &argc);
	test_cond("test parse numerous command with some symbol",
			argc == 5);
	test = "";
	result = parseCommand(test, &argc);
	test_cond("test parse empty command with some symbol",
			argc == 0);

	test = "get";
	result = parseCommand(test, &argc);
	test_cond("test parse one command with some symbol",
			argc == 1);
}
