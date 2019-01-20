/*************************************************************************
	> File Name: object.h
	> Author: 
	> Mail: 
	> Created Time: 2019年01月20日 星期日 18时21分35秒
 ************************************************************************/

#ifndef _OBJECT_H
#define _OBJECT_H

#include "redis.h"
#include "zmalloc.h"

/*api*/
robj *createObject(int type, void *ptr);
robj *createEmbeddedStringObject(char *ptr, size_t len);
robj *createRawStringObject(char *ptr, size_t len);
robj *createStringObject(char *ptr, size_t len);
robj *createObject(int type, void *ptr);
void freeStringObject(robj *o);
void decrRefCount(robj *o);
void decrRefCountVoid(void *o);
void incrRefCount(robj *o);
robj *createEmbeddedStringObject(char *ptr, size_t len);


#endif
