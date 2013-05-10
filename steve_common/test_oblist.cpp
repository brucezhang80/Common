#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#include "oblist.h"

#ifndef int64
typedef __int64_t	int64;
#endif


inline int64 GetAbsTime()
{
	struct timeval now;
	gettimeofday(&now, 0);
	return (int64)now.tv_sec*1000000+(int64)now.tv_usec;
}


class CMsgObj
{
public:
	int seq;
	int64 _time;
	int h;
	
	CMsgObj(int i):seq(i) {}
	~CMsgObj() {}
};

int timeout =0;
int *timeout2;
int toi = 0;

int loopnum = 1;

void *
releaseThread(void *arg)
{
	CObList <CMsgObj>	*pQueue =  (CObList <CMsgObj>	*)arg;
	int64 _s, _e;
	
	CMsgObj p(1);
	
	_s = GetAbsTime();
	for( int i=0; i<loopnum; ++i)
	{
		pQueue->release(&p);
//		CMsgObj *a = pQueue->get();

/*		if ( NULL != p )
			printf("thread[%lu], seq=%d, num=%d, idle_num=%d\n", 
				pthread_self(), p->seq, myPool->get_num(), myPool->get_idle_num()
				);
		else
			printf("thread[%lu], no more obj\n", pthread_self() );
*/
//		usleep(1);		
	}
	_e = GetAbsTime();

	sleep(2);
	pQueue->WakeUp();
//	printf("thread[%lu], num=%d, idle_num=%d\n", pthread_self(), myPool->get_num(), myPool->get_idle_num());

	printf("release thread[%lu], speed=%.3f\n", pthread_self(), (int64)loopnum*1000.0*1000.0/(_e-_s));

    return 0;
}

void *
getThread(void *arg)
{
	CObList <CMsgObj>	*pQueue =  (CObList <CMsgObj>	*)arg;
	int64 _s, _e;
	int cc = 0;
	CMsgObj p(1);
	
	_s = GetAbsTime();
	for( int i=0; i<loopnum; ++i)
	{
//		pQueue->release(&p);
		CMsgObj *a = pQueue->get();
		if ( NULL == a )
			break;

		++cc;
/*		if ( NULL != p )
			printf("thread[%lu], seq=%d, num=%d, idle_num=%d\n", 
				pthread_self(), p->seq, myPool->get_num(), myPool->get_idle_num()
				);
		else
			printf("thread[%lu], no more obj\n", pthread_self() );
*/
//		usleep(1);		
	}
	_e = GetAbsTime();

//	printf("thread[%lu], num=%d, idle_num=%d\n", pthread_self(), myPool->get_num(), myPool->get_idle_num());

	printf("get thread[%lu], cc=%d, speed=%.3f\n", pthread_self(), cc, (int64)loopnum*1000.0*1000.0/(_e-_s));

    return 0;
}

int main(int argc, char *argv[])
{	
	int count = 0;
	int thread_num = 1;
	if ( argc < 3 )
	{
		printf("Usage: %s <count> <loopnum> [thread_num]\n", argv[0]);
		printf("example: %s 1000000 5000000\n", argv[0]);
		printf("         %s 1000000 5000000 100\n", argv[0]);
		printf("\n");
		return 0;		
	}
	
	CObList <CMsgObj>		*m_pQueue;

	count = atoi(argv[1]);
	loopnum = atoi(argv[2]);		
	if ( argc > 3 )
		thread_num = atoi(argv[3]);
		
	m_pQueue = new CObList<CMsgObj> [count];

#define MacThreadNum	2000
    pthread_t    t[MacThreadNum];
/*
	if ( thread_num > MacThreadNum )
		thread_num = MacThreadNum;
	for(int m=0;m<thread_num;++m)
	{
	     pthread_create(&t[m], NULL, runThread, m_pQueue);
	}  

	printf("\twaiting for thread exit!\n");
	for(int m=0;m<thread_num;++m)
	{
	     pthread_join( t[m] , NULL );
	}	
*/
     pthread_create(&t[1], NULL, getThread, m_pQueue);
     pthread_create(&t[2], NULL, getThread, m_pQueue);
     pthread_create(&t[3], NULL, getThread, m_pQueue);
     pthread_create(&t[0], NULL, releaseThread, m_pQueue);
     pthread_create(&t[4], NULL, releaseThread, m_pQueue);
     pthread_create(&t[5], NULL, releaseThread, m_pQueue);
     
	printf("\twaiting for thread exit!\n");

     pthread_join( t[1] , NULL );
     pthread_join( t[2] , NULL );
     pthread_join( t[3] , NULL );
     pthread_join( t[0] , NULL );
     pthread_join( t[4] , NULL );
     pthread_join( t[5] , NULL );

	printf("\tall thread exit!\n");

	printf("test done\n");

	return 0;
}

