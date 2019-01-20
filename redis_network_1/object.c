/*************************************************************************
	> File Name: object.c
	> Author: 
	> Mail: 
	> Created Time: 2019年01月20日 星期日 18时19分12秒
 ************************************************************************/

#include "object.h"
#include <unistd.h>
#include <math.h>
#include <ctype.h>


/*
 * 创建一个REDIS_ENCODING_EMBSTR 编码的字符对象
 * 这个字符对象中的sds会和字符串对象的　redisObject结构一起分配
 * 因此这个字符也是不可修改的
 */
robj *createEmbeddedStringObject(char *ptr, size_t len) {
    robj *o = zmalloc (sizeof(robj) + sizeof(sdshdr) + len + 1);
    /**********************@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@22222a*/
    //在接下来会换成我自己的字符串 palorstring 
    struct sdshdr *sh = (void*)(o + 1);

    o->type = REDIS_STRING; 
    o->encoding = REDIS_ENCODING_EMBSTR;
    o->ptr = sh + 1;
    o->refcount = 1;

    sh->len = len;
    sh->free = 0;
    if (ptr) {
        memcpy(sh->buf, ptr, len);
        sh->buf[len] = '\0';
    }
    else {
        memset(sh->buf, 0, len + 1);1
    }
}

/*
 * 创建一个REDIS_ENCODING_RAW 编码的字符对象
 * 对象的指针指向一个sds结构
 */
robj *createRawStringObject(char *ptr, size_t len) {
    return createObject(REDIS_STRING, sdsnewlen(ptr, len));
}

/*
 * 关于为啥是39这个还蛮有意思的 具体原因如下
 * 　因为缓存行的大小是64个字节
 * 　在cache 缓存中可以很高效的被访问到
 */
#define REDIS_ENCODING_EMBSTR_SIZE_LIMIT 39
robj *createStringObject(char *ptr, size_t len) {
    if (len <= REDIS_ENCODING_EMBSTR_SIZE_LIMIT)
        return createEmbeddedStringObject(ptr, len);
    else
        return createRawStringObject(ptr, len);
}

/*
 * 创建一个新的robj对象
 */
robj *createObject(inttype, void *ptr) {

    robj *o = zmalloc(sizeof(*o));

    o->type = type;
    o->encoding  = REDIS_ENCODING_RAW;
    o->ptr = ptr;
    o->refcount = 1;

    return 0;
}

/*
 *释放字符串对象
 */
void freeStringObject(robj *o) {
	if (o->encoding == REDIS_ENCODING_RAW) {
		sdsfree(o->ptr);
	}
}

/*
 * 为对象的引用计数减一
 * 当对象的引用计数降为 0 时，释放对象。
 */
void decrRefCount(robj *o) {

	//if (o->refcount <= 0) redisPanic("decrRefCount against refcount <= 0");

	// 释放对象
	if (o->refcount == 1) {
		switch (o->type) {
            /* 由于我实现了string相关的 */
		    case REDIS_STRING: freeStringObject(o); break;
		    //case REDIS_LIST: freeListObject(o); break;
		    //case REDIS_SET: freeSetObject(o); break;
		    //case REDIS_ZSET: freeZsetObject(o); break;
		    //case REDIS_HASH: freeHashObject(o); break;
		    default:
		    	//redisPanic("Unknown object type"); 
		    	break;
		}
		zfree(o);
	}
	// 减少计数
	else {
		o->refcount--;
	}
}


/*释放资源函数　调用上边的*/
void decrRefCountVoid(void *o) {
    decrRefCount(o);
}


/*
 * 为对象的引用计数增一
 */
void incrRefCount(robj *o) {
	o->refcount++;
}

/*
 * 回复内容复制函数　那就是将对象的引用计数加１
 */
void *dupClientReplyValue(void *o) {
	incrRefCount((robj *o);
	
    return o;
}
 
