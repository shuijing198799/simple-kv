/*
* this demo use hashmap and skiplist to handler the
* set, get, del, scan request. hashmap store the key
* and the skiplist node. skiplist store the key and val.
* when use get method, the time complexity is O(1), del
* method time complexity is O(1), set is skiplist complexity
* O(log2n) 'via the skiplist introduction' and scan skipList
* complexity is O(n). this implemention is consider the zset 
* in redis
*/
#ifndef __SORTMAP_H
#define __SORTMAP_H

#include "hashmap.h"
#include "skiplist.h"

#define getsknval(ptr) ((skipnode*)(ptr))->val

typedef struct sortmap {
	skiplist* skl;
	map* m;
}sortmap;

sortmap* initSortMap();
void destroySortMap(sortmap* sm);
char* get(char* key,sortmap* sm);
int set(char* key, char* val, sortmap* sm);
int del(char* key, sortmap* sm);
char* scan(sortmap* map);

#endif
