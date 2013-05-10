#include <sys/time.h>
#include <string.h>

#include "sp_util.h"

/*
* ȡ��ǰʱ��usec
*/
TUINT64  GetAbsTime()
{
	struct timeval now;
	gettimeofday(&now, 0);
	return (TUINT64 )now.tv_sec*1000000+(TUINT64 )now.tv_usec;
}


/*
* ȡ��ǰʱ��msec
*/
TUINT64  GetTime()
{
	struct timeval now;
	gettimeofday(&now, 0);
	return (TUINT64 )now.tv_sec*1000+(TUINT64 )now.tv_usec/1000;
}

/*
* ��ĳ��ʱ��תΪusec
*/
TUINT64 ToAbsTime(struct timeval now)
{
	return (TUINT64)now.tv_sec* 1000000+(TUINT64)now.tv_usec;
}

/* 
* ������־��������תΪ��־����
*/
int get_log_level(const char *s)
{
	if(strcasecmp(s,"debug")==0) {
		return 0;
	} else if(strcasecmp(s,"info")==0) {
		return 1;
	} else if(strcasecmp(s,"warning")==0) {
		return 2;
	} else if(strcasecmp(s,"error")==0) {
		return 3;
	} else if(strcasecmp(s,"fatal")==0) {
		return 4;
	} 
	return 4;
}

