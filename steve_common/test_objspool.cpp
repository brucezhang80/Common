#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#include "objspool.h"

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

class CMyPool : public CObjsPool<CMsgObj>
{
	int i;
public: 
	CMyPool(int num) : CObjsPool<CMsgObj>(num) 
	{
		i = 0;
		init();
	};
	~CMyPool() {};

	CMsgObj* newobject()
	{
		++i;
		return new CMsgObj(i);
	}
};

int loopnum = 1;

void *
runThread(void *arg)
{
	CMyPool	*myPool = (CMyPool	*)arg;
	int64 _s, _e;
	
	_s = GetAbsTime();
	for( int i=0; i<loopnum; ++i)
	{
		CMsgObj *p = myPool->get();

/*		if ( NULL != p )
			printf("thread[%lu], seq=%d, num=%d, idle_num=%d\n", 
				pthread_self(), p->seq, myPool->get_num(), myPool->get_idle_num()
				);
		else
			printf("thread[%lu], no more obj\n", pthread_self() );
*/
		usleep(1);
		if ( NULL != p )
			myPool->release(p);
	}
	_e = GetAbsTime();

//	printf("thread[%lu], num=%d, idle_num=%d\n", pthread_self(), myPool->get_num(), myPool->get_idle_num());

	printf("thread[%lu], speed=%.3f\n", pthread_self(), (int64)loopnum*1000.0*1000.0/(_e-_s));

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
	
	count = atoi(argv[1]);
	loopnum = atoi(argv[2]);		
	if ( argc > 3 )
		thread_num = atoi(argv[3]);
		
	CMyPool	*myPool = new CMyPool(count);

#define MacThreadNum	2000
    pthread_t    t[MacThreadNum];

	if ( thread_num > MacThreadNum )
		thread_num = MacThreadNum;
	for(int m=0;m<thread_num;++m)
	{
	     pthread_create(&t[m], NULL, runThread, myPool);
	}  

	printf("\twaiting for thread exit!\n");
	for(int m=0;m<thread_num;++m)
	{
	     pthread_join( t[m] , NULL );
	}	

	printf("\tall thread exit!\n");

	printf("test done\n");

	return 0;
}

