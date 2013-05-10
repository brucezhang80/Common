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
    CSVShm m_shm; /* 共享内存 */
    CHashMap m_hash; /* 对应HASH结构 */
    CSemLock * m_lock; /* 锁 */

    int m_shmkey; /* 共享内存键值 */
    int m_semkey; /* 信号量键值 */
    int m_shmlen; /* 共享内存长度 */

public:
    CMCache();
    ~CMCache();

public:
    /*
     * Function: open
     * Desc: 打开共享内存
     * In: 
     *     int   shm_key       共享内存键值
     *     int   sem_key       信号量键值
     *     int   node_total    总结点数
     *     int   bucket_size   桶数
     *     int   chunk_total_  段数
     *     int   chunk_size    段长
     *     int   key_size      键值长
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Open(int shm_key, int sem_key, int node_total, int bucket_size, int chunk_total_, int chunk_size, 
		int key_size, pHashMapGetBucketID get_bucket_id, pHashMapCmpKey cmp_key);

    /*
     * Function: close
     * Desc: 关闭共享内存
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Close();

    /*
     * Function: Read
     * Desc: 读取数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Read(void * key, char *pBuf, unsigned int iBufLen, unsigned char * flag = NULL);

    /*
     * Function: Write
     * Desc: 写数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Write(void * key, unsigned int iDataLen, char *pBuf);

    /*
     * Function: Delete
     * Desc: 删除数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Delete(void * key);

    /*
     * Function: GetKeyBuf
     * Desc: 删除数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int GetKeyBuf(void * key);

    /*
     * Function: SetKeyBuf
     * Desc: 删除数据
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int SetKeyBuf(void * key);

    /*
     * Function: SetNodeFlag
     * Desc: 设置脏节点标记
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int SetNodeFlag(void * key, unsigned char flag);

    /*
     * Function: GetInfo
     * Desc: 获取共享内存信息
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int GetInfo(THashMap * hash, TChunk * chunk, int *dcnt);

    /*
     * Function: Dump
     * Desc: DUMP共享内存
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Dump();

    /*
     * Function: Recover
     * Desc: 恢复共享内存
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Recover(const char * fname);

    /*
     * Function: Enum
     * Desc: 枚举数据节点
     * In: 
     *     none
     * Out: 
     *     none
     * Return code: 
     *     -1                   出错
     *      0                   成功
     */
    int Enum(void * key, char *pBuf, unsigned int *iBufLen, unsigned char *flag, unsigned char unset = 1);
};

#endif
