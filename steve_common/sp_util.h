#ifndef __SP_UTIL_H_
#define __SP_UTIL_H_

#include <unistd.h>


#ifdef WIN32
    typedef __int64                 TINT64;
    typedef unsigned __int64        TUINT64;
#else
    #include <sys/types.h>
    typedef __int64_t               TINT64;
    typedef __uint64_t              TUINT64;
#endif

TUINT64 GetAbsTime();

TUINT64 ToAbsTime(struct timeval now);

TUINT64 GetTime();

int get_log_level(const char *s);

#endif
