#ifndef _MQUEUE_H
#define _MQUEUE_H

#include "lock.h"
#include "shm.h"

class CBufQueue
{
public:
	CBufQueue();
	~CBufQueue();

	int Create(char* pBuf, unsigned int iBufSize); // attach and init 
	int Attach(char* pBuf);

	int Dequeue(char *buffer,unsigned int& buffersize);
	int Enqueue(const char *buffer,unsigned int len);
	bool IsEmpty() const {return _header->iBegin == _header->iEnd;}
	bool IsFull(unsigned int len);

	unsigned int MsgBytes();

private:
	unsigned int GetLen(char *buf) {unsigned int ui; memcpy((void *)&ui,buf,sizeof(unsigned int)); return ui;}
	void SetLen(char *buf,unsigned int ui) {memcpy(buf,(void *)&ui,sizeof(unsigned int));}

private:
	const static unsigned int ReserveLen = 8;
	typedef struct Head
	{
		unsigned int iBufSize;
		unsigned int iReserveLen; // must be 8
		unsigned int iBegin;
		unsigned int iEnd;
	};

	Head *_header;
	char *_data;
};

class CMQueue
{
	int m_shm_key; /* 共享内存键值 */

    CSVShm m_shm; /* 共享内存 */
	CSemLock m_lock; /* 锁 */

	CBufQueue m_queue; /* 队列 */

public:
	CMQueue(int key, int semkey);
	~CMQueue();

public:
	int init(int queuesize = 0, unsigned char init = 0);
	int Dequeue(char *buffer,unsigned int& len);
	int Enqueue(const char *buffer,unsigned int len);
};

#endif
