#include <sys/signal.h>
#include <sys/time.h>
#include <event.h>

void signal_cb( int fd, short event, void* argc )
{
    struct event_base* base = ( event_base* )argc;
    struct timeval delay = { 2, 0 };
    printf( "Caught an interrupt signal; exiting cleanly in two seconds...\n" );
    event_base_loopexit( base, &delay );
}  

void timeout_cb( int fd, short event, void* argc )
{
    printf( "timeout\n" );
}

int main()  
{  
    struct event_base* base = event_init();/* 一个event_base相当于一个Reactor */

    struct event* signal_event = evsignal_new( base, SIGINT, signal_cb, base ); // 创建信号事件处理器
    event_add( signal_event, NULL );

    timeval tv = { 1, 0 };
    struct event* timeout_event = evtimer_new( base, timeout_cb, NULL );    // 创建定时事件处理器，本身是宏定义，入口函数其实是event_new()
    event_add( timeout_event, &tv );    // 将事件处理器添加到注册事件队列中，并且将该事件处理器对应的事件添加到时间多路分发器中

    event_base_dispatch( base );        // 执行事件循环

    event_free( timeout_event );    // 释放系统资源
    event_free( signal_event );
    event_base_free( base );
}  
