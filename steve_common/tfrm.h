/*
 * @File: tfrm.h
 * @Desc: head file of Process Frame
 * @Author: stevetang
 * @History:
 *      2007-11-23   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _TFRM_H
#define _TFRM_H

#include "event.h"
#include "lthread.h"

class CTFrm : public CLThread
{
    CEvent m_event; /* �˳��¼� */

    char m_fname[512]; /* �����ļ� */
    int m_fd; /* ���̺��ļ���� */

    char m_name[512]; /* SERVER���� */
    int m_daemon; /* �Ƿ��̨���� */

    unsigned char m_islock; /* �Ƿ񵥽��� */

public:
    CTFrm(const char * name, const char * conf, unsigned char islock = 1);
    ~CTFrm();

private:
    /*
     * Function: Usage
     * Desc: �÷�˵��
     * In: 
     *     const char *   name   ������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int Usage(const char * name);

    /*
     * Function: ParseCmd
     * Desc: ������������
     * In: 
     *      int    argc    ������Ŀ
     *      char * argv[]  �������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int ParseCmd(int argc, char * argv[]);

    /*
     * Function: DispatchCmd
     * Desc: ��������
     * In: 
     *      int    argc    ������Ŀ
     *      char * argv[]  �������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int DispatchCmd(int argc, char * argv[]);

    /*
     * Function: DaemonInit
     * Desc: ����Ϊ��̨����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     none
     */
    void DaemonInit();

    /*
     * Function: IsExit
     * Desc: �����Ƿ����˳�
     * In: 
     *     const char *   fname   ������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int IsExit(const char * fname);

    /*
     * Function: CmdStop
     * Desc: ��������
     * In: 
     *     const char *   fname   ������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int CmdStop(const char * fname);

    /*
     * Function: CmdReload
     * Desc:���¼�������
     * In: 
     *     const char *   fname   ������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int CmdReload(const char * fname);

    /*
     * Function: LockServ
     * Desc: �������Ƿ�������
     * In: 
     *     const char *   fname   ������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int LockServ(const char * fname);

    /*
     * Function: UnLockServ
     * Desc: �ͷ�ϵͳ��Դ
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    int UnLockServ();

    /*
     * Function: HandleUSR1
     * Desc: SIGUSR1�źŴ�����
     * In: 
     *     int    sig    �ź�
     * Out: 
     *     none
     * Return code:
     *     none
     */
    static void HandleUSR1(int sig);

    /*
     * Function: HandleUSR2
     * Desc: SIGUSR2�źŴ�����
     * In: 
     *     int    sig    �ź�
     * Out: 
     *     none
     * Return code:
     *     none
     */
    static void HandleUSR2(int sig);

protected:
    /*
     * Function: SetProcName
     * Desc: ���ý�������
     * In: 
     *      int    argc    ������Ŀ
     *      char * argv[]  �������
     * Out: 
     *     none
     * Return code:
     *      -1         ����
     *       0         �ɹ�
     */
    virtual int SetProcName(int argc, char * argv[]);

    /*
     * Function: InitConf
     * Desc: ��ʼ�����ò���
     * In: 
     *     const char *   conf  �����ļ���
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int InitConf(const char * conf);

    /*
     * Function: InitFrame
     * Desc: ϵͳ��ʼ��
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int InitFrame();

    /*
     * Function: ReleaseFrame
     * Desc: ϵͳ��Դ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int ReleaseFrame();

    /*
     * Function: HandleThread
     * Desc: �߳�ҵ����
     * In: 
     *      int    ibreak   �жϷ����־
     * Out: 
     *      int    ibreak   �жϷ����־
     * Return code: 
     *      0  -   �ɹ�
     *      -1 -   ʧ��
     */
    virtual int HandleThread(int& ibreak);

public:
    /*
     * Function: Start
     * Desc: �������
     * In: 
     *      int    argc    ������Ŀ
     *      char * argv[]  �������
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Start(int argc, char *argv[]);

    /*
     * Function: Stop
     * Desc: �˳����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Stop();

    /*
     * Function: ReloadConf
     * Desc: ���¼��������ļ�
     * In: 
     *     const char *   conf  �����ļ���
     * Out: 
     *     none
     * Return code:
     *     -1                   ����
     *      0                   �ɹ�
     */
    virtual int ReloadConf(const char *conf);
};

#endif
