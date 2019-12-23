#include "lru_cache_impl.h"

cacheEntryS Queue[MAX_SIZE];
int count = 0;
void *lruCache;
cacheEntryS *entryPointer = NULL;


const int L = 6200;

cacheEntryS* newCacheEntry(char *key)
{

	cacheEntryS* entry = new(cacheEntryS);
	if (NULL == entry ) {
		printk("error!entry cannot be init!\n");
		return NULL;
	}
	memset(entry, 0, sizeof(*entry));
	strncpy(entry->key, key, KEY_SIZE);
	entry->flag = 0;
	//entry->sum_bytes = data;
	entry->packet_count = 1;
	//entry->curr_bytes = data;
	//strncpy(entry->data, data, VALUE_SIZE);
	return entry;
}

/* ÊÍ·ÅÒ»žö»ºŽæµ¥Ôª */
void freeCacheEntry(cacheEntryS* entry)
{
	if (NULL == entry) return;
	kfree(entry);
}

/* ŽŽœšÒ»žöLRU»ºŽæ */
int LRUCacheCreate(int capacity, void **lruCache)
{
	LRUCacheS* cache = NULL;
	if (NULL == (cache = new(LRUCacheS))) {
		printk("error!\n");
		return -1;
	}
	memset(cache, 0, sizeof(*cache));
	cache->cacheCapacity = capacity;
	cache->hashMap = newarr(cacheEntryS*, capacity);
	if (NULL == cache->hashMap) {
		kfree(cache);
		printk("error!\n");
		return -1;
	}
	memset(cache->hashMap, 0, sizeof(cacheEntryS*)*capacity);
	*lruCache = cache;
	return 0;
}

/* ÊÍ·ÅÒ»žöLRU»ºŽæ */
int LRUCacheDestory(void *lruCache)
{
	LRUCacheS* cache = (LRUCacheS*)lruCache;
	if (NULL == cache) return 0;
	//kfree hashMap
	if (cache->hashMap) {
		kfree(cache->hashMap);
	}
	//kfree linklist
	freeList(cache);
	//kfree(cache);
	return 0;
}

/**********************************
* Ë«ÏòÁŽ±íÏà¹ØœÓ¿ÚŒ°ÊµÏÖ
***********************************/
/* ŽÓË«ÏòÁŽ±íÖÐÉŸ³ýÖž¶šœÚµã */
void removeFromList(LRUCacheS *cache, cacheEntryS *entry)
{
	/*ÁŽ±íÎª¿Õ*/
	if (cache->lruListSize == 0) {
		return;
	}

	if (entry == cache->lruListHead && entry == cache->lruListTail) {
		/* ÁŽ±íÖÐœöÊ£µ±Ç°Ò»žöœÚµã */
		//P(&cache->cache_lock);
		cache->lruListHead = cache->lruListTail = NULL;
		//V(&cache->cache_lock);
	}
	else if (entry == cache->lruListHead) {
		/*ÓûÉŸ³ýœÚµãÎ»ÓÚ±íÍ·*/
		//P(&cache->cache_lock);
		cache->lruListHead = entry->lruListNext;
		cache->lruListHead->lruListPrev = NULL;
		//V(&cache->cache_lock);
	}
	else if (entry == cache->lruListTail) {
		/*ÓûÉŸ³ýœÚµãÎ»ÓÚ±íÎ²*/
		//P(&cache->cache_lock);
		cache->lruListTail = entry->lruListPrev;
		cache->lruListTail->lruListNext = NULL;
		//V(&cache->cache_lock);
	}
	else {
		/*ÆäËû·Ç±íÍ·±íÎ²Çé¿ö£¬Ö±œÓÕª³­œÚµã*/
		//P(&cache->cache_lock);
		entry->lruListPrev->lruListNext = entry->lruListNext;
		entry->lruListNext->lruListPrev = entry->lruListPrev;
		//V(&cache->cache_lock);
	}
	/*ÉŸ³ý³É¹Š£¬ ÁŽ±íœÚµãÊýŒõ1*/
	//P(&cache->cache_lock);
	cache->lruListSize--;
	//V(&cache->cache_lock);
}

