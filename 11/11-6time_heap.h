#ifndef intIME_HEAP
#define intIME_HEAP

#include <iostream>
#include <netinet/in.h>
#include <time.h>
using std::exception;

#define BUFFER_SIZE 64

class heap_timer;       // ǰ������
/* ��socket�Ͷ�ʱ�� */
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[ BUFFER_SIZE ];
    heap_timer* timer;
};

/* ��ʱ���� */
class heap_timer
{
public:
    heap_timer( int delay )
    {
        expire = time( NULL ) + delay;
    }

public:
   time_t expire;                       // ��ʱ����Ч�ľ���ʱ��
   void (*cb_func)( client_data* );     // ��ʱ���Ļص�����
   client_data* user_data;              // �û�����
};

/* ʱ����� */
class time_heap
{
public:
    /* ���캯��֮һ����ʼ��һ����СΪcap�Ŀն� */
    time_heap( int cap ) throw ( std::exception )
        : capacity( cap ), cur_size( 0 )
    {
        array = new heap_timer* [capacity];     // ����������
        if ( ! array )
        {
                throw std::exception();
        }
        for( int i = 0; i < capacity; ++i )
        {
            array[i] = NULL;
        }
    }
    /* ���캯��֮������������������ʼ���� */
    time_heap( heap_timer** init_array, int size, int capacity ) throw ( std::exception )
        : cur_size( size ), capacity( capacity )
    {
        if ( capacity < size )
        {
            throw std::exception();
        }
        array = new heap_timer* [capacity];     //����������
        if ( ! array )
        {
            throw std::exception();
        }
        for( int i = 0; i < capacity; ++i )
        {
            array[i] = NULL;
        }
        if ( size != 0 )
        {
            /* ��ʼ�������� */
            for ( int i =  0; i < size; ++i )
            {
                array[ i ] = init_array[ i ];
            }
            for ( int i = (cur_size-1)/2; i >=0; --i )
            {
                /* �������еĵ�(cur_size-1)/2~0��Ԫ��ִ�����ǲ��� */
                percolate_down( i );
            }
        }
    }
    /* ����ʱ��� */
    ~time_heap()
    {
        for ( int i =  0; i < cur_size; ++i )
        {
            delete array[i];
        }
        delete [] array; 
    }

public:
    /* ���Ŀ�궨ʱ��timer */
    void add_timer( heap_timer* timer ) throw ( std::exception )
    {
        if( !timer )
        {
            return;
        }
        if( cur_size >= capacity )      // �����ǰ������������������������1��
        {
            resize();
        }
        /* �²�����һ��Ԫ�أ���ǰ�Ѵ�С��1��hole���½���Ѩ��λ�� */
        int hole = cur_size++;
        int parent = 0;
        /* �Ѵӿ�Ѩ�����ڵ��·���ϵ����нڵ�ִ�����ǲ��� */
        for( ; hole > 0; hole=parent )
        {
            parent = (hole-1)/2;
            if ( array[parent]->expire <= timer->expire )
            {
                break;
            }
            array[hole] = array[parent];
        }
        array[hole] = timer;
    }
    /* ɾ��Ŀ�궨ʱ��timer */
    void del_timer( heap_timer* timer )
    {
        if( !timer )
        {
            return;
        }
        /* ������Ŀ�궨ʱ���Ļص���������Ϊ�գ�����ν��
        �ӳ����١��⽫��ʡ����ɾ���ö�ʱ����ɵĿ�������
        ����������ʹ���������� */
        // lazy delelte
        timer->cb_func = NULL;
    }
    /* ��öѶ����Ķ�ʱ�� */
    heap_timer* top() const
    {
        if ( empty() )
        {
            return NULL;
        }
        return array[0];
    }
    /* ɾ���Ѷ����Ķ�ʱ�� */
    void pop_timer()
    {
        if( empty() )
        {
            return;
        }
        if( array[0] )
        {
            delete array[0];
            /* ��ԭ���ĶѶ�Ԫ���滻Ϊ�����������һ��Ԫ�� */
            array[0] = array[--cur_size];
            percolate_down( 0 );        // ���µĶѶ�Ԫ��ִ�����ǲ���
        }
    }
    /* �Ĳ����� */
    void tick()
    {
        heap_timer* tmp = array[0];
        time_t cur = time( NULL );      // ѭ��������е��ڵĶ�ʱ��
        while( !empty() )
        {
            if( !tmp )
            {
                break;
            }
            /* ����Ѷ�ʱ��û���ڣ����˳�ѭ�� */
            if( tmp->expire > cur )
            {
                break;
            }
            /* �����ִ�жѶ���ʱ���е����� */
            if( array[0]->cb_func )
            {
                array[0]->cb_func( array[0]->user_data );
            }
            /* ���Ѷ�Ԫ��ɾ����ͬʱ�����µĶѶ���ʱ����array[0]�� */
            pop_timer();
            tmp = array[0];
        }
    }
    bool empty() const { return cur_size == 0; }

private:
    /* ��С�ѵ����ǲ�������ȷ�����������Ե�hole���ڵ���Ϊ��������ӵ����С������ */
    void percolate_down( int hole )
    {
        heap_timer* temp = array[hole];
        int child = 0; 
        for ( ; ((hole*2+1) <= (cur_size-1)); hole=child )
        {
            child = hole*2+1;
            if ( (child < (cur_size-1)) && (array[child+1]->expire < array[child]->expire ) )
            {
                ++child;
            }
            if ( array[child]->expire < temp->expire )
            {
                array[hole] = array[child];
            }
            else
            {
                break;
            }
        }
        array[hole] = temp;
    }
    /* ����������������1�� */
    void resize() throw ( std::exception )
    {
        heap_timer** temp = new heap_timer* [2*capacity];
        for( int i = 0; i < 2*capacity; ++i )
        {
            temp[i] = NULL;
        }
        if ( ! temp )
        {
            throw std::exception();
        }
        capacity = 2*capacity;
        for ( int i = 0; i < cur_size; ++i )
        {
            temp[i] = array[i];
        }
        delete [] array;
        array = temp;
    }

private:
    heap_timer** array;     // ������
    int capacity;           // �����������
    int cur_size;           // �����鵱ǰ����Ԫ�صĸ���
};

#endif
