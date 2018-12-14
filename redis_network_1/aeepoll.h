/*************************************************************************
	> File Name: aeepoll.h
	> Author: 
	> Mail: 
	> Created Time: 2018年12月07日 星期五 12时20分27秒
 ************************************************************************/

#ifndef _AEEPOLL_H
#define _AEEPOLL_H

#include "ae.h"
#include <sys/epoll.h>

/*
 * aeApiState 的状态
 */
typedef struct aeApiState {
    
    /* epoll_event 的实例描述符 */
    int epfd;
    /* 事件集 */
    struct epoll_event *events;    
} aeApiState;

int aeApiPoll(aeEventLoop *eventLoop, struct timeval *tvp);
int aeApiCreate(aeEventLoop *eventLoop);
int aeApiResize(aeEventLoop *eventLoop, int setsize);
void aeApiFree(aeEventLoop *eventLoop);
int aeApiAddEvent(aeEventLoop *eventLoop, int fd, int mask);
void aeApiDelEvent(aeEventLoop *eventLoop, int fd, int delmask);
char *aeApiName(void);

#endif
