/*************************************************************************
	> File Name: redis.h
	> Author: 
	> Mail: 
	> Created Time: 2018年12月04日 星期二 16时45分36秒
 ************************************************************************/

#ifndef _REDIS_H
#define _REDIS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>
#include <netinet/in.h>
#include <signal.h>
#include "anet.h"
#include "ae.h"
#include "sds.h"
#include "zmalloc.h"

// 对象编码
#define REDIS_ENCODING_RAW 0     /* Raw representation */

/* 对象类型 */
#define REDIS_STRING 0 
#define REDIS_LIST 1
#define REDIS_SET 2
#define REDIS_ZSET 3
#define REDIS_HASH 4

/* Static server configuration */
#define REDIS_DEFAULT_HZ        10 
#define REDIS_SERVERPORT		6379 /* TCP port */
#define REDIS_TCP_BACKLOG       511     /* TCP listen backlog */
#define REDIS_BINDADDR_MAX		16
#define REDIS_IP_STR_LEN INET6_ADDRSTRLEN
#define REDIS_DEFAULT_DBNUM     16
#define REDIS_DEFAULT_TCP_KEEPALIVE 0

/* 对象编码 */
#define REDIS_ENCODING_RAW 0     /* Raw representation 原始代表*/
#define REDIS_ENCODING_INT 1     /* Encoded as integer */
#define REDIS_ENCODING_EMBSTR 8  /* Embedded sds string encoding */

/*
 * redisObject redis对象
 * 这个地方使用到了位域目的是为了节省内存
 */
typedef struct redisObject {
    unsigned type : 4; // 类型
    unsigned encoding : 4; // 编码
    unsigned lru : REDIS_LRU_BITS; //对象最后一次被访问的时间
    int refcount; // 引用计数
    void *ptr; //　指向实际值的指针　
} robj;


typedef struct redisDb {
 
} redisDb;

typedef struct redisClient {

    /**/
    int fd;

    /*正在使用的数据库*/
    redisDb * db;
    
    /*正在使用的数据库的id*/
    int dictid;
    
    /*客户端的名字*/
    robj *name;
    
    /*查询缓冲区*/
    sds querybuf;
    /*查询缓冲区长度峰值*/
    size_t querybuf_peak;
    
    /* 参数数量 */
    int argc;


    /*参数对象数组*/
    robj **argv;
    
    /* 记录被客户端执行的命令 */
    struct redisCommand *cmd, *lastcmd;
    
    /* 请求的类型 是内联命令还是多条命令 */
    int reqtype;
    
    /* 剩余未读取的命令内容数 */
    int multibulklen;
    
    /* 命令长度 */
    long bulklen;
    
    unsigned long reply_bytes; // 回复链表中对象的总大小

	int bufpos; // 回复偏移量

	char buf[REDIS_REPLY_CHUNK_BYTES];
} redisClient;




struct redisServer {
    
    /* 配置文件的绝对路径 */
    char *configfile;   
    
    /* serverCron()每秒钟调用的次数 */
    int hz;
    
    /* 一个数组，保存着服务器的所有数据库 */
    redisDb *db; 
    
    /* 命令表 */
    dict *commands;
    
    /**/
    dict *orig_command;
    
    /* 事件状态 */
    aeEventLoop *el;
    
    /* 关闭服务器的标识 */
    int shutdown_asap;

    /* TCP 监听port */
    int port;

    /* TCP listen() backlog */
    int tcp_backlog;

    /* ip地址 */
    char *bindaddr[REDIS_BINDADDR_MAX];
    /* 地址的数量 */
    int bindaddr_conut;

    /* TCP描述符 */
    int ipfd[REDIS_BINDADDR_MAX];

    /* 已经是用了的描述符的个数 */
    int ipfd_count;

    /* 一个链表,保存了所有的客户端状态结构 */
    list *clients;	

    /* 链表,保存了所有待关闭的客户端 */
	list *clients_to_close; 

    /* 服务器当前服务的客户端,仅用于崩溃报告 */
	redisClient *current_client; 

    /* 用于记录网络错误 */
	char neterr[ANET_ERR_LEN]; 

    /* 是否开启 SO_KEEPALIVE选项 */
	int tcpkeepalive;	
	int dbnum;			// 数据库的总数目

	/* Limits */
	int maxclients;      // Max number of simultaneous clients

};




#endif
