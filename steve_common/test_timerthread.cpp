#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

#include "timerthread.h"

#ifndef int64
typedef __int64_t	int64;
#endif
/*
inline int64 GetAbsTime()
{
	struct timeval now;
	gettimeofday(&now, 0);
	return (int64)now.tv_sec*1000000+(int64)now.tv_usec;
}
*/
int speed = 1000;
int exitflag = 0;
unsigned int loopnum = 10000000;


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

class CMyTT : public CTimerThread 
{
public: 
	CMyTT(int a, int b) : CTimerThread(a,b) {};
	~CMyTT() {};
	void vHandleTimeout(void *p)
	{
		CMsgObj *ptr = (CMsgObj *)p;
		++timeout;

		int64 n = GetAbsTime();
		timeout2[toi] = n-ptr->_time;
		if ( timeout2[toi] < 1000 )
		{
			printf("h=%d, %llu - %llu = %llu\n", ptr->h, n, ptr->_time, n-ptr->_time);
		}
		++toi;
		
		delete ptr;
		ptr = NULL;

		if ( 0 == timeout%100000 )
		{
//			printf("CMyTT::vHandleTimeout(). timeout num=%d\n", timeout);
		}
	}
	
	/*
	* hash函数
	*/
	/*
	int nHashFunc(unsigned long key)
	{
		return (int)(key%10);
	}
	*/
};

typedef struct
{
	int seq;
	CMyTT *pMyTT;
	unsigned int nStartNum;
	unsigned int nEndNum;
	unsigned int ii;
	unsigned int cc;
} TArg;

inline int nToMsec(TUINT64 nAbsMsec)
{
	// (m_nTimeBucket/1000) = 最大超时秒数
	return (int)(((nAbsMsec/1000)%600)*1000+nAbsMsec%1000);
}

void *
putThread(void *arg)
{

	 TArg *pa = (TArg *)arg;
	 CMyTT *ptt = pa->pMyTT;	
	 pa->ii = pa->nStartNum;

	printf("\tput thread#%d(%lu), from %u to %u\n", pa->seq, pthread_self(), pa->nStartNum, pa->nEndNum);
	 
     while( pa->ii < pa->nEndNum)
     {
     	for(int j=0; (j<speed)&&(pa->ii<pa->nEndNum); ++j )
     	{     	
			CMsgObj *p = new CMsgObj(pa->ii);
			p->_time = GetAbsTime();
			p->h = (nToMsec(p->_time/1000)+1)%(600*1000u);
    	 	ptt->nPut(p->seq, (void*)p);
    	 	++pa->ii;

	     	if ( 0 == pa->ii%10000 )
	     	{
	     		printf("\tput thread#%d(%lu), ii=%u\n", pa->seq, pthread_self(), pa->ii);
	     	}
     	}
     	sleep(1);
     }

     exitflag = 1;
     sleep(1);
	printf("\tput thread#%d(%lu), total cc=%u\n", pa->seq, pthread_self(), pa->ii-pa->nStartNum);

    return 0;
}

void *
getThread(void *arg)
{
	 TArg *pa = (TArg *)arg;
	 CMyTT *ptt = pa->pMyTT;	

     while( 0 == exitflag )
     {
 		int64 _s = GetAbsTime();
 		int k = pa->ii-_s%100;
 		if ( k < 0 )
 			continue;
		CMsgObj *ptr = (CMsgObj *)ptt->pvGet(k);
		if ( NULL != ptr )
		{		
			delete ptr;
			ptr = NULL;
			++pa->cc;
		}

     	usleep(1);
     }

	printf("\tget thread#%d(%lu), total cc=%u\n", pa->seq, pthread_self(), pa->cc);

    return 0;	
}

