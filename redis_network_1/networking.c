/*************************************************************************
	> File Name: networking.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月05日 星期三 23时01分15秒
 ************************************************************************/
#include "redis.h"
#include <math.h>

/*
 * 创建一个TCP连接处理器
 */
void acceptTcpHandler(aeEventLoop *el, int fd, void *private, int mask) {
    
    int cport, cfd, max = 2;
    char cip[REDIS_IP_LEN];

    while (max) {
        cfd = anetEcpAccept(server.neterr, fd, cip, sizeof(cip), &cport);
    }

}
