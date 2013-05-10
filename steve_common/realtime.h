/*
 * @File: realtime.h
 * @Desc: head file of realtime,用于超时管理
 * @Author: stevetang
 * @History:
 *      2010-01-18   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _REALTIME_H
#define _REALTIME_H

#include "oblist.h"
#include "hash.h"
#include "lthread.h"

class CRealTime;

class CRealTimeNode
{
	friend class CRealTime;

public:
	CRealTimeNode();
	~CRealTimeNode();

protected:
	size_t m_key; /* 键值 */
	time_t  m_use; /* 使用时间 */

public:
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
	int Use(size_t key);
};

class CRealTime : public CLThread
{
	CObList<CRealTimeNode>  m_free; /* 空闲节点 */
	CObList<CRealTimeNode>  m_use; /* 新增使用节点 */
	CObList<CRealTimeNode>  m_trash; /* 废弃节点 */

	CHash m_timeHash; /* 用于超时检查，键值为CRealTimeNode指针 */
	CHash m_useHash; /* 用于设置更新，键值为key */

	unsigned int m_timeout; /* 以秒为单位 */

public:
	CRealTime(int num, int timeout = -1);
	~CRealTime();

private:
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
	int Fill();

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
	int Clean();

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
	int EnumAll();

    /*
     * Function: HandleTimeout
     * Desc: 超时处理
     * In: 
     *      size_t key
     * Out: 
     *      none
     * Return code: 
     *      0  -   成功
     *      -1 -   失败
     */
	virtual int HandleTimeout(size_t key) = 0;

protected:
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
    virtual int HandleThread(int& ibreak);

public:
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
	int Use(size_t key);

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
	int UnUse(size_t key);

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
	int WakeUp(unsigned char isrelease = 1);

	int GetFreeNum() { return m_free.GetCount(); };
	int GetNewNum() { return m_use.GetCount(); };
	int GetTrashNum() { return m_trash.GetCount(); };
	int GetTimeNum() { return m_timeHash.GetNodeNum(); };
	int GetUseNum() { return m_useHash.GetNodeNum(); };
};

#endif
