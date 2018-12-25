#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "skiplist.h"
#include "str.h"
#include "tools.h"
#include "zmalloc.h"

struct skipnode *newskn(int level,char* key,char* val) {
	struct skipnode *node;
	node=zmalloc(sizeof(*node) + level * sizeof(struct sk_link));
	if (node != NULL) {
		node->key=newstr(key);
		node->val=newstr(val);
		node->level = level;
	}
	return node;
}

void delskn(struct skipnode *node) {
	freestr(node->key);
	freestr(node->val);
	zfree(node);
}

struct skiplist *newskl(void) {
	int i;
	struct skiplist *list=zmalloc(sizeof(*list));
	if (list != NULL) {
		list->level=1;
		list->count=0;
		for (i=0; i < sizeof(list->head) / sizeof(list->head[0]); i++) {
			list_init(&list->head[i]);
		}
	}
	return list;
}

void skldel(struct skiplist *list) {
	struct sk_link *n;
	struct sk_link *pos=list->head[0].next;
	skiplistforeach_safe(pos,n,&list->head[0]) {
		struct skipnode *node=list_entry(pos,struct skipnode,link[0]);
		delskn(node);
	}
	zfree(list);
}

int randoml(void) {
	int level=1;
	const double p=0.25;
	while ((random() & 0xffff) < 0xffff * p) {
		level++;
	}
	return level > MAX_LEVEL ? MAX_LEVEL : level;
}

struct skipnode *sklget(char* key,struct skiplist *list) {
	struct skipnode *node;
	int i=list->level - 1;
	struct sk_link *pos=&list->head[i];
	struct sk_link *end=&list->head[i];

	for (; i >= 0; i--) {
		pos=pos->next;
		skiplistforeach(pos,end) {
			node=list_entry(pos,struct skipnode,link[i]);
			if (strcmp(node->key,key) >= 0) {
				end=&node->link[i];
				break;
			}
		}
		if (strcmp(node->key,key) == 0) {
			return node;
		}
		pos=end->prev;
		pos--;
		end--;
	}

	return NULL;
}

struct skipnode * sklset(char* key,char* val,struct skiplist *list) {
	int level=randoml();
	if (level > list->level) {
		list->level=level;
	}

	struct skipnode *node=newskn(level,key,val);
	if (node != NULL) {
		int i=list->level - 1;
		struct sk_link *pos=&list->head[i];
		struct sk_link *end=&list->head[i];

		for (; i >= 0; i--) {
			pos=pos->next;
			skiplistforeach(pos,end) {
				struct skipnode *nd=list_entry(pos,struct skipnode,link[i]);
				if (strcmp(nd->key,key) > 0) {
					end=&nd->link[i];
					break;
				} else if (strcmp(nd->key,key) == 0) {
					strnumcpy(nd->val,val,strlen(val));
					return nd;
				}
			}
			pos=end->prev;
			if (i < level) {
				__list_add(&node->link[i],pos,end);
			}
			pos--;
			end--;
		}

		list->count++;
	}
	return node;
}

void _remove(struct skipnode *node,int level,struct skiplist *list) {
	int i;
	for (i=0; i < level; i++) {
		list_del(&node->link[i]);
		if (list_empty(&list->head[i])) {
			list->level--;
		}
	}
	delskn(node);
	list->count--;
}

void sklrm(char* key,struct skiplist *list) {
	struct sk_link *n;
	struct skipnode *node;
	int i=list->level - 1;
	struct sk_link *pos=&list->head[i];
	struct sk_link *end=&list->head[i];

	for (; i >= 0; i--) {
		pos=pos->next;
		skiplistforeach_safe(pos,n,end) {
			node=list_entry(pos,struct skipnode,link[i]);
			if (strcmp(node->key,key) > 0) {
				end=&node->link[i];
				break;
			} else if (strcmp(node->key,key)==0) {
				/* we allow nodes with same key. */
				_remove(node,i + 1,list);
				return;
			}
		}
		pos=end->prev;
		pos--;
		end--;
	}
}

//this function should wirte numerous memory check
//TODO: change str implementation to accomplish
char* sklscan(struct skiplist *list) {
	struct skipnode *node;
	struct sk_link *pos = &list->head[0];
	struct sk_link *end = &list->head[0];
	char* result = emptystr();

	pos = pos->next;
	skiplistforeach(pos, end) {
		node = list_entry(pos, struct skipnode, link[0]);
		result = strnumcat(result,"key:",4);
		result = strnumcat(result,node->key,getlen(node->key));
		result = strnumcat(result," val:",5);
		result = strnumcat(result,node->val,getlen(node->val));
		result = strnumcat(result,"\n",1);
	}
//	printf("%s\n",result);
	return result;
}

void sklrmnode(skipnode* node,struct skiplist *list) {
	_remove(node,node->level,list);
}

void skiplist_test() {
	struct skiplist* skl = newskl();
/*	sklset("22","33",skl);
	sklset("33","44",skl);
	sklset("45","42",skl);
	sklset("33","24",skl);*/
	for(int i = 0; i < 100; i++) {
		char string[20];
		itoa(i,string);
		sklset(string,string,skl);
	}
	char* result = emptystr();
	result = sklscan(skl);
	printf("%s",result);
	freestr(result);
}

