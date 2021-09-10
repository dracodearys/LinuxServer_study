#define TIMEOUT 5000

int timeout = TIMEOUT;
time_t start = time( NULL );
time_t end = time( NULL );
while( 1 )
{
    printf( "the timeout is now %d mill-seconds\n", timeout );
    start = time( NULL );
    int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, timeout );
    if( ( number < 0 ) && ( errno != EINTR ) )
    {
        printf( "epoll failure\n" );
        break;
    }
    /* ���epoll_wait�ɹ�����0����˵����ʱʱ�䵽��
    ��ʱ����Դ���ʱ���񣬲����ö�ʱʱ�� */
    if( number == 0 )
    {
        // timeout
        timeout = TIMEOUT;
        continue;
    }

    end = time( NULL );
    /* ���epoll_wait�ķ���ֵ����0���򱾴�epoll_wait���ó�����ʱ����(end - start)*1000 ms��
    ������Ҫ����ʱʱ��timeout��ȥ���ʱ�䣬�Ի���´�epoll_wait���õĳ�ʱ���� */
    timeout -= ( end - start ) * 1000;
    /* ���¼���֮���timeoutֵ���ܵ���0��˵������epoll_wait���÷���ʱ��
    �������ļ������������������䳬ʱʱ��Ҳ�պõ����ʱ����ҲҪ����ʱ���񣬲����ö�ʱʱ�� */
    if( timeout <= 0 )
    {
        // timeout
        timeout = TIMEOUT;
    }

    // handle connections
}
