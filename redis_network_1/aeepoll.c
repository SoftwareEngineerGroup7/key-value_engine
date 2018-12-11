/*************************************************************************
	> File Name: aeepoll.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月07日 星期五 12时17分50秒
 ************************************************************************/
#include "aeepoll.h"

/*
 * 创建一个epoll 实例 并将结果返回 eventLoop
 */
int aeApiCreate(aeEventLoop *eventLoop) {

    aeApiState *state = zmalloc(sizeof(aeApiState));

    if (!state) return -1;

    /* 初始化事件槽空间 */
    state->events = zmalloc(sizeof(struct epoll_event)*eventLoop->setsize);
    if (!state->events) {
        zfree(state);
        return -1;
    }

    /* 创建 epoll 实例,一般来说,一个应用之中,只需要一个 epoll 实例便足够了 */
    state->epfd = epoll_create(1024); // 1024 is just a hint for the kernel
    if (state->epfd == -1) {
        zfree(state->events);
        zfree(state);
        return -1;
    }

    /* 赋值给 eventLoop
	 * apidata这个玩意被设置为了私有数据*/
    eventLoop->apidata = state;
    return 0;
}

/*
 * 调整事件槽大小
 */
int aeApiResize(aeEventLoop *eventLoop, int setsize) {

    aeApiState *state = eventLoop->apidata;
    /*重新分配事件槽大小*/
    state->events = zrealloc(state->events, sizeof(struct epoll_event)*setsize);
    return 0;
}

/*
 * 释放epoll实例和事件槽
 */
void aeAPiFree(aeEventLoop *eventLoop)
{
    aeApiState *state = eventLoop->apidata;
    close(state->epfd);
    zfree(state->events);
    zfree(state);
}

/*
 * 关联给定事件到fd
 */
int aeApiAddEvent (aeEventLoop *eventLoop, int fd, int mask) {

    aeApiState *state = eventLoop->apidata;
    struct epoll_event ee;

    /*如果fd已经没有任何关联 那么这是一个ADD操作 如果已经关联那么这是一个MOD操作*/
    int op = eventLoop->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    /* 注册事件到epoll  ee.data 是一个uint32_t  */
    ee.data = 0;
    mask |= eventLoop->events[fd].mask;
    /* 可读可写事件 */
    if (mask & AE_READABLE) ee.events |= EPOLLIN:
    if (mask & AE_READABLE) ee.events |= EPOLLOUT:
    // avoid valgrind warning 这个之后研究一下
    ee.data.u64 = 0;
    ee.data.fd = fd;

    /* -1 失败 */
    if (epoll_ctl (state->epfd, op, fd, &ee) == -1) return -1;

    return 0;
}

/*
 * 从 fd 中删除给定事件
 */
void  aeApiDelEvent (aeEventLoop *eventLoop, int fd, int delmask) {
    aeApiState *state = eventLoop->apidata;
    struct epoll_event ee;
    
    /*在events[fd].mask 中除去delmask这个属性*/
    int mask = eventLoop->events[fd].mask & (~delmask);
    
    ee.events = 0;
    if (mask & AE_READABLE) ee.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.u64 = 0;
    ee.data.fd = fd;

    if (mask != AE_NONE) {
        epoll_ctl(state->epfd, EPOLL_CTL_MOD, fd, &ee);
    } else {
        /* kernel < 2,6.9 规定event 不能为null */
        epoll_ctl(state->epfd, EPOLL_CTL_DEL, fd, &ee);
    }

}

/*
 * 获取可执行事件
 */
int aeApiPoll(aeEventLoop *eventLoop, struct timeval *tvp) {
    aeApiState *state = eventLoop->apidata;
    int retval, numevents = 0;

    /* 等待时间  tvp 不为空则等待一段时间 */
    /* 函数调用完毕之后 state->events 中存放了就绪事件的信息 */
    retval = epooll_wait (state->epfd, state->events, eventLoop->setsize,
                          tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : -1);

    if (retval > 0) { 

        /* 为已就绪事件设置相应模式 */
        numevents = retval;
        for (j = 0; j < numevents; j++) {
            int mask = 0;
            struct epoll_event *e = state->event + j;
            
            /* &,&& 与 |,|| 作为逻辑运算符时 单与双的计算结果是相同的  
             * &, | 做位运算时， &是求交集 |是并起来
             * 
             *  例如如下首例之中，可以看到 e->events与EPOLLIN 是否有交集
             *  mask = mask | AE_READABLE, 这是给mask 赋予AE_READABLE 的属性 */
            
            if (e->events & EPOLLIN) mask |= AE_READABLE;   //可读
            if (e->events & EPOLLOUT) mask |= AE_WRITABLE;  //可写    
            if (e->events & EPOLLERR) mask |= AE_WRITABLE;  //出错
            if (e->events & EPOLLHUP) mask |= AE_WRITABLE;

            /* 将事件集的各项属性加入已就绪队列之中*/
            eventLoop->fired[j].fd = e->data.fd;
            /* 可以看到这个是传了这个套接字的所有事件集合 那么应该是处理完所有事件材返回epoll_wait*/
            eventLoop->fired[j].mask = mask;
        }
    }

    /* 返回已就绪事件的个数*/
    return numevents;
 }




