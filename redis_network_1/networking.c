/*************************************************************************
	> File Name: networking.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月05日 星期三 23时01分15秒
 ************************************************************************/
#include "redis.h"
#include <math.h>




/*
 * 创建一个新的客户端
 *
 */
redisClient *createClient(int fd) {
    
    redisClient *c = zmalloc (sizeof(redisClient));
    
    if (fd != -1) {
        
        /*将 fd 设置为非阻塞模式 (O_NONBLOCK) */
        anetNonBlock(NULL, fd);

        /* 禁用 Nagle 算法 */
        anetEnableTcpNoDelay (NULL, fd);
    
        if (server.tcpkeepalive) {
            /* 修改 TCP 连接的 keepalive选项 */
            anetKeepAlive (NULL, fd, server.tcpkeepalive);
        }
        
        /* 绑定读事件到事件Loop(开始接收命令请求) */
        if (aeCreateFileEvent(server.el, fd, AE_READABLE, readQueryFromClient, c) == AE_ERR) {

            close (fd);
            zfree (c);
            return NULL;
        }
    
    }



	/* 初始化各个属性 */
	c->fd = fd;
	c->name = NULL;
	c->bufpos = 0; // 回复缓冲区的偏移量
	c->querybuf = sdsempty();
	c->reqtype = 0; // 命令请求的类型
	c->argc = 0; // 命令参数的数量
	c->argv = NULL; // 命令参数
	c->cmd = c->lastcmd = NULL; // 当前执行的命令和最近一次执行的命令

	c->bulklen = -1; // 读入的参数的长度 
	c->multibulklen = 0; // 查询缓冲区中未读入的命令内容数量

	c->reply = listCreate(); // 回复链表 
	c->reply_bytes = 0; //  回复链表的字节量

	listSetFreeMethod(c->reply, decrRefCountVoid);
	listSetDupMethod(c->reply, dupClientReplyValue);

    /* 如果不是伪客户端,那么添加服务器的客户端到客户端链表之中 */
	if (fd != -1) listAddNodeTail(server.clients, c);

	return c;

}











/*
 * TCP 连接 accept 处理器
 */
#define MAX_ACCEPT_PRE_CALL 1000
static void acceptCommandHandler(int fd, int flags) {
    /* 创建客户端 */
    redisClient *c;
    
    /*
     * 构造一个redisClient结构　××完成fd与RedisClient结构的关联并初始化
     * 然后加入server的clients链表上　　对fd的读事件进行监听
     */
    if ((c = createClient(fd)) == NULL) {
        /*加入log*/
        close(fd);
        return;
    }
}



/*
 * 创建一个TCP连接处理器
 */
void acceptTcpHandler(aeEventLoop *el, int fd, void *private, int mask) {
    
    int cport, cfd, max = 2;
    char cip[REDIS_IP_LEN];

    /* max 为啥是2*/
    while (max--) {
        cfd = anetEcpAccept(server.neterr, fd, cip, sizeof(cip), &cport);
        if (cfd == ANET_ERR) {
            if (errno != EWOULDBLOCK) {
                /*log*/
                return;
            }
        }
        
        /* 为客户端创建客户端状态 */
        acceptCommandHandler (cfd, 0);
    
    }

}
