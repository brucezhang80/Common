/*
 * @File: mcache.cpp
 * @Desc: implement file of Mcache
 * @Author: stevetang
 * @History:
 *      2007-08-09   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "mcache.h"

CMCache::CMCache() : m_lock(NULL), m_shmkey(0), m_semkey(0), m_shmlen(0)
{
}

CMCache::~CMCache()
{
    Close();
}

/*
 * Function: open
 * Desc: 打开共享内存
 * In: 
 * int   shm_key       共享内存键值
 * int   sem_key       信号量键值
 * int   node_total    总结点数
 * int   bucket_size   桶数
 * int   n_chunks      段数
 * int   chunk_size    段长
 * Out: 
 *     none
 * Return code: 
 *     -1                   出错
 *      0                   成功
 */
int CMCache::Open(int shm_key, int sem_key, int node_total, int bucket_size, int chunk_total_, int chunk_size, 
	int key_size, pHashMapGetBucketID get_bucket_id, pHashMapCmpKey cmp_key)
{
	unsigned int pool_size = 0;
	int ret = 0;

    /* 创建锁 */
    if (m_lock)
    {
        delete m_lock;
        m_lock = NULL;
    }

    m_lock = new CSemLock(sem_key);
    if (m_lock == NULL)
    {
        return -3;
    }

    CAutoLock l(m_lock);

    /* 所需计算长度 */
	pool_size = m_hash.get_total_pool_size(node_total, bucket_size, chunk_total_, chunk_size, key_size);

    /* 打开共享内存 */
	ret = m_shm.force_open_and_attach(shm_key, pool_size);
	if(ret < 0)
	{
		return -1;
	}

    /* 跟hash结构绑定 */
	ret = m_hash.open((char *)m_shm.get_segment_ptr(), 0, node_total, bucket_size, chunk_total_, chunk_size, 
		key_size, get_bucket_id, cmp_key);
	if(ret < 0)
	{
		return -2;
	}

    m_shmkey = shm_key;
    m_semkey = sem_key;

    m_shmlen = pool_size;

    return 0;
}

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
int CMCache::Close()
{
    if (m_lock)
    {
        delete m_lock;
        m_lock = NULL;
    }

    return 0;
}

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
int CMCache::Read(void * key, char *pBuf, unsigned int iBufLen, unsigned char * flag /* = NULL */)
{
    if (pBuf == NULL)
    {
        return -1;
    }

    CAutoLock l(m_lock);

    /* 找节点 */
	THashNode *node = NULL;
	node = m_hash.find_node(key);
	if (node == NULL)
    {
        return 0;
    }

    /* 获取数据 */
    int ret = m_hash.merge_node_data(node, pBuf, (int *)&iBufLen);
    if (ret < 0)
    {
        return -1;
    }

    /* 返回键值 */
    memcpy(key, &node->key_, m_hash.get_key_size());

    /* 获取节点状态 */
    if (flag)
    {
        *flag = node->flag_;
    }

    return iBufLen;
}

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
int CMCache::Write(void * key, unsigned int iDataLen, char *pBuf)
{
	THashNode *node = NULL;

    CAutoLock l(m_lock);

    /* 删除存在对应键值节点 */
	node = m_hash.find_node(key);
	if (node)
    {
        //删除节点
        m_hash.delete_node(node);
    }	

    /* 确认是否需要淘汰节点 */
    int ret = m_hash.purge(iDataLen);
    if (ret < 0)
    {
        return -1;
    }

    /* 插入节点 */
	node = m_hash.insert_node(key, pBuf, iDataLen);
    if (node == NULL)
    {
        return -1;
    }

    /* 返回键值 */
    memcpy(key, &node->key_, m_hash.get_key_size());

    /* 设置为脏 */
    m_hash.set_node_flag(node, NODE_FLAG_DIRTY);

    return 0;
}

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
int CMCache::Delete(void * key)
{
    CAutoLock l(m_lock);

    /* 找节点 */
	THashNode *node = NULL;
	node = m_hash.find_node(key);
	if (node == NULL)
    {
        return -1;
    }
	
    /* 返回键值 */
    memcpy(key, &node->key_, m_hash.get_key_size());

    //删除节点
	m_hash.delete_node(node);

    return 0;
}

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
int CMCache::GetKeyBuf(void * key)
{
    CAutoLock l(m_lock);

    /* 找节点 */
	THashNode *node = NULL;
	node = m_hash.find_node(key);
	if (node == NULL)
    {
        return -1;
    }

    memcpy(key, &node->key_, m_hash.get_key_size());

    return 0;
}

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
int CMCache::SetKeyBuf(void * key)
{
    CAutoLock l(m_lock);

    /* 找节点 */
	THashNode *node = NULL;
	node = m_hash.find_node(key);
	if (node == NULL)
    {
        return -1;
    }

    memcpy(&node->key_, key, m_hash.get_key_size());

    return 0;
}

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
int CMCache::SetNodeFlag(void * key, unsigned char flag)
{
    CAutoLock l(m_lock);

    /* 找节点 */
	THashNode *node = NULL;
	node = m_hash.find_node(key);
	if (node == NULL)
    {
        return -1;
    }

    m_hash.set_node_flag(node, (ENodeFlag)flag);

    return 0;
}

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
int CMCache::GetInfo(THashMap * hash, TChunk * chunk, int *dcnt)
{
    if (hash == NULL || chunk == NULL || dcnt == NULL)
    {
        return -1;
    }

    CAutoLock l(m_lock);

    /* 获取HASH信息 */
    memcpy(hash, (char *)m_shm.get_segment_ptr(), sizeof(THashMap));

    /* 获取CHUNK信息 */
    memcpy(chunk, m_hash.chunks()->get_chunk(), sizeof(TChunk));

    /* 统计脏节点数目 */
    int cnt = 0;

	THashNode  *node = m_hash.get_add_list_tail();
    while (node)
    {
        if (node->flag_)
        {
            /* 脏节点 */
            cnt++;
        }

        node = m_hash.get_add_list_prev(node);
    }

    *dcnt = cnt;

    return 0;
}

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
int CMCache::Dump()
{
    char name[100];
    time_t t_now = time(NULL);
    struct tm *t = localtime(&t_now);

    memset(name, 0, sizeof(name));
    sprintf(name, "%d.%04d%02d%02d%02d%02d%02d", m_shmkey, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

    FILE * fp = fopen(name, "wb");
    if (fp == NULL)
    {
        return -1;
    }

    CAutoLock l(m_lock);

    fwrite((char *)m_shm.get_segment_ptr(), m_shm.get_segment_size(), 1, fp);

    fclose(fp);

    return 0;
}

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
int CMCache::Recover(const char * fname)
{
    FILE * fp = fopen(fname, "rb");
    if (fp == NULL)
    {
        return -1;
    }

    CAutoLock l(m_lock);

    char *ptr = (char *)m_shm.get_segment_ptr();

    while (!feof(fp))
    {
        char buffer[100 * 1024];

        memset(buffer, 0, sizeof(buffer));
        int ret = fread(&buffer, 1, sizeof(buffer), fp);

        memcpy(ptr, buffer, ret);
        
        ptr += ret;
    }

    fclose(fp);

    return 0;
}

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
int CMCache::Enum(void * key, char *pBuf, unsigned int *iBufLen, unsigned char *flag, unsigned char unset /* = 1 */)
{
    static int status = 0;
    static THashNode * node = NULL;

    if (key == NULL || pBuf == NULL || iBufLen == NULL || flag == NULL)
    {
        goto ENUM_ERR;
    }

    if (status == 0)
    {
        /* 枚举开始,上锁 */
        m_lock->Lock();

     	node = m_hash.get_add_list_tail();
        if (node == NULL)
        {
            status = 2; /* 枚举下一状态为结束状态 */
            *iBufLen = 0;
            return 0;
        }

        /* 获取数据 */
        int ret = m_hash.merge_node_data(node, pBuf, (int *)iBufLen);
        if (ret < 0)
        {
            goto ENUM_ERR;
        }

        /* 返回键值 */
        memcpy(key, &node->key_, m_hash.get_key_size());

        /* 获取节点状态 */
        *flag = node->flag_;

        status = 1; /* 枚举下一状态为继续状态 */
    }
    else if (status == 1)
    {
        if (node == NULL)
        {
            status = 2; /* 枚举下一状态为结束状态 */
            *iBufLen = 0;
            return 0;
        }

        if (!unset)
        {
            /* 去掉脏节点标记 */
            m_hash.set_node_flag(node, (ENodeFlag)NODE_FLAG_UNCHG);
        }

        /*取下一节点 */
        node = m_hash.get_add_list_prev(node);
        if (node == NULL)
        {
            status = 2; /* 枚举下一状态为结束状态 */
            *iBufLen = 0;
            return 0;
        }

        /* 获取数据 */
        int ret = m_hash.merge_node_data(node, pBuf, (int *)iBufLen);
        if (ret < 0)
        {
            goto ENUM_ERR;
        }

        /* 返回键值 */
        memcpy(key, &node->key_, m_hash.get_key_size());

        /* 获取节点状态 */
        *flag = node->flag_;

        status = 1; /* 枚举下一状态为继续状态 */
    }
    else if (status == 2)
    {
        /* 置为初始状态 */
        status = 0;
        /* 解锁 */
        m_lock->UnLock();

        return 1;
    }

    return 0;

ENUM_ERR:
    if (status)
    {
        /* 设定初始状态 */
        status = 0;
        /* 解锁 */
        m_lock->UnLock();
    }

    return -1;
}
