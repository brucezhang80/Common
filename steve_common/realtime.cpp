/*
 * @File: realtime.cpp
 * @Desc: impliment file of realtime,用于超时管理
 * @Author: stevetang
 * @History:
 *      2010-01-19   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "realtime.h"

CRealTimeNode::CRealTimeNode()
{
}

CRealTimeNode::~CRealTimeNode()
{
}

/*
 * Function: Use
 * Desc: 绑定对象
 * In: 
 *      unsigned int  key   键值
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTimeNode::Use(size_t key)
{
	m_key = key;
	m_use = time(NULL);

	return 0;
}

CRealTime::CRealTime(int num, int timeout /* = -1 */) : CLThread("realtime", 1, 1000), /* 间隔1秒 */
m_free(num), m_use(num, 0), m_trash(num, 0), m_timeHash(num), m_useHash(num), m_timeout(timeout)
{
	/* 空闲队列中放入节点 */
	for (int i=0; i<num; i++)
	{
		CRealTimeNode * node = new CRealTimeNode();

		m_free.release(node);
	}

	Start();
}

CRealTime::~CRealTime()
{
	Stop();
}

/*
 * Function: Fill
 * Desc: 处理所有新增状态
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTime::Fill()
{
	/* 将使用队列中的数据取出来放入hash表 */
	while(1)
	{
		CRealTimeNode * node = m_use.get();
		if (node == NULL)
			break;

		m_timeHash.Add((size_t)node, node);
	}

	return 0;
}

/*
 * Function: UnUse
 * Desc: 清除所有未用节点
 * In: 
 *      size_t key
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTime::Clean()
{
	while(1)
	{
		CRealTimeNode * node = m_trash.get();
		if (node == NULL)
			break;

		/* 从超时队列中删除 */
		m_timeHash.Remove((size_t)node);

		/* 放入空闲队列 */
		m_free.release(node);
	}

	return 0;
}

/*
 * Function: EnumAll
 * Desc: 遍历在用节点
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTime::EnumAll()
{
	/* 枚举开始 */
	m_timeHash.EnumBegin();

	time_t t_now = time(NULL);
	while (1)
	{
		/* 枚举节点 */
		CRealTimeNode * node = (CRealTimeNode *)m_timeHash.EnumNext();
		if (node == NULL)
			break;

		/* 检查是否超时，进行超时处理 */
		if ((t_now - node->m_use) >= m_timeout)
		{
			HandleTimeout(node->m_key);

			/* 放入删除队列 */
			m_trash.release(node);
		}
	}

	/* 枚举结束 */
	m_timeHash.EnumEnd();

	return 0;
}

/*
 * Function: HandleThread
 * Desc: 线程业务处理
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTime::HandleThread(int& ibreak)
{
	/* 整理新增 */
	Fill();

	/* 清理删除 */
	Clean();

	if (m_timeout > 0)
	{
		/* 枚举当前，清除垃圾 */
		EnumAll();
	}

	return 0;
}

/*
 * Function: Use
 * Desc: 设置新状态
 * In: 
 *      size_t key
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTime::Use(size_t key)
{
	/* 状态HASH表中取节点 */
	CRealTimeNode * node = (CRealTimeNode *)m_useHash.Get(key);
	if (node != NULL)
	{
		/* 更新时间 */
		node->Use(key);
		return 0;
	}

	/* 取空闲节点 */
	node = m_free.get();
	if (node == NULL)
		return -1;

	/* 更新时间 */
	node->Use(key);

	/* 放入状态HASH */
	m_useHash.Add(key, node);

	/* 放入新增队列 */
	m_use.release(node);

	return 0;
}

/*
 * Function: UnUse
 * Desc: 取消使用
 * In: 
 *      size_t key
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTime::UnUse(size_t key)
{
	/* 状态HASH表中删除节点 */
	CRealTimeNode * node = (CRealTimeNode *)m_useHash.Remove(key);
	if (node == NULL)
		return -1;

	/* 放入删除队列 */
	m_trash.release(node);

	return 0;
}

/*
 * Function: WakeUp
 * Desc: 唤醒
 * In: 
 *      size_t key
 * Out: 
 *      none
 * Return code: 
 *      0  -   成功
 *      -1 -   失败
 */
int CRealTime::WakeUp(unsigned char isrelease /* = 1 */)
{
	m_free.WakeUpAll(isrelease);

	return 0;
}
