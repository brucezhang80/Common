/*
 * @File: realtime.h
 * @Desc: head file of realtime,���ڳ�ʱ����
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
	size_t m_key; /* ��ֵ */
	time_t  m_use; /* ʹ��ʱ�� */

public:
    /*
     * Function: Use
     * Desc: �󶨶���
     * In: 
	 *      unsigned int  key   ��ֵ
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Use(size_t key);
};

class CRealTime : public CLThread
{
	CObList<CRealTimeNode>  m_free; /* ���нڵ� */
	CObList<CRealTimeNode>  m_use; /* ����ʹ�ýڵ� */
	CObList<CRealTimeNode>  m_trash; /* �����ڵ� */

	CHash m_timeHash; /* ���ڳ�ʱ��飬��ֵΪCRealTimeNodeָ�� */
	CHash m_useHash; /* �������ø��£���ֵΪkey */

	unsigned int m_timeout; /* ����Ϊ��λ */

public:
	CRealTime(int num, int timeout = -1);
	~CRealTime();

private:
    /*
     * Function: Fill
     * Desc: ������������״̬
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Fill();

    /*
     * Function: UnUse
     * Desc: �������δ�ýڵ�
     * In: 
     *      size_t key
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Clean();

    /*
     * Function: EnumAll
     * Desc: �������ýڵ�
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int EnumAll();

    /*
     * Function: HandleTimeout
     * Desc: ��ʱ����
     * In: 
     *      size_t key
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	virtual int HandleTimeout(size_t key) = 0;

protected:
    /*
     * Function: HandleThread
     * Desc: �߳�ҵ����
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    virtual int HandleThread(int& ibreak);

public:
    /*
     * Function: Use
     * Desc: ������״̬
     * In: 
     *      size_t key
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int Use(size_t key);

    /*
     * Function: UnUse
     * Desc: ȡ��ʹ��
     * In: 
     *      size_t key
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int UnUse(size_t key);

    /*
     * Function: WakeUp
     * Desc: ����
     * In: 
     *      size_t key
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
	int WakeUp(unsigned char isrelease = 1);

	int GetFreeNum() { return m_free.GetCount(); };
	int GetNewNum() { return m_use.GetCount(); };
	int GetTrashNum() { return m_trash.GetCount(); };
	int GetTimeNum() { return m_timeHash.GetNodeNum(); };
	int GetUseNum() { return m_useHash.GetNodeNum(); };
};

#endif
