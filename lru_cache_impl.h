#ifndef LRUCACHEIMPL_H
#define LRUCACHEIMPL_H


#include <linux/string.h>
#include "sketch_util.h"

typedef unsigned long u_int32;
//#include <semaphore.h>

#define KEY_SIZE 50
//#define VALUE_SIZE 100

/*����LRU����Ļ��浥Ԫ*/
typedef struct cacheEntryS {
	char key[KEY_SIZE];   /* ���ݵ�key */
	//char data[VALUE_SIZE];  /* ���ݵ�data */
	//u_int32 sum_bytes;
	int packet_count;
	short flag;

	//sem_t entry_lock;

	struct cacheEntryS *hashListPrev;   /* �����ϣ��ָ�룬 ָ���ϣ�����ǰһ��Ԫ�� */
	struct cacheEntryS *hashListNext;   /* �����ϣ��ָ�룬 ָ���ϣ����ĺ�һ��Ԫ�� */

	struct cacheEntryS *lruListPrev;    /* ����˫������ָ�룬 ָ�������ǰһ��Ԫ�� */
	struct cacheEntryS *lruListNext;    /* ����˫������ָ�룬 ָ�������һ��Ԫ�� */
}cacheEntryS;


/* ����LRU���� */
typedef struct LRUCacheS {
	//sem_t cache_lock;
	int cacheCapacity;  /*���������*/
	//int cacheCapacity_short;
	int lruListSize;    /*�����˫������ڵ����*/
	//int lruListSize_short;

	cacheEntryS **hashMap;  /*����Ĺ�ϣ��*/

	cacheEntryS *lruListHead;   /*�����˫�������ͷ*/
	cacheEntryS *lruListTail;   /*�����˫�������β*/
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
