#include "runable.h"

bool CRunable::Start()
{
    pthread_attr_t attr;

    unsigned int err=pthread_attr_init(&attr);

    if(err!=0)
        return false;

    unsigned int stack_size=(1<<20)*20;

    pthread_attr_setstacksize(&attr, stack_size);
    
    if (pthread_create(&_pid, &attr, &svc, this)!=0)
    {
        return false;
    }

    return true;
}

void CRunable::Stop()
{
	pthread_exit(NULL);
}

void* svc(void *arg)
{
	pthread_detach(pthread_self());
	
	CRunable *task = (CRunable*)arg;
	if (task==NULL)
	{
		return NULL;
	}
	
	task->Run();

      	return NULL;
}

/*
*$id:$
*/

