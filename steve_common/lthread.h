/*
 * @File: lthread.h
 * @Desc: head file of thread interface
 * @Author: stevetang
 * @History:
 *      2007-11-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _LTHREAD_H
#define _LTHREAD_H

#include "event.h"

class CLThread
{
	char m_name[128]; /* �߳����� */

    unsigned char m_running; /* �Ƿ��������� */
    pthread_t m_hThread; /* �߳̾�� */

    unsigned char m_hangup; /* �Ƿ�Ϊ�ȴ��������߳� */
    CEvent m_hEvent; /* �¼� */

	int m_timeout; /* ��ʱʱ��,���� */

public:
    CLThread(const char * name = "", unsigned char hangup = 0, int timeout = -1);
    virtual ~CLThread();

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
    virtual int HandleThread(int& ibreak) = 0;

    /*
     * Function: OnStop
     * Desc: ֹͣ�¼�
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    virtual int OnStop();

private:
    /*
     * Function: ThreadDispatch
     * Desc: �߳�ҵ������
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    int ThreadDispatch();

    /*
     * Function: ThreadEntry
     * Desc: �̴߳�����
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    static void * ThreadEntry(void * argv);

public:
    /*
     * Function: Start
     * Desc: �����߳�
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    int Start();

    /*
     * Function: Stop
     * Desc: ֹͣ�߳�
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    int Stop();

    /*
     * Function: IsRunning
     * Desc: �Ƿ��˳�
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      true  -   �ɹ�
     *      false -   ʧ��
     */
    bool IsRunning();

    /*
     * Function: WakeUp
     * Desc: ����
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    int WakeUp();

    /*
     * Function: GetHandle
     * Desc: ��ȡ�߳̾��
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      ��NULL  -   �ɹ�
     *      NULL    -   ʧ��
     */
    pthread_t GetHandle() { return m_hThread; };

    /*
     * Function: GetName
     * Desc: ��ȡ�߳�����
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      ��NULL  -   �ɹ�
     *      NULL    -   ʧ��
     */
    const char * GetName() { return (const char *)m_name; };
};

#endif