int main(int argc, char *argv[])
{

	int64 ccc = 0;
	int _max=0, _min=999999;
	
	if ( argc < 5 )
	{
		printf("Usage: %s <hash size> <bucket> <pthread put speed> <total put num>\n", argv[0]);
		printf("\nexample:\t./%s 1000000 1000000  100 100000 \n", argv[0]);
		printf("\n");
		return 0;
	}

	
	int count = atoi(argv[1]);
	int bucket = atoi(argv[2]);	
	speed = atoi(argv[3]);
	loopnum = atoi(argv[4]);

	timeout2 = new int[count];
	memset(timeout2,0,sizeof(int)*count);

	int i;
	CMyTT tt(count, bucket);	

	int64 _s, _e;
	

	tt.Start();

/*
	// test put
	CMsgObj *p = new CMsgObj(0);
	p->_time = GetAbsTime();
	p->h = (nToMsec(p->_time/1000)+1)%(600*1000);
 	tt.nPut(p->seq, (void*)p);
*/
	
	CMsgObj **pp;

	pp = new CMsgObj*[count];
	for( i=0; i<count; ++i)
		pp[i] = new CMsgObj(i);

	// 1.test put speed
	printf("1. test put speed\n");
	_s = GetAbsTime();
	for( i=0; i<count; ++i)
	{
		pp[i]->_time = GetAbsTime();
		pp[i]->h = (nToMsec(pp[i]->_time/1000)+1)%(600*1000u);
		tt.nPut(pp[i]->seq, (void*)pp[i]);
	}	
	_e = GetAbsTime();
	if ( _e != _s )
	{
		printf("\telapse=%lld(usec), avg(%d) put speed=%.3f\n", _e-_s, count, (int64)count*1000.*1000./(_e-_s));
	}	
	printf("\ttotal put num=%d\n\n", count);

	// 2.test get speed
	// 由于get与put不同步，可能get不到数据
	printf("2. test get speed\n");

	int cc = 0;
	_s = GetAbsTime();	
	for( i=0; i<count; ++i)
	{
		CMsgObj *ptr = (CMsgObj *)tt.pvGet(i);
		if ( NULL != ptr )
		{
			delete ptr;
			ptr = NULL;
			++cc;
		}
	}
	_e = GetAbsTime();
	if ( _e != _s )
	{
		printf("\telapse=%lld(usec), avg(%d) get speed=%.3f\n", _e-_s, count, (int64)count*1000.*1000./(_e-_s));
	}	
	printf("\ttotal get num=%d\n\n", cc);

	printf("\tsleep 2s, wait for all node timeout\n\n");
	sleep(2);

	// 计算延时
	ccc = 0;
	_max=0;
	_min=999999;
	for(int m=0; m<timeout; ++m)
	{
		if ( timeout2[m] > _max )
			_max = timeout2[m];
		if ( timeout2[m] < _min )
			_min = timeout2[m];
			
		ccc += timeout2[m];
	}
	printf("\ttimeout=%d, avg=%.3f(usec), max=%d(usec), min=%d(usec)\n", timeout, ccc*1.0/timeout, _max, _min);
	printf("\n");
	// 3.test threads put&get speed
	memset(timeout2,0,sizeof(int)*count);	
	timeout = 0;
	toi = 0;
	printf("3.test random put&get speed\n");

#define MacThreadNum	20
    pthread_t    t[MacThreadNum], t1[MacThreadNum];
	TArg a[MacThreadNum];
	
	for(int m=0;m<MacThreadNum;++m)
	{
		a[m].seq = m;
		a[m].pMyTT = &tt;
		a[m].nStartNum = m*loopnum/MacThreadNum;
		a[m].nEndNum = loopnum/MacThreadNum*(m+1);
		a[m].ii = 0;
		a[m].cc = 0;		

	     pthread_create(&t[m], NULL, putThread, &a[m]);
	     pthread_create(&t1[m], NULL, getThread, &a[m]);
	}  

	printf("\twaiting for thread exit!\n");
	for(int m=0;m<MacThreadNum;++m)
	{
	     pthread_join( t[m] , NULL );
	     pthread_join( t1[m] , NULL );
	}	

	printf("\tall thread exit!\n");
	
	ccc = 0;
	for(int m=0;m<MacThreadNum;++m)
	{
		ccc += a[m].cc;
	}
	printf("\tget=%lld\n", ccc);

	// 计算延时
	ccc = 0;
	_max=0;
	_min=999999;
	for(int m=0; m<timeout; ++m)
	{
		if ( timeout2[m] > _max )
			_max = timeout2[m];
		if ( timeout2[m] < _min )
			_min = timeout2[m];
	
		ccc += timeout2[m];
	}
	printf("\ttimeout=%d, avg=%.3f(usec), max=%d(usec), min=%d(usec)\n", timeout, ccc*1.0/timeout, _max, _min);

	// 等待退出
	printf("\tsleep 2s\n");
	sleep(2);

	printf("\ttest done\n");
	
	return 0;
}

