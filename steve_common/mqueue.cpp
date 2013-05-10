#include "general.h"
#include "mqueue.h"

CBufQueue::CBufQueue() : _header(NULL), _data(NULL)
{
}

CBufQueue::~CBufQueue()
{
}

int CBufQueue::Create(char* pBuf, unsigned int iBufSize) // attach and init 
{
	if (iBufSize <= sizeof(Head)+sizeof(unsigned int)+ReserveLen)
		return -1;

	_header = (Head *)pBuf;
	_data = pBuf+sizeof(Head);

	_header->iBufSize = iBufSize - sizeof(Head);
	_header->iReserveLen = ReserveLen;
	_header->iBegin = 0;
	_header->iEnd = 0;

	return 0;
}

int CBufQueue::Attach(char* pBuf)
{
	_header = (Head *)pBuf;
	_data = pBuf+sizeof(Head);

	if(_header->iReserveLen != ReserveLen)
		return -1;

	return 0;
}

int CBufQueue::Dequeue(char *buffer, unsigned int& buffersize)
{
	if (_header == NULL || _data == NULL) {
		return -1;
	}

	/* 是否不存在数据 */
	if(IsEmpty()) {
		return 0;
	}

	if(_header->iEnd > _header->iBegin) {
		/* 数据未分段 */
		if (_header->iBegin+sizeof(unsigned int) >= _header->iEnd)
			return -1;
		unsigned int len = GetLen(_data+_header->iBegin);
		if (_header->iBegin+sizeof(unsigned int)+len > _header->iEnd)
			return -1;
		if(len > buffersize) {
			_header->iBegin += len+sizeof(unsigned int);
			buffersize = 0;
			return -1;
		}
		buffersize = len;
		memcpy(buffer,_data+_header->iBegin+sizeof(unsigned int),len);
		_header->iBegin += len+sizeof(unsigned int);
	} else {
		// 被分段
		if (_header->iEnd+ReserveLen > _header->iBegin)
			return -1;
		unsigned int len = 0;
		unsigned int new_begin = 0;
		char *data_from = NULL;
		char *data_to = NULL;
		if (_header->iBegin+1 > _header->iBufSize)
			return -1;
		// 长度字段也被分段
		if(_header->iBegin+sizeof(unsigned int)-1 >= _header->iBufSize){			
			char tmp[4];
			unsigned len_in_tail_size = _header->iBufSize - _header->iBegin;
			unsigned len_in_head_size = sizeof(unsigned int) - len_in_tail_size;		
			memcpy(tmp, _data + _header->iBegin, len_in_tail_size);		
			memcpy(tmp + len_in_tail_size, _data, len_in_head_size);
			len = GetLen(tmp);
			data_from = _data + len_in_head_size;
			new_begin = len_in_head_size + len;
			if (new_begin > _header->iEnd)
				return -1;
		}else {
			len = GetLen(_data+_header->iBegin);
			data_from = _data+_header->iBegin+sizeof(unsigned int);
			if(data_from == _data+_header->iBufSize) data_from = _data;
			if(_header->iBegin+sizeof(unsigned int)+len < _header->iBufSize) { 
				new_begin = _header->iBegin+sizeof(unsigned int)+len;
			} else { // 数据被分段
				new_begin = _header->iBegin+sizeof(unsigned int)+len-_header->iBufSize;
				if (new_begin > _header->iEnd)
					return -1;
			}
		}
		data_to = _data+new_begin;
		_header->iBegin = new_begin;

		if(len > buffersize) {
			buffersize = 0;
			return -1;
		}
		buffersize = len;
		if(data_to > data_from) {
			if ((unsigned int)(data_to - data_from) != len)
				return -1;
			memcpy(buffer,data_from,len);
		} else {
			memcpy(buffer,data_from,_data-data_from+_header->iBufSize);
			memcpy(buffer+(_data-data_from+_header->iBufSize),_data,data_to-_data);
			if (_header->iBufSize-(data_from-data_to) != len)
				return -1;
		}
	}

	return buffersize;
}

