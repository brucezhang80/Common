#include <sys/time.h>
#include <string.h>

#include "sp_util.h"

/*
* 取当前时间usec
*/
TUINT64  GetAbsTime()
{
	struct timeval now;
	gettimeofday(&now, 0);
	return (TUINT64 )now.tv_sec*1000000+(TUINT64 )now.tv_usec;
}


/*
* 取当前时间msec
*/
TUINT64  GetTime()
{
	struct timeval now;
	gettimeofday(&now, 0);
	return (TUINT64 )now.tv_sec*1000+(TUINT64 )now.tv_usec/1000;
}

/*
* 将某个时间转为usec
*/
TUINT64 ToAbsTime(struct timeval now)
{
	return (TUINT64)now.tv_sec* 1000000+(TUINT64)now.tv_usec;
}

/* 
* 根据日志级别名称转为日志级别
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

