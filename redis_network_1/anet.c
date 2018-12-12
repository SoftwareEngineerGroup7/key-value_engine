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
int anetTcpAccpt (char *err, int s, char *ip, size_t ip_len, in *port)
{
    
    int fd;
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);

    if ((fd == anetGenericAccpt (err, s, (sockaddr *)&sa, &salen)) == -1)
        return ANET_ERR;

    if (sa.ss_family == AF_INET) 
    {
        
        struct sockadd_in *s = (struct sockadd_in *)&sa;
        /* 将对方的信息记录下来 ip地址以及端口信息*/
        if (ip) 
            inet_ntop(AF_INET, (void *)&(s->addr), ip, ip_len);
        if (port)
            *port = ntohs (s->sin_port);
    }
    return fd;
}