/*œ«œÚµã²åÈëµœÁŽ±í±íÍ·*/
cacheEntryS* insertToListHead(LRUCacheS *cache, cacheEntryS *entry)
{
	cacheEntryS *removedEntry = NULL;
	//P(&cache->cache_lock);
	++cache->lruListSize;
	//V(&cache->cache_lock);

	if (cache->lruListSize > cache->cacheCapacity) {
		/*Èç¹û»ºŽæÂúÁË£¬ ŒŽÁŽ±íµ±Ç°œÚµãÊýÒÑµÈÓÚ»ºŽæÈÝÁ¿£¬
		ÄÇÃŽÐèÒªÏÈÉŸ³ýÁŽ±í±íÎ²œÚµã£¬ ŒŽÌÔÌ­×îŸÃÃ»ÓÐ±»·ÃÎÊµœµÄ»ºŽæÊýŸÝµ¥Ôª*/
		removedEntry = cache->lruListTail;

		removeFromList(cache, cache->lruListTail);
	}

	if (cache->lruListHead == NULL && cache->lruListTail == NULL) {
		/*Èç¹ûµ±Ç°ÁŽ±íÎª¿ÕÁŽ±í*/
		//P(&cache->cache_lock);
		cache->lruListHead = cache->lruListTail = entry;
		//V(&cache->cache_lock);
	}
	else {
		//P(&cache->cache_lock);
		/*µ±Ç°ÁŽ±í·Ç¿Õ£¬ ²åÈë±íÍ·*/
		entry->lruListNext = cache->lruListHead;
		entry->lruListPrev = NULL;
		cache->lruListHead->lruListPrev = entry;
		cache->lruListHead = entry;
		//V(&cache->cache_lock);
	}
    //printk("Remove: %d, %s\n", cache->lruListSize, removedEntry->key);
	return removedEntry;
}

/*ÊÍ·ÅÕûžöÁŽ±í*/
void freeList(LRUCacheS* cache)
{
	cacheEntryS *entry = cache->lruListHead;
	/*ÁŽ±íÎª¿Õ*/
	if (0 == cache->lruListSize) {
		return;
	}

	
	/*±éÀúÉŸ³ýÁŽ±íÉÏËùÓÐœÚµã*/
	while (entry) {
		cacheEntryS *temp = entry->lruListNext;
		freeCacheEntry(entry);
		entry = temp;
	}
	cache->lruListSize = 0;
}

/*žšÖúÐÑœÓ¿Ú£¬ ÓÃÓÚ±£Ö€×îœü·ÃÎÊµÄœÚµã×ÜÊÇÎ»ÓÚÁŽ±í±íÍ·*/
void updateLRUList(LRUCacheS *cache, cacheEntryS *entry)
{
	/*œ«œÚµãŽÓÁŽ±íÖÐÕª³­*/
	removeFromList(cache, entry);
	/*œ«œÚµã²åÈëÁŽ±í±íÍ·*/

	insertToListHead(cache, entry);
}

/*********************************************************************************
* ¹þÏ£±íÏà¹ØœÓ¿ÚÊµÏÖ
**********************************************************************************/

/*¹þÏ£º¯Êý*/
unsigned int hashKey(LRUCacheS *cache, char* key)
{
	unsigned int len = strlen(key);
	unsigned int b = 378551;
	unsigned int a = 63689;
	unsigned int hash = 0;
	unsigned int i = 0;
	for (i = 0; i < len; key++, i++)
	{
		hash = hash * a + (unsigned int)(*key);
		a = a * b;
	}
	return hash % (cache->cacheCapacity);
}

