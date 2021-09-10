#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUFFER_SIZE 64
class util_timer;       /* ǰ������ */

/* �û����ݽṹ���ͻ���socket��ַ��socket�ļ���������������Ͷ�ʱ�� */
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[ BUFFER_SIZE ];
    util_timer* timer;
};

/* ��ʱ���� */
class util_timer
{
public:
    util_timer() : prev( NULL ), next( NULL ){}

public:
   time_t expire;       /* ����ĳ�ʱ�¼�������ʹ�þ���ʱ�� */
   void (*cb_func)( client_data* );     /* ����ص����� */
   /* �ص���������Ŀͻ����ݣ��ɶ�ʱ����ִ���ߴ��ݸ��ص����� */
   client_data* user_data;
   util_timer* prev;        /* ָ��ǰһ����ʱ�� */
   util_timer* next;        /* ָ����һ����ʱ�� */
};

/* ��ʱ����������һ������˫���б��Ҵ���ͷ����β�ڵ� */
class sort_timer_lst
{
public:
    sort_timer_lst() : head( NULL ), tail( NULL ) {}
    /* ��������ʱ��ɾ���������еĶ�ʱ�� */
    ~sort_timer_lst()
    {
        util_timer* tmp = head;
        while( tmp )
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }
    /* ��Ŀ�궨ʱ��timer��ӵ������� */
    void add_timer( util_timer* timer )
    {
        if( !timer )
        {
            return;
        }
        if( !head )
        {
            head = tail = timer;
            return; 
        }
        /* ���Ŀ�궨ʱ���ĳ�ʱʱ��С�ڵ�ǰ���������ж�ʱ���ĳ�ʱʱ�䣬
        ��Ѹö�ʱ����������ͷ������Ϊ�����µ�ͷ��㡣�������Ҫ����
        ���غ���add_timer(util_timer* timer, uti_timer* lst_head)��
        �������������к��ʵ�λ�ã��Ա�֤������������� */
        if( timer->expire < head->expire )
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer( timer, head );
    }

    /* ��ĳ����ʱ�������仯ʱ��������Ӧ�Ķ�ʱ���������е�λ�á�
    �������ֻ���Ǳ������Ķ�ʱ���ĳ�ʱʱ���ӳ����������
    �ö�ʱ����Ҫ�������β���ƶ� */
    void adjust_timer( util_timer* timer )
    {
        if( !timer )
        {
            return;
        }
        util_timer* tmp = timer->next;
        /* �����������Ŀ�궨ʱ����������β��������
        �ö�ʱ���µĳ�ʱֵ��ȻС������һ����ʱ���ĳ�ʱֵ�����õ��� */
        if( !tmp || ( timer->expire < tmp->expire ) )
        {
            return;
        }
        /* ���Ŀ�궨ʱ���������ͷ�ڵ㣬�򽫸ö�ʱ����������ȡ�������²������� */
        if( timer == head )
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer( timer, head );
        }
        /* ���Ŀ�궨ʱ�����������ͷ�ڵ㣬�򽫸ö�ʱ����������ȡ����Ȼ��
        ������ԭ������λ��֮��Ĳ��������� */
        else
        {
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer( timer, timer->next );
        }
    }
    /* ��Ŀ�궨ʱ��timer��������ɾ�� */
    void del_timer( util_timer* timer )
    {
        if( !timer )
        {
            return;
        }
        /* �����������������ʾ������ֻ��һ����ʱ������Ŀ�궨ʱ�� */
        if( ( timer == head ) && ( timer == tail ) )
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return;
        }
        /* ���������������������ʱ������Ŀ�궨ʱ���������ͷ��㣬
        �������ͷ�������ԭͷ�ڵ����һ���ڵ㣬Ȼ��ɾ��Ŀ�궨ʱ�� */
        if( timer == head )
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        /* ���������������������ʱ������Ŀ�궨ʱ���������β�ڵ㣬��
        �������β�ڵ�����Ϊԭβ�ڵ��ǰһ���ڵ㣬Ȼ��ɾ��Ŀ�궨ʱ�� */
        if( timer == tail )
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        /* ���Ŀ�궨ʱ��λ��������м䣬�����ǰ��Ķ�ʱ��
        ����������Ȼ��ɾ��Ŀ�궨ʱ�� */
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        delete timer;
    }

    /* SIGALRM�ź�ÿ�α������������źŴ����������ʹ��ͳһ�¼�Դ��
    ��������������ִ��һ��tick�������Դ��������ϵ��ڵ����� */
    void tick()
    {
        if( !head )
        {
            return;
        }
        printf( "timer tick\n" );
        time_t cur = time( NULL );      /* ���ϵͳ��ǰ��ʱ�� */
        util_timer* tmp = head;
        /* ��ͷ��㿪ʼ���δ���ÿ����ʱ����ֱ������һ����δ���ڵĶ�ʱ����
        ����Ƕ�ʱ���ĺ����߼� */
        while( tmp )
        {
            /* ��Ϊÿ����ʱ�������þ���ʱ����Ϊ��ʱֵ���������ǿ���
            �Ѷ�ʱ���ĳ�ʱֵ��ϵͳ��ǰʱ��Ƚϣ����ж϶�ʱ���Ƿ��� */
            if( cur < tmp->expire )
            {
                break;
            }
            /* ���ö�ʱ���Ļص���������ִ�ж�ʱ���� */
            tmp->cb_func( tmp->user_data );
            /* ִ���궨ʱ���еĶ�ʱ����֮�󣬾ͽ�����������ɾ����
            ����������ͷ��� */
            head = tmp->next;
            if( head )
            {
                head->prev = NULL;
            }
            delete tmp;
            tmp = head;
        }
    }

private:
    /* һ�����صĸ����������������е�add_timer������adjust_timer�������á�
    �ú�����ʾ��Ŀ�궨ʱ��timer��ӵ��ڵ�lst_head֮��Ĳ��������� */
    void add_timer( util_timer* timer, util_timer* lst_head )
    {
        util_timer* prev = lst_head;
        util_timer* tmp = prev->next;
        /* ����lst_head�ڵ�֮��Ĳ�������ֱ���ҵ�һ����ʱʱ�����
        Ŀ�궨ʱ���ĳ�ʱʱ��Ľڵ㣬����Ŀ�궨ʱ������ýڵ�֮ǰ */
        while( tmp )
        {
            if( timer->expire < tmp->expire )
            {
                prev->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prev;
                break;
            }
            prev = tmp;
            tmp = tmp->next;
        }
        /* ���������lst_head�ڵ�֮��Ĳ���������δ�ҵ���ʱʱ�����
        Ŀ�궨ʱ���ĳ�ʱʱ��Ľڵ㣬��Ŀ�궨ʱ����������β��������������Ϊ
        �����µ�β�ڵ� */
        if( !tmp )
        {
            prev->next = timer;
            timer->prev = prev;
            timer->next = NULL;
            tail = timer;
        }
        
    }

private:
    util_timer* head;
    util_timer* tail;
};

#endif
