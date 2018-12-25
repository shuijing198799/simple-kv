/*use an malloc module here for centrliaze alloc 
 * and free memories. it can handler the oom exception
 * and maybe we can statistics the memory this porcess
 * has used later?
 */

#ifndef __ZMALLOC_H
#define __ZMALLOC_H

void *zmalloc(size_t size);
void *zcalloc(size_t size);
void *zrealloc(void *ptr, size_t size);
void zfree(void *ptr);

//TODO use used_memory in all malloc and free to statistic
//how many memory have been used.
static size_t used_memory;

#endif
