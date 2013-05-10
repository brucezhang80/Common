/*
 * @File: mcache.h
 * @Desc: head file of Mcache
 * @Author: stevetang
 * @History:
 *      2007-08-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _MCACHE_H_
#define _MCACHE_H_

#include "hashmap.h"
#include "shm.h"
#include "lock.h"

class CMCache
{
    CSVShm m_shm; /* �����ڴ� */
    CHashMap m_hash; /* ��ӦHASH�ṹ */
    CSemLock * m_lock; /* �� */

    int m_shmkey; /* �����ڴ��ֵ */
    int m_semkey; /* �ź�����ֵ */
    int m_shmlen; /* �����ڴ泤�� */

public:
    CMCache();
    ~CMCache();

public:
    /*
     * Function: open
     * Desc: �򿪹����ڴ�
     * In: 
     *     int   shm_key       �����ڴ��ֵ
     *     int   sem_key       �ź�����ֵ
     *     int   node_total    �ܽ����
     *     int   bucket_size   Ͱ��
     *     int   chunk_total_  ����
     *     int   chunk_size    �γ�
     *     int   key_size      ��ֵ��
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Open(int shm_key, int sem_key, int node_total, int bucket_size, int chunk_total_, int chunk_size, 
		int key_size, pHashMapGetBucketID get_bucket_id, pHashMapCmpKey cmp_key);

    /*
     * Function: close
     * Desc: �رչ����ڴ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Close();

    /*
     * Function: Read
     * Desc: ��ȡ����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Read(void * key, char *pBuf, unsigned int iBufLen, unsigned char * flag = NULL);

    /*
     * Function: Write
     * Desc: д����
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Write(void * key, unsigned int iDataLen, char *pBuf);

    /*
     * Function: Delete
     * Desc: ɾ������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Delete(void * key);

    /*
     * Function: GetKeyBuf
     * Desc: ɾ������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int GetKeyBuf(void * key);

    /*
     * Function: SetKeyBuf
     * Desc: ɾ������
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int SetKeyBuf(void * key);

    /*
     * Function: SetNodeFlag
     * Desc: ������ڵ���
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int SetNodeFlag(void * key, unsigned char flag);

    /*
     * Function: GetInfo
     * Desc: ��ȡ�����ڴ���Ϣ
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int GetInfo(THashMap * hash, TChunk * chunk, int *dcnt);

    /*
     * Function: Dump
     * Desc: DUMP�����ڴ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Dump();

    /*
     * Function: Recover
     * Desc: �ָ������ڴ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Recover(const char * fname);

    /*
     * Function: Enum
     * Desc: ö�����ݽڵ�
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   ����
     *      0                   �ɹ�
     */
    int Enum(void * key, char *pBuf, unsigned int *iBufLen, unsigned char *flag, unsigned char unset = 1);
};

#endif
