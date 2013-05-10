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
 * Desc: �򿪹����ڴ�
 * In: 
 * int   shm_key       �����ڴ��ֵ
 * int   sem_key       �ź�����ֵ
 * int   node_total    �ܽ����
 * int   bucket_size   Ͱ��
 * int   n_chunks      ����
 * int   chunk_size    �γ�
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMCache::Open(int shm_key, int sem_key, int node_total, int bucket_size, int chunk_total_, int chunk_size, 
	int key_size, pHashMapGetBucketID get_bucket_id, pHashMapCmpKey cmp_key)
{
	unsigned int pool_size = 0;
	int ret = 0;

    /* ������ */
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

    /* ������㳤�� */
	pool_size = m_hash.get_total_pool_size(node_total, bucket_size, chunk_total_, chunk_size, key_size);

    /* �򿪹����ڴ� */
	ret = m_shm.force_open_and_attach(shm_key, pool_size);
	if(ret < 0)
	{
		return -1;
	}

    /* ��hash�ṹ�� */
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
 * Desc: �رչ����ڴ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: ��ȡ����
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMCache::Read(void * key, char *pBuf, unsigned int iBufLen, unsigned char * flag /* = NULL */)
{
    if (pBuf == NULL)
    {
        return -1;
    }

    CAutoLock l(m_lock);

    /* �ҽڵ� */
	THashNode *node = NULL;
	node = m_hash.find_node(key);
	if (node == NULL)
    {
        return 0;
    }

    /* ��ȡ���� */
    int ret = m_hash.merge_node_data(node, pBuf, (int *)&iBufLen);
    if (ret < 0)
    {
        return -1;
    }

    /* ���ؼ�ֵ */
    memcpy(key, &node->key_, m_hash.get_key_size());

    /* ��ȡ�ڵ�״̬ */
    if (flag)
    {
        *flag = node->flag_;
    }

    return iBufLen;
}

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
int CMCache::Write(void * key, unsigned int iDataLen, char *pBuf)
{
	THashNode *node = NULL;

    CAutoLock l(m_lock);

    /* ɾ�����ڶ�Ӧ��ֵ�ڵ� */
	node = m_hash.find_node(key);
	if (node)
    {
        //ɾ���ڵ�
        m_hash.delete_node(node);
    }	

    /* ȷ���Ƿ���Ҫ��̭�ڵ� */
    int ret = m_hash.purge(iDataLen);
    if (ret < 0)
    {
        return -1;
    }

    /* ����ڵ� */
	node = m_hash.insert_node(key, pBuf, iDataLen);
    if (node == NULL)
    {
        return -1;
    }

    /* ���ؼ�ֵ */
    memcpy(key, &node->key_, m_hash.get_key_size());

    /* ����Ϊ�� */
    m_hash.set_node_flag(node, NODE_FLAG_DIRTY);

    return 0;
}

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
int CMCache::Delete(void * key)
{
    CAutoLock l(m_lock);

    /* �ҽڵ� */
	THashNode *node = NULL;
	node = m_hash.find_node(key);
	if (node == NULL)
    {
        return -1;
    }
	
    /* ���ؼ�ֵ */
    memcpy(key, &node->key_, m_hash.get_key_size());

    //ɾ���ڵ�
	m_hash.delete_node(node);

    return 0;
}

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
int CMCache::GetKeyBuf(void * key)
{
    CAutoLock l(m_lock);

    /* �ҽڵ� */
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
 * Desc: ɾ������
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMCache::SetKeyBuf(void * key)
{
    CAutoLock l(m_lock);

    /* �ҽڵ� */
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
 * Desc: ������ڵ���
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMCache::SetNodeFlag(void * key, unsigned char flag)
{
    CAutoLock l(m_lock);

    /* �ҽڵ� */
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
 * Desc: ��ȡ�����ڴ���Ϣ
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
 */
int CMCache::GetInfo(THashMap * hash, TChunk * chunk, int *dcnt)
{
    if (hash == NULL || chunk == NULL || dcnt == NULL)
    {
        return -1;
    }

    CAutoLock l(m_lock);

    /* ��ȡHASH��Ϣ */
    memcpy(hash, (char *)m_shm.get_segment_ptr(), sizeof(THashMap));

    /* ��ȡCHUNK��Ϣ */
    memcpy(chunk, m_hash.chunks()->get_chunk(), sizeof(TChunk));

    /* ͳ����ڵ���Ŀ */
    int cnt = 0;

	THashNode  *node = m_hash.get_add_list_tail();
    while (node)
    {
        if (node->flag_)
        {
            /* ��ڵ� */
            cnt++;
        }

        node = m_hash.get_add_list_prev(node);
    }

    *dcnt = cnt;

    return 0;
}

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
 * Desc: �ָ������ڴ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
 * Desc: ö�����ݽڵ�
 * In: 
 *     none
 * Out: 
 *     none
 * Return code: 
 *     -1                   ����
 *      0                   �ɹ�
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
        /* ö�ٿ�ʼ,���� */
        m_lock->Lock();

     	node = m_hash.get_add_list_tail();
        if (node == NULL)
        {
            status = 2; /* ö����һ״̬Ϊ����״̬ */
            *iBufLen = 0;
            return 0;
        }

        /* ��ȡ���� */
        int ret = m_hash.merge_node_data(node, pBuf, (int *)iBufLen);
        if (ret < 0)
        {
            goto ENUM_ERR;
        }

        /* ���ؼ�ֵ */
        memcpy(key, &node->key_, m_hash.get_key_size());

        /* ��ȡ�ڵ�״̬ */
        *flag = node->flag_;

        status = 1; /* ö����һ״̬Ϊ����״̬ */
    }
    else if (status == 1)
    {
        if (node == NULL)
        {
            status = 2; /* ö����һ״̬Ϊ����״̬ */
            *iBufLen = 0;
            return 0;
        }

        if (!unset)
        {
            /* ȥ����ڵ��� */
            m_hash.set_node_flag(node, (ENodeFlag)NODE_FLAG_UNCHG);
        }

        /*ȡ��һ�ڵ� */
        node = m_hash.get_add_list_prev(node);
        if (node == NULL)
        {
            status = 2; /* ö����һ״̬Ϊ����״̬ */
            *iBufLen = 0;
            return 0;
        }

        /* ��ȡ���� */
        int ret = m_hash.merge_node_data(node, pBuf, (int *)iBufLen);
        if (ret < 0)
        {
            goto ENUM_ERR;
        }

        /* ���ؼ�ֵ */
        memcpy(key, &node->key_, m_hash.get_key_size());

        /* ��ȡ�ڵ�״̬ */
        *flag = node->flag_;

        status = 1; /* ö����һ״̬Ϊ����״̬ */
    }
    else if (status == 2)
    {
        /* ��Ϊ��ʼ״̬ */
        status = 0;
        /* ���� */
        m_lock->UnLock();

        return 1;
    }

    return 0;

ENUM_ERR:
    if (status)
    {
        /* �趨��ʼ״̬ */
        status = 0;
        /* ���� */
        m_lock->UnLock();
    }

    return -1;
}
