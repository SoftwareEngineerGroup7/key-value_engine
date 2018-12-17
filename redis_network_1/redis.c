/*************************************************************************
	> File Name: redis.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月04日 星期二 16时38分59秒
 ************************************************************************/

#include "redis.h"

#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <locale.h>

/*=============================== Function declariton =======================*/
void setCommand(redisClient *c);
void getCommand(redisClient *c);
/*================================= Globals ================================= */

/* Global vars */
struct redisServer server; /* server global state */


/*
 * 设置信号处理函数
 */
void setupSignalHandlers(void) {
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = sigtermHandler;
	sigaction(SIGTERM, &act, NULL);

}

/*
 * 在port端口监听
 */
int listenToPort(int port, int *fds, int *count) {
	int j;

	if (server.bindaddr_count == 0) server.bindaddr[0] = NULL;

	for (j = 0; j < server.bindaddr_count || j == 0; j++) {
		if (server.bindaddr[j] == NULL) {
			// 这里做了一些简化工作,那就是仅支持ipv4即可
			fds[*count] = anetTcpServer(server.neterr, port, NULL, server.tcp_backlog);
			if (fds[*count] != ANET_ERR) {
				anetNonBlock(NULL, fds[*count]); // 设置为非阻塞
				(*count)++;
			}
			if (*count) break;
		}
		else {
			// Bind IPv4 address.
			fds[*count] = anetTcpServer(server.neterr, port, server.bindaddr[j],
				server.tcp_backlog);
		}

		if (fds[*count] == ANET_ERR) {
			return REDIS_ERR;
		}
		anetNonBlock(NULL, fds[*count]);
		(*count)++;
	}
	return REDIS_OK;
}

/*
 * 初始化服务器的配置信息
 */
void initServerConfig()
{
    server.hz = REDIS_DEFAULT_HZ;
    server.port = REDIS_SERVERPORT;
    server.tcp_backlog = REDIS_TCP_BACKLOG;

    server.maxclients = REDIS_MAX_CLIENTS;
    server.ipfd_count = 0;
    server.bindaddr_count = 0;

    server.maxclients = REDIS_MAX_CLIENTS;
    server.ipfd_count = 0;
    server.dbnum = REDIS_DEFAULT_DBNUM;
    server.tcp_keepalive = REDIS_DEFAULT_TCP_KEEPALIVE;
    //待续
    //server.commands 
}

void initServer()
{
    int j;
    /* 忽略信号 */
    signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

    setupSignalHandlers();

    /* 初始化并创建数据结构*/
	server.clients = listCreate();
	server.clients_to_close = listCreate();

    /* 创建一个事件处理器*/
    server.el = aeCreateEventLoop(server.maxclients + REDIS_EVENTLOOP_FDSET_INCR);

    /* 打开 TCP 监听端口,用于等待客户端的命令请求 */
	if (server.port != 0 &&
		listenToPort(server.port, server.ipfd, &server.ipfd_count) == REDIS_ERR) {
		exit(1);
	}

    /* 为serverCron 创建定时事件 */
    if (aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL) == AE_ERR) {
        exit (1);
    }

    /* 
     * 为 TCP 连接关联应答(accept) 处理器， 用于接收并应答客户端的connect请求
     * networking.c/acceptTcpHandler 函数是Redis的连接应答处理器，用于对连接
     * 服务器监听套截字的客户端进行应答，就是对 socket.h/accept函数的包装，将
     * 此应答器与AE_READABLE事件关联起来，当有客户端进行connect连接时，套接字
     * 就会产生AE_READABLE时间，引发连接应答器进行处理，执行应答操作。
     */
    for (j = 0; j < server.ipfd_count; j++) {
        if (aeCreateFileEvent (server.el, server.ipfd[j], AE_READABLE,
                               acceptTcpHandler, NULL) == AE_ERR) {
                                    printf ("Con't accept!\n");
                                    exit (-1);
                              }
    }

}

int main (int argc, char **argv)
{
    initServerConfig();
    initServer();
    /* 事件处理器 */
    aeMain(server.el);
    return 0;
}
