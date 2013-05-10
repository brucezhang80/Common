#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include "general.h"
#include "chunk.h"

typedef enum tagENodeFlag
{
	NODE_FLAG_UNCHG = 0x00,
	NODE_FLAG_DIRTY = 0x01,	
}ENodeFlag;

#pragma pack(1)

typedef struct tagTHashNode
{
	int chunk_len_;                      //CHUNK�е����ݳ���
	BC_MEM_HANDLER chunk_head_;          //CHUNK ���
	BC_MEM_HANDLER node_prev_;           //�ڵ�����ǰָ��
	BC_MEM_HANDLER node_next_;           //�ڵ������ָ��
	BC_MEM_HANDLER add_prev_;            //��������ǰָ��
	BC_MEM_HANDLER add_next_;            //���������ָ��
	time_t add_info_1_;                     //������ʱ��
	int add_info_2_;                     //���ʴ���
    unsigned char flag_;                 //����

	char key_[0];                        //��ֵ
}THashNode;

typedef struct tagTHashMap
{
    int node_total_;                //�ڵ�����
    int bucket_size_;               //HASHͰ�Ĵ�С
	int key_size_;                  //��ֵ��С
    int used_node_num_;             //ʹ�õĽڵ���
    int used_bucket_num_;           //HASHͰʹ����
    BC_MEM_HANDLER add_head_;     //��������ͷָ��
    BC_MEM_HANDLER add_tail_;     //��������βָ��
    BC_MEM_HANDLER free_list_;    //�ռ�ڵ�����ͷָ��
    BC_MEM_HANDLER bucket[1];     //HASHͰ
}THashMap;

#pragma pack()

typedef int (* pHashMapGetBucketID)(void *);
//��ֵ�Ƚ� 0-��ʾ��ͬ ��0-��ʾ��һ��
typedef int (* pHashMapCmpKey)(void *, void *);

class CHashMap
{
public:
	enum HASH_MAP_ERROR
	{
		HASH_MAP_ERROR_BASE = -1000,    
		HASH_MAP_ERROR_INVALID_PARAM = HASH_MAP_ERROR_BASE -1,    //�Ƿ�����
		HASH_MAP_ERROR_NODE_NOT_EXIST = HASH_MAP_ERROR_BASE -2,    //�ڵ㲻����
		HASH_MAP_ERROR_NODE_HAVE_EXIST = HASH_MAP_ERROR_BASE -3,    //�ڵ��Ѿ�����
		HASH_MAP_ERROR_NO_FREE_NODE = HASH_MAP_ERROR_BASE -4,    //û�п��нڵ�
	};

public:
	CHashMap();
	~CHashMap();    

	//��ʼ�� HASH_MAP �ڴ��
	int open(char* pool, bool init, int node_total, int bucket_size, int n_chunks, 
		int chunk_size, int key_size, pHashMapGetBucketID get_bucket_id, pHashMapCmpKey cmp_key);

	// ʹ�� <key> ���в�ѯ.
	THashNode* find_node(void * key);    
	 //����ڵ�, ����ɽڵ����, �򷵻�ʧ��
	THashNode* insert_node(void * key, void* new_data, int new_len);
	//�޸Ľڵ�
	THashNode* update_node(THashNode* node, void* new_data, int new_len, 
									char* old_data = NULL, int* old_len = NULL);
	//insert or update
	THashNode* replace_node(void * key, void* new_data, int new_len, char* old_data = NULL, int* old_len = NULL);
	//ɾ�����. ͬʱ�Ὣ�ڵ�Ӹ������������
	//����ֵ = 0 ��ʾ�ɹ�, < 0 ��ʾʧ��(��ڵ㲻����,Ҳ����ʧ��)
	int delete_node(THashNode* node, char* data = NULL, int* data_len = NULL);

	int merge_node_data(THashNode* node, char* data, int* data_len);

    int purge(int data_len);

	// ���ص�ǰ�ڵ�ʹ����
	int used_node_num() { return hash_map_->used_node_num_; }
	int free_node_num() { return hash_map_->node_total_ - hash_map_->used_node_num_; }
	int get_node_total() { return hash_map_->node_total_; }
	int get_bucket_used() { return hash_map_->used_bucket_num_; }
	int free_bucket_num() {return hash_map_->bucket_size_ - hash_map_->used_bucket_num_; }
	int get_bucket_size() {return hash_map_->bucket_size_;}
	int get_key_size() {return hash_map_->key_size_;}

	CChunkAllocator* chunks() {return &allocator_; };

	// ����HASH_MAP��������ڴ��ߴ�
	static int get_pool_size(int node_total, int bucket_size, int key_size)
	{
		int head_size = sizeof(THashMap) - sizeof(BC_MEM_HANDLER[1]);
		int bucket_total_size = bucket_size * sizeof(BC_MEM_HANDLER);
		int node_total_size = node_total * (sizeof(THashNode) + key_size);

		int pool_size = head_size + bucket_total_size + node_total_size;
		return pool_size;        
	}
	// ȡHASH_MAP ��CHUNK���ڴ��ߴ�
	static int get_total_pool_size(int node_total, int bucket_size, int n_chunks, int chunk_size, int key_size)
	{
		return get_pool_size(node_total, bucket_size, key_size) + CChunkAllocator::get_pool_size(n_chunks, chunk_size);
	}

	//transform handler to address
	THashNode *handler2ptr(BC_MEM_HANDLER handler);

