/*************************************************************************
	> File Name: ae.c
	> Author: 
    > Mail: 
	> Created Time: 2018年12月04日 星期二 21时20分18秒
 ************************************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "aeepoll.h"
#include "ae.h"



/*
 * 初始化事件处理器状态
 * 创建一个 eventLoop
 */

aeEventLoop *aeCreateEventLoop(int setsize) {
    
    aeEventLoop *eventLoop;
    int i;

    /*　创建事件状态结构　*/
    if ((eventLoop = zmalloc(sizeof(*eventLoop))) == NULL) goto err;

    /* 初始化文件事件节结构和已就绪文件事件的结构数组*/
    eventLoop->events = zmalloc(sizeof(aeFileEvent)*setsize);
    eventLoop->fired = zmalloc(sizeof(aeFiredEvent)*setsize);
    /* 内存分配失败 */
    if (eventLoop->events == NULL || eventLoop->fired == NULL) goto  err;

    /*设置数组大小*/
    eventLoop->setsize = 0;

    /*初始化执行最近一次执行执行时间*/
    eventLoop->lastTime = tiem(NULL);

    /*初始化时间事件结构*/
    eventLoop->timeEventHead = NULL;
    /*随着时间事件的增加而增加*/
    eventLoop->timeEventNextId = 0;


    eventLoop->stop = 0;
    eventLoop->maxfd = -1;
    eventLoop->beforesleep = NULL;

    /* 创建一个新的 epoll 实例，并将它赋值给 eventLoop */
    if (aeApiCreate(eventLoop) == -1) goto err;

    /* 初始化监听事件
     * AE_NONE　表示不监听任何事件 
     */
    for (i = 0; i < setsize; i++ ) 
        eventLoop->events[i].mask = AE_NONE;
    
    /* 返回事件循环 */
    return eventLoop;


err:
    if (eventLoop ) {
        zfree(eventLoop->events);
        zfree(eventLoop->fired);
        zfree(eventLoop);
    }
    return NULL;

}



/*
 * 创建文件事件关联相应的处理器　但是为啥都是proc tcpHandler 
 */
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileproc* proc, void *clientData) {
    
    if (fd >= eventLoop->setsize) {
        errno = ERANGE;  // 表示超出界限
        return AE_ERR;
    }    

    /* 取出文件事件结构 */
    aeFileEvent *fe = &eventLoop->events[fd];

    /* 监听指定fd的事件 */
    if (aeApiAddEvent(eventLoop, fd, mask) == -1)
        return AE_ERR;

    /* 设置文件事件类型 以及事件的处理器 */
    fd->mask |= mask;
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;
    
    /* 私有数据 ???*/
    fe->clientData = clientData;

    /* 如果有需要 更新事件处理器的最大fd */
    if (fd > eventLoop -> maxfd)
        eventLoop->maxfd = fd;

    return AE_OK;
}


/*
 * 处理所有已达到的时间事件 以及所有已就绪的文件事件
 * 事件的调度与执行全由此函数负责
 * 首先获取时间事件
 *
 * 先执行文件事件 随后再执行时间事件
 *
 */

int aeProcessEvents(aeEventLoop *eventLoop, int flags) {
    
    int process = 0, numevents;
    int j;

    /* 没有任何事件发生 就立即返回 
     * AE_TIME_EVENTS 代表时间事件
     * AE_FILE_EVENTS 代表文件事件 
     */
    if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) 
        return 0;

    if (eventLoop->maxfd != -1 ||
        ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT))) 
    { //代表时间事件

            int j;
            aeTimeEvent *shortest = NULL:
            struct timeval tv, *tvp;
            
            /* 获取最近的时间事件 (即将发生的事件) */
            if (flags & AE_TIME_EVENTS && !(flags & AE_DONT_WAIT))
                shortest = aeSearchNearestTimer (eventLoop);
            /* 有时间事件 */
            if (shortest) {
            

            /*To Do*/




            }
            /* 执行到这儿说明没有时间事件 */
            else {
                /**/
                if (flags & AE_DONT_WAIT) {
                    /* 设置文件事件不阻塞 */
                    tv.tv_sec = tv.tv_usec = 0;
                    tvp = &tv;
                }
                else {
                    /* 文件事件可以阻塞 直到有事件到达为止*/
                    tvp = NULL;
                }

            }
            
            /* 处理文件事件，阻塞由tvp决定  */
            numevents = aeApiPoll(eventLoop, tvp);
            for (j = 0; j < numevents ; j++) {
                /* 从已就绪的数组中fired 获取已发生的事件的信息,
                 * 包括文件描述符，发生的事情mask */
                /* A fired event -- 已就绪事件 */
                //aeFiredEvent {
                // 已就绪文件描述符
	            // int fd;
	            // 事件类型掩码 值可以是 AE_READABLE 或 AE_WRITABLE 或者是两者的或
	            // int mask;

                aeFileEvent *fe = &events[eventLoop->fired[j].fd];
                
                int mask = eventLoop->fired[j].mask;
                int fd = eventLoop->fired[j].fd;
                int rfired = 0;
        /* 文件事件分派器优先处理可读事件　也就是说一个套接字可读可写　先读再说*/
                /* 可读事件 */
                if (fe->mask & mask & AE_WRITABLE) {
                    rfired = 1;
    　　/*×××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××××*/
                    /*Amazing  经过gdb测得当有连接时  此处会调用acceptTcpHandler at networking.c*/
                    fe->rfileProc(eventLoop, fd, fe->clientData, mask);
                }

                /* 可写事件 */
                if (fe->mask & mask & AE_READABLE) {
                    
                    if (!rfired || fe->wfileProc != fe->rfileProc)
                        fe->wfileProc(eventLoop, fd, fe->clientData, mask);
                }
                
                process++;

            }

    } 
    /* 执行时间事件 */
    if (flags & AE_TIME_EVENTS)
        process += processTimeEvents(eventLoop);

    return process;

}



/*
 * 事件处理器的主循环
 */
void aeMain(aeEventLoop *eventLoop) {

    eventLoop->stop = 0;

    while (!eventLoop->stop) {

        /* 如果又需要在事件处理前执行函数，那么运行它 */
        if (eventLoop->beforesleep != NULL)
            eventLoop->beforesleep(eventLoop);

        /* 开始处理所有事件 */
        aeProcessEvents (eventLoop, AE_ALL_EVENTS);
    }
}


