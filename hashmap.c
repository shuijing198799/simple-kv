#include "hashmap.h" 

map* initMap() {
	map* newmap = zmalloc(sizeof(*newmap));
	if(!newmap) {
		return NULL;
	}
	initMapTable(&newmap->mt[0]);
	initMapTable(&newmap->mt[1]);
	newmap->rehashIdx = -1;
	return newmap;
}

/* Destroy an entire dictionary */
void _mapClear(map *m, mapTable *mt) {
    unsigned long i;

    /* Free all the elements */
    for (i = 0; i < mt->size && mt->used > 0; i++) {
        mapEntry *me, *nextMe;

        if ((me = mt->mtable[i]) == NULL) continue;
        while(me) {
            nextMe = me->next;
            freestr(me->key);
            zfree(me);
            mt->used--;
            me = nextMe;
        }
    }
    /* Free the table and the allocated cache structure */
    zfree(mt->mtable);
    /* Re-initialize the table */
    initMapTable(mt);
    return; /* never fails */
}

void destroyMap(map* m) {
	_mapClear(m,&m->mt[0]);
    _mapClear(m,&m->mt[1]);
    zfree(m);
}

void initMapTable(mapTable* table) {
	table->size = INIT_DICT_SIZE;
	table->used = 0;
	table->mtable = NULL;
}

uint64_t mapgetIdx(const char* k,  size_t mask) {
	//printf("mapgetidx begin\n");
	uint64_t idx,h;
	h = crc64(0,(unsigned char*)k,strlen(k));
	idx = h & mask;
	//printf("mapgetidx end idx %lu h %lu\n",idx,h);
	return idx;
}

//implement crc64code
mapEntry* mapfind(const char* k, map* m) {
	//printf("mapfind begin\n");
	if(m->mt[0].used == 0 && m->mt[1].used == 0) return NULL;
	mapEntry* me = NULL;
	uint64_t idx;
	idx = mapgetIdx(k,m->mt[0].size - 1);
	me = m->mt[0].mtable[idx];
	while(me) {
		if(strcmp(me->key,k) == 0)
			return me;
		me = me->next;
	}
	//printf("mapfind end\n");
	return NULL;
}

int mapset(const char* k, void* v, map* m) {
	mapEntry *me;
	//if the table is not init , init the table first.
	if(m->mt[0].mtable == NULL) {
		m->mt[0].mtable = zmalloc(sizeof(mapEntry*) * INIT_DICT_SIZE);
		if(!m->mt[0].mtable) {
			return -1;
		}
		m->mt[0].used = 0;
		m->mt[0].size = INIT_DICT_SIZE;
	} 

	if(m->mt[1].mtable == NULL) {
		m->mt[1].mtable = zmalloc(sizeof(mapEntry*) * INIT_DICT_SIZE);
		if(!m->mt[1].mtable) {
			return -1;
		}
		m->mt[1].used = 0;
		m->mt[1].size = INIT_DICT_SIZE;
	}

	me = mapfind(k,m);
	if(me != NULL) {
		//if key is exist in this dict,just replace the value
		me->val = v;
	} else {
		//if key is not in this dict, create a entry and add value
		me = zmalloc(sizeof(*me));
		if(!me) {
			return -1;
		}
		me->key = newstr(k);
		me->val = v;
		//insert into head of the linked
		uint64_t idx = mapgetIdx(me->key,m->mt[0].size - 1);
		me->next = m->mt[0].mtable[idx];
		m->mt[0].mtable[idx] = me;
		m->mt[0].used++;
	}
	return 0;
}

void* mapget(const char* k, map *m) {
	//printf("mapget begin %s\n",k);
	mapEntry* me;
	me = mapfind(k,m);
	//printf("mapget end %s\n",k);
	return me == NULL ? NULL: me->val;
}

//del function has to be idempotent for this function may used for 
//control error occured.
void* mapdel(char* k, map* m) {
	mapEntry *me,*pre;
	void* ret;
	if(m->mt[0].used == 0 && m->mt[1].used == 0) return NULL;
	uint64_t idx;
	idx = mapgetIdx(k,m->mt[0].size - 1);
	me = m->mt[0].mtable[idx];
	pre = NULL;
	while(me) {
		if(strcmp(me->key,k) == 0)
			break;
		pre = me;
		me = me->next;
	}
	if(!me) return NULL;
	if(pre == NULL) {
		// if me is the head node;
		m->mt[0].mtable[idx] = me->next;
	} else {
		pre->next = me->next;
	}

	//make a back up
	ret = me->val;

	freestr(me->key);
	zfree(me);
	m->mt[0].used--;

	return ret;
}


void hashmap_test() {
	//make a numerous amount test here to test the hashmap
	map* m = initMap();
	//check numerous key insert and delete
	for(int i = 0; i < 1000000; i++) {
		char string[20];
		itoa(i,string);
		mapset(string,string,m);
	}

	for(int i = 0; i < 1000000; i++) {
		char string[20];
		itoa(i,string);
		char* tmp = mapget(string,m);
		printf("%s\n",tmp);
	}

	//checkdelete
	mapset("21","test",m);

	char* tmp = mapget("21",m);
	if(tmp == NULL) 
		printf("mapget null\n");
	else
		printf("mapget %s \n",tmp);

	mapdel("21",m);
	//printf("size %lu\n",m->mt[0].used);
	//printf("size %lu\n",m->mt[0].used);
	tmp = mapget("21",m);
	if(tmp == NULL) 
		printf("mapget null\n");
	else
		printf("mapget %s \n",tmp);
}
