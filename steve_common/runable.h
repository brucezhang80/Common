/**
*  /brief	active thread interface
* 
*  create date: 2005/6/17
*/

#ifndef __RUNABLE_H_
#define __RUNABLE_H_

#include <pthread.h>

class CRunable
{
public:
	CRunable()
	{};
	
	virtual ~CRunable()
	{};	

	/*
	* 描述：起动函数
	*
	*/
	bool	 Start();

	virtual void	 Stop();

	/*
	* 描述：真正的入口函数，派生类必须实现它
	*
	*/
	virtual void Run()=0;
	
private:
	pthread_t _pid;
	
};

extern "C" void* svc(void *arg);

#endif//__RUNABLE_H_

/*
*$id:$
*/

