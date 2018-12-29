#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sortmap.h"

sortmap* initSortMap() {
	sortmap* sortmap = zmalloc(sizeof(*sortmap));
	sortmap->skl = newskl();
	sortmap->m = initMap();
	if(!sortmap || !sortmap->skl || !sortmap->m) goto err;
	return sortmap;
err:
	if(sortmap) {
		destroySortMap(sortmap);
	}
}

void destroySortMap(sortmap* sm) {
	destroyMap(sm->m);
	skldel(sm->skl);
	zfree(sm);
}

char* get(char* key,sortmap* sm) {
	void* ptr = mapget(key,sm->m);
	if(ptr == NULL) 
		return NULL;
//	printf("get val key %s val %s\n",key,((skipnode*)ptr)->val);
	return ((skipnode*)ptr)->val;
}

int set(char* key, char* val, sortmap* sm) {
	mapEntry* me = mapfind(key,sm->m);
	if(!me) {
		skipnode* node = sklset(key,val,sm->skl);
		mapset(key,(void*)node,sm->m);
	} else {
		getsknval(me->val) = strnumcpy(getsknval(me->val),val,strlen(val));
	}
	
	return 0;
}

// remember test delete null value.
int del(char* key, sortmap* sm) {
	skipnode* node = (skipnode*)mapdel(key,sm->m);
	if(!node)
		return -1;
	else
		sklrmnode(node,sm->skl);
	return 0;
}

char* scan(sortmap* sm) {
	return sklscan(sm->skl);
}

void sortmap_test() {
	sortmap* sm = initSortMap();
	//test set and get function
	set("test","test1",sm);
	test_cond("test sortmap set and get",
			strlen(get("test",sm)) == 5 && strcmp(get("test",sm),"test1\0") == 0);

	//test del and get null
	del("test",sm);
	test_cond("test sortmap del and get",get("test",sm) == NULL);

	//test del invalid key
	del("nokey",sm);
	test_cond("del none key",true);

	//test scan 1 key
	set("test1","test1",sm);
	test_cond("test scan 1 key",
			strlen(scan(sm)) == 20 && strcmp(scan(sm),"key:test1 val:test1\n\0") == 0);
}