int CBufQueue::Enqueue(const char *buffer,unsigned int len)
{
	if (_header == NULL || _data == NULL || len > _header->iBufSize) {
		return -1;
	}

	if(IsFull(len)) {
		return 0;
	}

	// 长度字段被分段
	if(_header->iEnd+sizeof(unsigned int)-1 >= _header->iBufSize)
	{
		char tmp[4]; SetLen(tmp, len);
		unsigned len_in_tail_size = _header->iBufSize - _header->iEnd;
		unsigned len_in_head_size = sizeof(unsigned int) - len_in_tail_size;		
		memcpy(&_data[_header->iEnd], tmp, len_in_tail_size);		
		memcpy(_data, &tmp[len_in_tail_size], len_in_head_size);
		memcpy(_data+len_in_head_size, buffer, len);
		_header->iEnd = len_in_head_size + len;
		if (_header->iEnd+ReserveLen > _header->iBegin)
			return -1;
	}	
	// 数据被分段
	else if(_header->iEnd+sizeof(unsigned int)+len > _header->iBufSize){
		unsigned data_in_tail_size = _header->iBufSize-_header->iEnd-sizeof(unsigned int);
		SetLen(_data+_header->iEnd,len);
		memcpy(_data+_header->iEnd+sizeof(unsigned int),buffer,data_in_tail_size);
		memcpy(_data,buffer+data_in_tail_size,len-data_in_tail_size);
		_header->iEnd = len-data_in_tail_size;
		if (_header->iEnd+ReserveLen > _header->iBegin)
			return -1;
	} else {
		SetLen(_data+_header->iEnd,len);
		memcpy(_data+_header->iEnd+sizeof(unsigned int),buffer,len);
		_header->iEnd = (_header->iEnd+sizeof(unsigned int)+len)%_header->iBufSize;
	}

	return len;
}

bool CBufQueue::IsFull(unsigned int len)
{
	if(_header->iEnd == _header->iBegin) {
		if(len+sizeof(unsigned int)+ReserveLen > _header->iBufSize) 
			return true;

		return false;
	}

	if(_header->iEnd > _header->iBegin) {
		return _header->iBufSize - _header->iEnd + _header->iBegin < sizeof(unsigned int)+len+ReserveLen;
	}

	return (_header->iBegin - _header->iEnd < sizeof(unsigned int)+len+ReserveLen);
}

unsigned int CBufQueue::MsgBytes()
{
	unsigned int begin, end;
	begin = _header->iBegin;
	end = _header->iEnd;
	if(end >= begin)
	{
		return end - begin;
	}
	
	return _header->iBufSize - begin + end;	
}

CMQueue::CMQueue(int key, int semkey) : m_shm_key(key), m_lock(semkey)
{
}

CMQueue::~CMQueue()
{
}

int CMQueue::init(int queuesize /* = 0 */, unsigned char init /* = 0 */)
{
	CAutoLock l(&m_lock);

	if (queuesize > 0)
	{
		/* 打开共享内存 */
		int ret = m_shm.force_open_and_attach((key_t)m_shm_key, (size_t)queuesize, init);
		if(ret < 0)
		{
			return -1;
		}

		/* 构造QUEUE */
		if (m_queue.Create((char *)m_shm.get_segment_ptr(), queuesize))
		{
			return -1;
		}
	}
	else
	{
		/* 打开共享内存 */
		int ret = m_shm.open_and_attach((key_t)m_shm_key, 0);
		if(ret < 0)
		{
			return -1;
		}

		/* 跟QUEUE绑定 */
		if (m_queue.Attach((char *)m_shm.get_segment_ptr()))
		{
			return -1;
		}
	}

	return 0;
}

int CMQueue::Dequeue(char *buffer,unsigned int& len)
{
	CAutoLock l(&m_lock);

	return m_queue.Dequeue(buffer, len);
}

int CMQueue::Enqueue(const char *buffer,unsigned int len)
{
	CAutoLock l(&m_lock);

	return m_queue.Enqueue(buffer, len);
}
