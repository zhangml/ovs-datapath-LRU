#ifndef SKETCH_MANAGE_H
#define SKETCH_MANAGE_H


#include "sketch_util.h"

int init_LRU (int LRU_Capacity);
void clean_LRU(void);
void my_label_sketch( char* key);
#endif
