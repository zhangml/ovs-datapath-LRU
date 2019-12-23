#ifndef LRUCACHEIMPL_H
#define LRUCACHEIMPL_H


#include <linux/string.h>
#include "sketch_util.h"

typedef unsigned long u_int32;
//#include <semaphore.h>

#define KEY_SIZE 50
//#define VALUE_SIZE 100

/*定义LRU缓存的缓存单元*/
typedef struct cacheEntryS {
	char key[KEY_SIZE];   /* 数据的key */
	//char data[VALUE_SIZE];  /* 数据的data */
	//u_int32 sum_bytes;
	int packet_count;
	short flag;

	//sem_t entry_lock;

	struct cacheEntryS *hashListPrev;   /* 缓存哈希表指针， 指向哈希链表的前一个元素 */
	struct cacheEntryS *hashListNext;   /* 缓存哈希表指针， 指向哈希链表的后一个元素 */

	struct cacheEntryS *lruListPrev;    /* 缓存双向链表指针， 指向链表的前一个元素 */
	struct cacheEntryS *lruListNext;    /* 缓存双向链表指针， 指向链表后一个元素 */
}cacheEntryS;


/* 定义LRU缓存 */
typedef struct LRUCacheS {
	//sem_t cache_lock;
	int cacheCapacity;  /*缓存的容量*/
	//int cacheCapacity_short;
	int lruListSize;    /*缓存的双向链表节点个数*/
	//int lruListSize_short;

	cacheEntryS **hashMap;  /*缓存的哈希表*/

	cacheEntryS *lruListHead;   /*缓存的双向链表表头*/
	cacheEntryS *lruListTail;   /*缓存的双向链表表尾*/
}LRUCacheS;


#define THRESHOLD 3

#define MAX_ETH_FRAME 1514
#define ERROR_FILE_OPEN_FAILED -1
#define ERROR_MEM_ALLOC_FAILED -2
#define ERROR_PCAP_PARSE_FAILED -3
#define BUFSIZE 10240
#define STRSIZE 1024

#define MAX_SIZE 80000

typedef unsigned short  u_short;



extern cacheEntryS Queue[MAX_SIZE];
extern int count;
extern void *lruCache;
extern cacheEntryS *entryPointer;


extern const int L;






cacheEntryS *newCacheEntry(char *key);
void freeCacheEntry(cacheEntryS* entry);
int LRUCacheCreate(int capacity, void **lruCache);
int LRUCacheDestory(void *lruCache);
void removeFromList(LRUCacheS *cache, cacheEntryS *entry);
cacheEntryS* insertToListHead(LRUCacheS *cache, cacheEntryS *entry);
void freeList(LRUCacheS* cache);
void updateLRUList(LRUCacheS *cache, cacheEntryS *entry);
unsigned int hashKey(LRUCacheS *cache, char* key);
cacheEntryS *getValueFromHashMap(LRUCacheS *cache, char *key);
void insertentryToHashMap(LRUCacheS *cache, cacheEntryS *entry);
void removeEntryFromHashMap(LRUCacheS *cache, cacheEntryS *entry);
cacheEntryS* LRUCacheSet(void *lruCache, char *key, cacheEntryS **pEntryPointer);
char *LRUCacheGet(void *lruCache, char *key);

int LRUCacheLength(void *lruCache);

int Judge(void *lruCache, char *key, int data);



#endif 