	//transform address to handler
	BC_MEM_HANDLER ptr2handler(THashNode* ptr);

	//���������������
	void insert_add_list_head(THashNode* node);
	void insert_add_list_tail(THashNode* node);
	void delete_from_add_list(THashNode* node);
	THashNode* get_add_list_prev(THashNode* node);
	THashNode* get_add_list_next(THashNode* node);
	THashNode* get_add_list_head();
	THashNode* get_add_list_tail();
	////////////////	

	void set_node_flag(THashNode * node, ENodeFlag f){ if (node == NULL) return; node->flag_ = (unsigned char)f; }
	ENodeFlag get_node_flag(THashNode *node){ if (node == NULL) return NODE_FLAG_UNCHG; return (ENodeFlag)node->flag_; }
	THashNode* get_bucket_list_head(unsigned bucket_id);
	THashNode* get_bucket_list_prev(THashNode* node);
	THashNode* get_bucket_list_next(THashNode* node);

protected:
	void init_pool_data(int node_total, int bucket_size, int key_size);
	int verify_pool_data(int node_total, int bucket_size, int key_size);

	int get_bucket_list_len(int bucket_id); //ȡHASHͰ����ײ��

	//���ڵ���뵽��������
	void free_list_insert(THashNode *node);
	//�ӿ���������ȡ�ڵ�
	THashNode *free_list_remove();

	//�ڵ������������
	void insert_node_list(THashNode* node);
	void delete_from_node_list(THashNode* node);

	//��ʼ���ڵ�
	void init_node(THashNode* node);
	//���ڵ���Ϊ����ģʽ
	void free_node(THashNode *node);
	//���ڵ���Ϊʹ��ģʽ
	void use_node(THashNode *node, void * key, int chunk_len, BC_MEM_HANDLER chunk_head);

	char *pool_;        //�ڴ����ʼ��ַ
	char *pool_tail_;   //�ڴ�������ַ

	THashMap* hash_map_;   //�ڴ���е�HASHMAP �ṹ
	THashNode* hash_node_; //�ڴ���е�HASH�ڵ�����
	CChunkAllocator allocator_; //CHUNK������

	//������������HASHͰֵ
	pHashMapGetBucketID m_get_bucket_id;

	//��ֵ�Ƚ� 0-��ʾ��ͬ ��0-��ʾ��һ��
	pHashMapCmpKey m_cmp_key;
};

inline THashNode* CHashMap::handler2ptr(BC_MEM_HANDLER handler)
{
    if (handler == INVALID_BC_MEM_HANDLER)
    {
        return NULL;
    }
    return (THashNode*)(pool_ + handler);
}
    
inline BC_MEM_HANDLER CHashMap::ptr2handler(THashNode* ptr)
{
    char *tmp_ptr = (char *)ptr;
    if((tmp_ptr < pool_) || (tmp_ptr >= pool_tail_))
    {
        return INVALID_BC_MEM_HANDLER;
    }
    return (BC_MEM_HANDLER)(tmp_ptr - pool_);    
}

inline void CHashMap::free_list_insert(THashNode *node)
{
    //insert to free list's head
    node->node_next_ = hash_map_->free_list_;
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    hash_map_->free_list_ = node_hdr;
    return;
}

inline THashNode* CHashMap::free_list_remove()
{
    //get head node from free list
    if(hash_map_->free_list_ == INVALID_BC_MEM_HANDLER)
    {
        return NULL;
    }
	
    THashNode* head_node = handler2ptr(hash_map_->free_list_);
    hash_map_->free_list_ = head_node->node_next_;
    head_node->node_next_ = INVALID_BC_MEM_HANDLER;	
    return head_node;
}

inline void CHashMap::init_node(THashNode* node)
{
	node->chunk_len_ = 0;
	node->add_info_1_ = 0;
	node->add_info_2_ = 0;
	node->flag_ = NODE_FLAG_UNCHG;

	node->chunk_head_ = INVALID_BC_MEM_HANDLER;
	node->node_next_= INVALID_BC_MEM_HANDLER;
	node->node_prev_= INVALID_BC_MEM_HANDLER;
	node->add_next_= INVALID_BC_MEM_HANDLER;
	node->add_prev_= INVALID_BC_MEM_HANDLER;

	memset(node->key_, 0, get_key_size());

    return;
}

inline THashNode*  CHashMap::get_bucket_list_head(unsigned bucket_id)
{
	if (bucket_id >= (unsigned)hash_map_->bucket_size_)
    {
        return NULL;
    }
	
	BC_MEM_HANDLER node_hdr = hash_map_->bucket[bucket_id];
	return node_hdr != INVALID_BC_MEM_HANDLER ? handler2ptr(node_hdr) : NULL; 
}

inline THashNode*  CHashMap::get_bucket_list_prev(THashNode* node)
{
	if (node == NULL)
	{
        return NULL;
	}

	return node->node_prev_!= INVALID_BC_MEM_HANDLER ? handler2ptr( node->node_prev_) : NULL;
}
inline THashNode*  CHashMap::get_bucket_list_next(THashNode* node)
{
	if (node == NULL)
	{
        return NULL;
	}

	return node->node_next_!= INVALID_BC_MEM_HANDLER ? handler2ptr( node->node_next_) : NULL;
}

inline int CHashMap::merge_node_data(THashNode* node, char* data, int* data_len)
{
    return allocator_.merge(node->chunk_head_, node->chunk_len_, data, data_len);
}

#endif
