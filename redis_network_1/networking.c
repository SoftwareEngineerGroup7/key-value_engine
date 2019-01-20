/*************************************************************************
	> File Name: networking.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月05日 星期三 23时01分15秒
 ************************************************************************/
#include "redis.h"
#include <math.h>

void processInputBuffer(redisClient *c) {
	// 尽可能地处理查询缓存区中的内容.如果读取出现short read, 那么可能会有内容滞留在读取缓冲区里面
	// 这些滞留的内容也许不能完整构成一个符合协议的命令,需要等待下次读事件的就绪.
	while (sdslen(c->querybuf)) {

		if (!c->reqtype) {
			if (c->querybuf[0] == '*') {
				c->reqtype = REDIS_REQ_MULTIBULK; // 多条查询 
			}
			else {
				c->reqtype = REDIS_REQ_INLINE; // 内联查询 
			}
		}
		// 将缓冲区的内容转换成命令,以及命令参数
		if (c->reqtype == REDIS_REQ_INLINE) {
			if (processInlineBuffer(c) != REDIS_OK) break;
		}
		else if (c->reqtype == REDIS_REQ_MULTIBULK) {
			if (processMultibulkBuffer(c) != REDIS_OK) break;
		}
		else {
			// todo
		}

		if (c->argc == 0) {
			resetClient(c); // 重置客户端
		}
		else {
			if (processCommand(c) == REDIS_OK)
				resetClient(c);
		}
	}
}

/*
 * 读取客户端的查询缓冲区内容
 */
void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask) {
	redisClient *c = (redisClient *)privdata;
	int nread, readlen;
	size_t qblen;

	server.current_client = c; // 设置服务器的当前客户端

	readlen = REDIS_IOBUF_LEN; // 读入长度,默认为16MB

	// 获取查询缓冲区当前内容的长度 
	qblen = sdslen(c->querybuf); 

	/* 为查询缓冲区分配空间 */
	c->querybuf = sdsMakeRoomFor(c->querybuf, readlen);
	// 读入内容到查询缓存
	nread = read(fd, c->querybuf + qblen, readlen);

	if (nread == -1) {
		if (errno == EAGAIN) {
			nread = 0;
		} 
		else {
			//freeClient(c);
			return;
		}
	}
	else if (nread == 0) { // 对方关闭了连接
		// todo
		//freeClient(c);
		return;
	}

	if (nread) {
		sdsIncrLen(c->querybuf, nread);
	} 
	else {
		// 在 nread == -1 且 errno == EAGAIN 时运行
		server.current_client = NULL;
		return;
	}
	//
	// 从查询缓存中读取内容,创建参数,并执行命令,函数会执行到缓存中的所有内容都被处理完为止
	//
	processInputBuffer(c);
	server.current_client = NULL;
}


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

    /* max 为2  在源码中这里的定义是1000 这里就简化了一下*/
    while (max--) {
        /*anetEcpAccept 通过接受监听的本端套接字 获取请求连接的客户端的地址 并为之创建 套接字标示之*/
        cfd = anetTcpAccept(server.neterr, fd, cip, sizeof(cip), &cport);
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