/*ŽÓ¹þÏ£±í»ñÈ¡»ºŽæµ¥Ôª*/
cacheEntryS *getValueFromHashMap(LRUCacheS *cache, char *key)
{
	/*1.Ê¹ÓÃ¹þÏ£º¯Êý¶šÎ»ÊýŸÝŽæ·ÅÓÚÄÄžö²Û*/
	cacheEntryS *entry = cache->hashMap[hashKey(cache, key)];

	/*2.±éÀú²éÑ¯²ÛÄÚÁŽ±í£¬ ÕÒµœ×ŒÈ·µÄÊýŸÝÏî*/
	while (entry) {
		if (!strncmp(entry->key, key, KEY_SIZE)) {
			break;
		}
		entry = entry->hashListNext;
	}

	return entry;
}

/*Ïò¹þÏ£±í²åÈë»ºŽæµ¥Ôª*/
void insertentryToHashMap(LRUCacheS *cache, cacheEntryS *entry)
{

	/*1.Ê¹ÓÃ¹þÏ£º¯Êý¶šÎ»ÊýŸÝŽæ·ÅÔÚÄÄžö²Û*/
	cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];
	//P(&cache->cache_lock);
	if (n != NULL) {
		/*2.Èç¹û²ÛÄÚÒÑÓÐÆäËûÊýŸÝÏî£¬ œ«²ÛÄÚµÄÊýŸÝÏîÓëµ±Ç°ÓûŒÓÈëÊýŸÝÏîÁŽ³ÉÁŽ±í£¬
		µ±Ç°ÓûŒÓÈëÊýŸÝÏîÎª±íÍ·*/
		entry->hashListNext = n;
		n->hashListPrev = entry;
	}
	/*3.œ«ÊýŸÝÏîŒÓÈëÊýŸÝ²ÛÄÚ*/
	cache->hashMap[hashKey(cache, entry->key)] = entry;
	//V(&cache->cache_lock);
}

/*ŽÓ¹þÏ£±íÉŸ³ý»ºŽæµ¥Ôª*/
void removeEntryFromHashMap(LRUCacheS *cache, cacheEntryS *entry)
{
	cacheEntryS *n = cache->hashMap[hashKey(cache, entry->key)];
	/*ÎÞÐè×öÈÎºÎÉŸ³ý²Ù×÷µÄÇé¿ö*/
	if (NULL == entry || NULL == cache || NULL == cache->hashMap) {
		return;
	}
	/*1.Ê¹ÓÃ¹þÏ£º¯Êý¶šÎ»ÊýŸÝÎ»ÓÚÄÄžö²Û*/
	
	/*2.±éÀú²ÛÄÚÁŽ±í£¬ ÕÒµœÓûÉŸ³ýµÄœÚµã£¬ œ«œÚµãŽÓ¹þÏ£±íÉŸ³ý*/
	while (n) {
		if (strcmp(n->key, entry->key) == 0) { /*ÕÒµœÓûÉŸ³ýœÚµã£¬ œ«œÚµãŽÓ¹þÏ£±íÕª³­*/
			if (n->hashListPrev) {
				//P(&cache->cache_lock);
				n->hashListPrev->hashListNext = n->hashListNext;
				//V(&cache->cache_lock);
			}
			else {
				//P(&cache->cache_lock);
				cache->hashMap[hashKey(cache, entry->key)] = n->hashListNext;
				//V(&cache->cache_lock);
			}
			if (n->hashListNext) {
				//P(&cache->cache_lock);
				n->hashListNext->hashListPrev = n->hashListPrev;
				//V(&cache->cache_lock);
			}
			return;
		}
		n = n->hashListNext;
	}
}

