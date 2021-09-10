#ifndef TIME_WHEEL_TIMER
#define TIME_WHEEL_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUFFER_SIZE 64
class tw_timer;
/* ��socket�Ͷ�ʱ�� */
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[ BUFFER_SIZE ];
    tw_timer* timer;
};

/* ��ʱ���� */
class tw_timer
{
public:
    tw_timer( int rot, int ts ) 
    : next( NULL ), prev( NULL ), rotation( rot ), time_slot( ts ){}

public:
    int rotation;           // ��¼��ʱ����ʱ����ת����Ȧ����Ч
    int time_slot;          // ��¼��ʱ������ʱ�������ĸ��ۣ���Ӧ������
    void (*cb_func)( client_data* );    // ��ʱ���ص�����
    client_data* user_data;     // �ͻ�����
    tw_timer* next;             // ָ����һ����ʱ��
    tw_timer* prev;             // ָ��ǰһ����ʱ��
};

class time_wheel
{
public:
    time_wheel() : cur_slot( 0 )
    {
        for( int i = 0; i < N; ++i )
        {
            slots[i] = NULL;        //��ʼ��ÿ���۵�ͷ���
        }
    }
    ~time_wheel()
    {
        /* ����ÿ���ۣ����������еĶ�ʱ�� */
        for( int i = 0; i < N; ++i )
        {
            tw_timer* tmp = slots[i];
            while( tmp )
            {
                slots[i] = tmp->next;
                delete tmp;
                tmp = slots[i];
            }
        }
    }
    /* ���ݶ�ʱֵtimeout����һ����ʱ����������������ʵĲ��� */
    tw_timer* add_timer( int timeout )
    {
        if( timeout < 0 )
        {
            return NULL;
        }
        int ticks = 0;
        /* ������ݴ����붨ʱ���ĳ�ʱֵ����������ʱ����ת�����ٸ��δ�󱻴�����
        �����õδ����洢�ڱ���ticks�С���������붨ʱ���ĳ�ʱֵС��ʱ���ֵĲۼ��SI��
        ��ticks�����ۺ�Ϊ1������ͽ�ticks�����ۺ�Ϊtimeout/SI */
        if( timeout < TI )
        {
            ticks = 1;
        }
        else
        {
            ticks = timeout / TI;
        }
        /* ���������Ķ�ʱ����ʱ����ת������Ȧ�󱻴��� */
        int rotation = ticks / N;
        /* ���������Ķ�ʱ��Ӧ�ñ������ĸ����� */
        int ts = ( cur_slot + ( ticks % N ) ) % N;
        /* �����µĶ�ʱ��������ʱ����ת��rotationȦ֮�󱻴�������λ�ڵ�ts������ */
        tw_timer* timer = new tw_timer( rotation, ts );
        /* �����ts�����������κζ�ʱ��������½��Ķ�ʱ���������У�
        ������ʱ������Ϊ�ò۵�ͷ��� */
        if( !slots[ts] )
        {
            printf( "add timer, rotation is %d, ts is %d, cur_slot is %d\n", rotation, ts, cur_slot );
            slots[ts] = timer;
        }
        else
        /* ���򣬽���ʱ�������ts������ */
        {
            timer->next = slots[ts];
            slots[ts]->prev = timer;
            slots[ts] = timer;
        }
        return timer;
    }
    /* ɾ��Ŀ�궨ʱ��timer */
    void del_timer( tw_timer* timer )
    {
        if( !timer )
        {
            return;
        }
        int ts = timer->time_slot;
        /* slots[ts]��Ŀ�궨ʱ�����ڲ۵�ͷ��㡣���Ŀ�궨ʱ�����Ǹ�ͷ��㣬
        ����Ҫ���õ�ts���۵�ͷ��� */
        if( timer == slots[ts] )
        {
            slots[ts] = slots[ts]->next;
            if( slots[ts] )
            {
                slots[ts]->prev = NULL;
            }
            delete timer;
        }
        else
        {
            timer->prev->next = timer->next;
            if( timer->next )
            {
                timer->next->prev = timer->prev;
            }
            delete timer;
        }
    }
    /* SIʱ�䵽�󣬵��øú�����ʱ������ǰ����һ���۵ļ�� */
    void tick()
    {
        tw_timer* tmp = slots[cur_slot];        /* ȡ��ʱ�����ϵ�ǰ�۵�ͷ��� */
        printf( "current slot is %d\n", cur_slot );
        while( tmp )
        {
            printf( "tick the timer once\n" );
            /* �����ʱ����rotation����0����������һ�ֲ������� */
            if( tmp->rotation > 0 )
            {
                tmp->rotation--;
                tmp = tmp->next;
            }
            /* ����˵����ʱ���Ѿ����ڣ�����ִ�ж�ʱ����Ȼ��ɾ���ö�ʱ�� */
            else
            {
                tmp->cb_func( tmp->user_data );
                if( tmp == slots[cur_slot] )
                {
                    printf( "delete header in cur_slot\n" );
                    slots[cur_slot] = tmp->next;
                    delete tmp;
                    if( slots[cur_slot] )
                    {
                        slots[cur_slot]->prev = NULL;
                    }
                    tmp = slots[cur_slot];
                }
                else
                {
                    tmp->prev->next = tmp->next;
                    if( tmp->next )
                    {
                        tmp->next->prev = tmp->prev;
                    }
                    tw_timer* tmp2 = tmp->next;
                    delete tmp;
                    tmp = tmp2;
                }
            }
        }
        cur_slot = ++cur_slot % N;  /* ����ʱ���ֵĵ�ǰ�ۣ��Է�ӳʱ���ֵ�ת�� */
    }

private:
    static const int N = 60;        // ʱ�����ϲ۵���Ŀ
    static const int TI = 1;        // ÿ1sʱ����ת��һ�Σ����ۼ��Ϊ1s
    tw_timer* slots[N];             // ʱ���ֵĲۣ�����ÿ��Ԫ��ָ��һ����ʱ��������������
    int cur_slot;                   // ʱ���ֵĵ�ǰ��
};

#endif
