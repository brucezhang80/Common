/*
 * @File: realtime.cpp
 * @Desc: impliment file of realtime,���ڳ�ʱ����
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
 * Desc: �󶨶���
 * In: 
 *      unsigned int  key   ��ֵ
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CRealTimeNode::Use(size_t key)
{
	m_key = key;
	m_use = time(NULL);

	return 0;
}

CRealTime::CRealTime(int num, int timeout /* = -1 */) : CLThread("realtime", 1, 1000), /* ���1�� */
m_free(num), m_use(num, 0), m_trash(num, 0), m_timeHash(num), m_useHash(num), m_timeout(timeout)
{
	/* ���ж����з���ڵ� */
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
 * Desc: ������������״̬
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CRealTime::Fill()
{
	/* ��ʹ�ö����е�����ȡ��������hash�� */
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
 * Desc: �������δ�ýڵ�
 * In: 
 *      size_t key
 * Out: 
 *      none
 * Return code: 
 *      0  -   �ɹ�
 *      -1 -   ʧ��
 */
int CRealTime::Clean()
{
	while(1)
	{
		CRealTimeNode * node = m_trash.get();
		if (node == NULL)
			break;

		/* �ӳ�ʱ������ɾ�� */
		m_timeHash.Remove((size_t)node);

		/* ������ж��� */
		m_free.release(node);
	}

	return 0;
}

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
int CRealTime::EnumAll()
{
	/* ö�ٿ�ʼ */
	m_timeHash.EnumBegin();

	time_t t_now = time(NULL);
	while (1)
	{
		/* ö�ٽڵ� */
		CRealTimeNode * node = (CRealTimeNode *)m_timeHash.EnumNext();
		if (node == NULL)
			break;

		/* ����Ƿ�ʱ�����г�ʱ���� */
		if ((t_now - node->m_use) >= m_timeout)
		{
			HandleTimeout(node->m_key);

			/* ����ɾ������ */
			m_trash.release(node);
		}
	}

	/* ö�ٽ��� */
	m_timeHash.EnumEnd();

	return 0;
}

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
int CRealTime::HandleThread(int& ibreak)
{
	/* �������� */
	Fill();

	/* ����ɾ�� */
	Clean();

	if (m_timeout > 0)
	{
		/* ö�ٵ�ǰ��������� */
		EnumAll();
	}

	return 0;
}

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
int CRealTime::Use(size_t key)
{
	/* ״̬HASH����ȡ�ڵ� */
	CRealTimeNode * node = (CRealTimeNode *)m_useHash.Get(key);
	if (node != NULL)
	{
		/* ����ʱ�� */
		node->Use(key);
		return 0;
	}

	/* ȡ���нڵ� */
	node = m_free.get();
	if (node == NULL)
		return -1;

	/* ����ʱ�� */
	node->Use(key);

	/* ����״̬HASH */
	m_useHash.Add(key, node);

	/* ������������ */
	m_use.release(node);

	return 0;
}

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
int CRealTime::UnUse(size_t key)
{
	/* ״̬HASH����ɾ���ڵ� */
	CRealTimeNode * node = (CRealTimeNode *)m_useHash.Remove(key);
	if (node == NULL)
		return -1;

	/* ����ɾ������ */
	m_trash.release(node);

	return 0;
}

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
int CRealTime::WakeUp(unsigned char isrelease /* = 1 */)
{
	m_free.WakeUpAll(isrelease);

	return 0;
}
