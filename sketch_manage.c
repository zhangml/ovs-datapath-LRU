#include "sketch_manage.h"
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/string.h>
#include <linux/types.h>
#include <net/sock.h>


#include "sketch_manage.h"
#include "lru_cache_impl.h"


/*****sketch update*****/


int init_LRU (int LRU_Capacity ) {
	if (0 != LRUCacheCreate(LRU_Capacity, &lruCache))
	{
		printk("error!!!\n");
		return 0;
	}
    printk("init_LRU success\n");
    return 0;
}

void clean_LRU() {
	
    LRUCacheDestory(&lruCache);
    printk("clean_LRU success\n");
}



void my_label_sketch( char* key) {
    
	cacheEntryS *haha = NULL;
	int LRUCurrLength,LRUCurrLength1;
	LRUCurrLength = LRUCacheLength(lruCache);

	if (LRUCurrLength == L)
		entryPointer = ((LRUCacheS *)(void *)lruCache)->lruListTail;
	else if(LRUCurrLength < L)
		entryPointer = NULL;
	haha = LRUCacheSet(lruCache, key, &entryPointer);

	LRUCurrLength1 = LRUCacheLength(lruCache);

	if (haha != NULL){

	strcpy(Queue[count].key, haha->key);

    Queue[count].packet_count = haha->packet_count;

    freeCacheEntry(haha);
	count++;
	}
	
    printk("LRU success\n");    
}







