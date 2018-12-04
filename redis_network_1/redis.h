/*************************************************************************
	> File Name: redis.h
	> Author: 
	> Mail: 
	> Created Time: 2018年12月04日 星期二 16时45分36秒
 ************************************************************************/

#ifndef _REDIS_H
#define _REDIS_H

/* Static server configuration */
#define REDIS_DEFAULT_HZ        10 
#define REDIS_SERVERPORT		6379 /* TCP port */
#define REDIS_TCP_BACKLOG       511     /* TCP listen backlog */
#define REDIS_BINDADDR_MAX		16
#define REDIS_IP_STR_LEN INET6_ADDRSTRLEN
#define REDIS_DEFAULT_DBNUM     16
#define REDIS_DEFAULT_TCP_KEEPALIVE 0

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
