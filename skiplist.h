#ifndef _skiplistH
#define _skiplistH

#include "str.h"

#define list_entry(ptr, type, member) \
        ((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))

#define skiplistforeach(pos, end) \
        for (; pos != end; pos=pos->next)

#define skiplistforeach_safe(pos, n, end) \
        for (n=pos->next; pos != end; pos=n, n=pos->next)

#define MAX_LEVEL 32  /* Should be enough for 2^32 elements */

struct sk_link {
        struct sk_link *prev, *next;
};

static inline void list_init(struct sk_link *link) {
        link->prev=link;
        link->next=link;
}

static inline void __list_add(struct sk_link *link, struct sk_link *prev, struct sk_link *next) {
        link->next=next;
        link->prev=prev;
        next->prev=link;
        prev->next=link;
}

static inline void __list_del(struct sk_link *prev, struct sk_link *next) {
        prev->next=next;
        next->prev=prev;
}

static inline void list_add(struct sk_link *link, struct sk_link *prev) {
        __list_add(link, prev, prev->next);
}

static inline void list_del(struct sk_link *link) {
        __list_del(link->prev, link->next);
        list_init(link);
}

static inline int list_empty(struct sk_link *link) {
        return link->next == link;
}

typedef struct skiplist {
        int level;
        int count;
        struct sk_link head[MAX_LEVEL];
}skiplist;

typedef struct skipnode {
        char* key;
    	char* val;
		int level;
        struct sk_link link[0];
}skipnode;

struct skipnode *newskn(int level, char* key, char* val);
void delskn(struct skipnode *node);
struct skiplist *newskl(void);
void skldel(struct skiplist *list);
int randoml(void);
struct skipnode *sklget(char* key, struct skiplist *list);
struct skipnode * sklset(char* key, char* val, struct skiplist *list);
void _remove(struct skipnode *node, int level, struct skiplist *list);
void sklrm(char* key, struct skiplist *list);
char* sklscan(struct skiplist *list);
void skiplist_test();
void sklrmnode(skipnode* node, struct skiplist *list);

#endif
