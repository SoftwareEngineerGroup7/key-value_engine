/*************************************************************************
	> File Name: anet.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月12日 星期三 17时43分10秒
 ************************************************************************/
#include <stddef.h>
#include "anet.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <netdb.h>

/*
 * 打印错误信息而已 为啥看起来这么的邪乎
 */

static void anetSetError(char *err, const char *fmt, ...)
{
    va_list ap;
    if(!err) return;

    va_start(ap, fmt);
    vsnprintf(err, ANET_ERR_LEN, fmt, ap);
    va_end(ap);
}


/* 
 * anetListen 绑定并创建监听套接字
 */
static int anetListen(char *err, int fd, struct sockaddr *sa, socklen_t len, int backlog)
{
    
    if (bind(fd, sa, len) == -1) 
    {
        anetSetError(err, "bind: %s", strerror(errno));
        close(fd);
        return ANET_ERR;
    }

    if (listen (fd, backlog) == -1)
    {
        anetSetError(err, "listen: %s", strerror(errno));
        close(fd);
        return ANET_ERR;
    }
    
    return  ANET_OK;
}

/*
 *  用于获取一个连接  返回连接的描述符 对accept 进行了封装
 */
int anetGenericAccpt(char *err, int s, struct sockaddr *sa, socklen_t *len)
{
    
    int fd;
    while (1)
    {
        fd = accept (s, sa, len);
        if (fd == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                anetSetError (err, "accept: %s", strerror(errno));
                return ANET_ERR;
            }
        }
        break;
    }
    return fd;
}

/*
 * TCP 连接函数 最后返回连接的描述符 
 */
int anetTcpAccpt (char *err, int s, char *ip, size_t ip_len, int *port)
{
    
    int fd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);

    if ((fd == anetGenericAccpt (err, s, (struct sockaddr *)&sa, &salen)) == -1)
        return ANET_ERR;

    if (sa.ss_family == AF_INET) 
    {
        
        struct sockaddr_in *s = (struct sockaddr_in *)&sa;
        /* 将对方的信息记录下来 ip地址以及端口信息*/
        if (ip) 
            inet_ntop(AF_INET, (void *)&(s->sin_addr), ip, ip_len);
        if (port)
            *port = ntohs (s->sin_port);
   }
    return fd;
}


/*
 * 
 */
static int _anetTcpServer(char *err, int port, char *bindaddr, int af, int backlog) {
	int s, rv;
	char _port[6];
	struct addrinfo hints;
	struct addrinfo *servinfo, *p;

	snprintf(_port, 6, "%d", port); // 将端口的值转换为char类型
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = af;
	hints.ai_socktype = SOCK_STREAM; // 流式套接字
	//hints.ai_flags = AI_PASSIVE;
	/* 获取IP地址 及端口号 */
	if ((rv = getaddrinfo(bindaddr, _port, &hints, &servinfo)) != 0) {
		//anetSetError(err, "%s", gai_sererror(rv));
		return ANET_ERR;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue; // 创建套接字失败的话,继续
		if (anetSetReuseAddr(err, s) == ANET_ERR) goto error;
		if (anetListen(err, s, p->ai_addr, p->ai_addrlen, backlog) == ANET_ERR) goto error;

		goto end;
	}
	if (p == NULL) {
		anetSetError(err, "unable to bind socket");
		goto error;
	}
error:
	s = ANET_ERR;
end:
	/* 由于addrinfo 是隐式分配内存的 所以必须掉用下函数才能将空间释放掉*/
	freeaddrinfo(servinfo);
	return s;
}

int anetTcpServer(char *err, int port, char *bindaddr, int backlog) {
	return _anetTcpServer(err, port, bindaddr, AF_INET, backlog);
}

/*
 * 将fd 设置为非阻塞模式 （O_NONBLOCK）
 */
int anetNonBlock (char *err, int fd)
{
    
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        anetSetError(err, "fcntl(F_GETFL): %s", strerror(errno));
		return ANET_ERR;
    }
    if ((flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1) {
        anetSetError(err, "fcntl(F_GETFL): %s", strerror(errno));
		return ANET_ERR;
    }

    return ANET_OK;
}


/*
 * 打开或者关闭 Nagle 算法
 * 用来控制拥塞的 开启之后数据不是立即到达
 */

static int anetSetTcpNoDelay(char *err, int fd, int val) {
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1) {
		anetSetError(err, "setsockopt TCP_NODELAY: %s", strerror(errno));
		return ANET_ERR;
	}
	return ANET_OK;
}


/*
 * 禁用 Nagle 算法
 */
int anetEnableTcpNoDelay(char *err, int fd) {
	return anetSetTcpNoDelay(err, fd, 1);
}



