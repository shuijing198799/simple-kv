#ifndef __HASHMAP_H
#define __HAHSMAP_H

#include "str.h"
#include "hash.h"
#include "skiplist.h"
#include "tools.h"
#include "zmalloc.h"

#define INIT_DICT_SIZE 1024 *1024 * 64
#define RESIZE_FACOTR 3

//this datastruct is a hashmap,use 
//link method to resolve the conflict.
//the initialize size of hashmap is 4,
//when the node num is larger than size of
//hashmap multi factor, this map process resize.

typedef struct mapEntry {
	char* key;
	void* val;
	struct mapEntry* next;
}mapEntry;

typedef struct mapTable {
	struct mapEntry** mtable;
	size_t size;
	size_t used;
}mapTable;

typedef struct map {
	mapTable mt[2];
	long rehashIdx;
}map;

mapEntry* mapfind(const char* k, map* m);
void* mapget(const char* k, map* m);
int mapset(const char* k,void* v,map* m);
void* mapdel(char* k, map* m);
map* initMap();
void destroyMap(map* m);
void initMapTable(mapTable* table);
void hashmap_test();


#endif