/*******************************************************************************
* »ºŽæŽæÈ¡œÓ¿Ú
********************************************************************************/
/* œ«ÊýŸÝ·ÅÈëLRU»ºŽæÖÐ*/
cacheEntryS* LRUCacheSet(void *lruCache, char *key, cacheEntryS **pEntryPointer)
{
	LRUCacheS *cache = (LRUCacheS *)lruCache;
	/*ŽÓ¹þÏ£±íÖÐ²éÕÒÊýŸÝÊÇ·ñÒÑŸ­ŽæÔÚ»ºŽæÖÐ*/
	cacheEntryS *entry = getValueFromHashMap(cache, key);
	if (entry != NULL) {
		//printk("entry != NULL\n");
		//entry->curr_bytes += data;
		//entry->sum_bytes += data;
		entry->packet_count++;
		//strncpy(entry->data, data, VALUE_SIZE);
		if (entry->flag == 0){
			//printk("entry->flag == 0\n");
			if ((*pEntryPointer) != NULL)
				if (strcmp(entry->key, (*pEntryPointer)->key) == 0){
					//printk("entry == (*pEntryPointer)\n");
					(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				}
			updateLRUList(cache, entry);
		}
		if (entry->flag == 1){
			//printk("entry->flag == 1\n");
			if ((*pEntryPointer)->packet_count < THRESHOLD){
				//printk("(*pEntryPointer)->sum_bytes < THRESHOLD\n");
				cacheEntryS *temp = NULL;
				temp = *pEntryPointer;
				//Queue[count] = *(*pEntryPointer);
				strcpy(Queue[count].key, (*pEntryPointer)->key);
				Queue[count].packet_count = (*pEntryPointer)->packet_count;
				//Queue[count].sum_bytes = (*pEntryPointer)->sum_bytes;
				//freeCacheEntry(*pEntryPointer);
				count++;
				(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				removeEntryFromHashMap(cache, (*pEntryPointer)->lruListNext);
				(*pEntryPointer)->lruListNext = (*pEntryPointer)->lruListNext->lruListNext;
				(*pEntryPointer)->lruListNext->lruListPrev = (*pEntryPointer);
				entry->flag = 0;
				cache->lruListSize--;
				freeCacheEntry(temp);
			}
			else{
				//printk("(*pEntryPointer)->sum_bytes >= THRESHOLD\n");
				(*pEntryPointer)->flag = 1;
				(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				entry->flag = 0;
			}
			updateLRUList(cache, entry);
		}
		//updateLRUList(cache, entry);
	}
	else {
		//printk("entry == NULL\n");
		/*ÊýŸÝÃ»ÔÚ»ºŽæÖÐ*/
		/*ÐÂœš»ºŽæµ¥Ôª*/
		entry = newCacheEntry(key);

		/*œ«ÐÂœš»ºŽæµ¥Ôª²åÈëÁŽ±í±íÍ·*/
		if ((*pEntryPointer) == NULL){
			//printk("(*pEntryPointer) == NULL\n");
			cacheEntryS *removedEntry = insertToListHead(cache, entry);
			if (NULL != removedEntry) {
				/*œ«ÐÂœš»ºŽæµ¥Ôª²åÈëÁŽ±í±íÍ·¹ý³ÌÖÐ£¬ ·¢Éú»ºŽæÂúÁËµÄÇé¿ö£¬
				ÐèÒªÌÔÌ­×îŸÃÃ»ÓÐ±»·ÃÎÊµœµÄ»ºŽæÊýŸÝµ¥Ôª*/
				removeEntryFromHashMap(cache, removedEntry);
				freeCacheEntry(removedEntry);
			}
			/*œ«ÐÂœš»ºŽæµ¥Ôª²åÈë¹þÏ£±í*/
			insertentryToHashMap(cache, entry);

			/*œ«µ¯³öµÄÊýŸÝ±£ŽæÔÚÁŽ±íÖÐ*/
			return removedEntry;
		}
		else{
			//printk("(*pEntryPointer) != NULL\n");
			//need test!!!!!!
			if ((*pEntryPointer)->packet_count < THRESHOLD){
				//printk("(*pEntryPointer)->sum_bytes < THRESHOLD\n");
				cacheEntryS *temp = NULL;
				cacheEntryS *removedEntry;
				temp = *pEntryPointer;
				//Queue[count] = *(*pEntryPointer);
				strcpy(Queue[count].key, (*pEntryPointer)->key);
				Queue[count].packet_count = (*pEntryPointer)->packet_count;
				//Queue[count].sum_bytes = (*pEntryPointer)->sum_bytes;
				count++;
				if ((*pEntryPointer)->lruListNext == NULL){
					//*(*pEntryPointer) = *((*pEntryPointer)->lruListPrev);
					(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
					removeEntryFromHashMap(cache, (*pEntryPointer)->lruListNext);
					(*pEntryPointer)->lruListNext = NULL;
					((LRUCacheS *)(void *)lruCache)->lruListTail = (*pEntryPointer);
				}
				else{
					(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
					removeEntryFromHashMap(cache, (*pEntryPointer)->lruListNext);
					(*pEntryPointer)->lruListNext = (*pEntryPointer)->lruListNext->lruListNext;
					(*pEntryPointer)->lruListNext->lruListPrev = (*pEntryPointer);
				}
				cache->lruListSize--;
				removedEntry = insertToListHead(cache, entry);
				insertentryToHashMap(cache, entry);
				freeCacheEntry(temp);
			}
			else{
				cacheEntryS *removedEntry;
				//printk("(*pEntryPointer)->sum_bytes >= THRESHOLD\n");
				(*pEntryPointer)->flag = 1;
				//printk("(*pEntryPointer) hash value is: %d\n", hashKey(lruCache, (*pEntryPointer)->key));
				(*pEntryPointer) = (*pEntryPointer)->lruListPrev;
				//printk("(*pEntryPointer)->lruListPrev hash value is: %d\n", hashKey(lruCache, (*pEntryPointer)->key));
				removedEntry = insertToListHead(cache, entry);
				//printk("Tail is :%s\n", removedEntry->key);
				if (NULL != removedEntry) {
					/*œ«ÐÂœš»ºŽæµ¥Ôª²åÈëÁŽ±í±íÍ·¹ý³ÌÖÐ£¬ ·¢Éú»ºŽæÂúÁËµÄÇé¿ö£¬
					ÐèÒªÌÔÌ­×îŸÃÃ»ÓÐ±»·ÃÎÊµœµÄ»ºŽæÊýŸÝµ¥Ôª*/
					removeEntryFromHashMap(cache, removedEntry);
					//freeCacheEntry(removedEntry);
				}
				/*œ«ÐÂœš»ºŽæµ¥Ôª²åÈë¹þÏ£±í*/
				insertentryToHashMap(cache, entry);

				/*œ«µ¯³öµÄÊýŸÝ±£ŽæÔÚÁŽ±íÖÐ*/
				return removedEntry;
			}
		}
	}
	return NULL;
}

/*ŽÓ»ºŽæÖÐ»ñÈ¡ÊýŸÝ*/
char *LRUCacheGet(void *lruCache, char *key)
{
	LRUCacheS *cache = (LRUCacheS *)lruCache;
	/*ŽÓ¹þÏ£±í²éÕÒÊýŸÝÊÇ·ñÒÑŸ­ÔÚ»ºŽæÖÐ*/
	cacheEntryS* entry = getValueFromHashMap(cache, key);
	if (NULL != entry) {
		/*»ºŽæÖÐŽæÔÚžÃÊýŸÝ£¬ žüÐÂÖÁÁŽ±í±íÍ·*/
		updateLRUList(cache, entry);
		/*·µ»ØÊýŸÝ*/
		return entry->key;
	}
	else {
		/*»ºŽæÖÐ²»ŽæÔÚÏà¹ØÊýŸÝ*/
		return NULL;
	}
}

/*ÅÐ¶Ïµ±Ç°µÄÊýŸÝ°üÊÇ·ñŽæÔÚÓÚLRU£¬ÈôŽæÔÚÊÇ·ñÐèÒªœøÈë¶àŒ¶¶ÓÁÐ*/


/**************************************************
* µ÷ÊÔœÓ¿Ú
***************************************************/
/*±éÀú»ºŽæÁÐ±í£¬ ŽòÓ¡»ºŽæÖÐµÄÊýŸÝ£¬ °Ž·ÃÎÊÊ±ŒäŽÓÐÂµœŸÉµÄË³ÐòÊä³ö*/


int LRUCacheLength(void *lruCache)
{
	LRUCacheS *cache = (LRUCacheS *)lruCache;
	return cache->lruListSize;
}
