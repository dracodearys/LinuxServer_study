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
    struct event_base* base = event_init();/* һ��event_base�൱��һ��Reactor */

    struct event* signal_event = evsignal_new( base, SIGINT, signal_cb, base ); // �����ź��¼�������
    event_add( signal_event, NULL );

    timeval tv = { 1, 0 };
    struct event* timeout_event = evtimer_new( base, timeout_cb, NULL );    // ������ʱ�¼��������������Ǻ궨�壬��ں�����ʵ��event_new()
    event_add( timeout_event, &tv );    // ���¼���������ӵ�ע���¼������У����ҽ����¼���������Ӧ���¼���ӵ�ʱ���·�ַ�����

    event_base_dispatch( base );        // ִ���¼�ѭ��

    event_free( timeout_event );    // �ͷ�ϵͳ��Դ
    event_free( signal_event );
    event_base_free( base );
}  
