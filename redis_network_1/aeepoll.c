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

    aeApiState *statex = zmalloc(sizeof(aeApiCreate);
    if(!state) return -1;

    /*初始化事件集 */
    state->events = zmalloc(sizeof(struct epoll_event) *eventLoop->setsize);
    if (!state->event)
    {
        zfree(state);
         return -1;
    }
    /*创建一个epoll实例 一般来说　一个应用之中　只需要有一个epoll 实例就足够了*/
    state->epfd = epoll_create(1024);
    if (state->epfd == -1) {
        zfree(state->events);
        zfree(state);
        return -1;
    }

    /*赋值给eventloop apidata 被设置成了私有数据*/
    eventLoop->apidata = state;
    return 0;

}


/*
 * 关联给定事件到fd
 */
int aeApiAddEvent (aeEventLoop *eventLoop, int fd, int mask) {

    aeApiState *state = eventLoop->apidata;
    struct epoll_event ee;

    /*如果fd已经没有任何关联 那么这是一个ADD操作 如果已经关联那么这是一个MOD操作*/
    int op = eventLoop->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    /* 注册事件到epoll */
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