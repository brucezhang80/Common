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
	int chunk_len_;                      //CHUNK中的数据长度
	BC_MEM_HANDLER chunk_head_;          //CHUNK 句柄
	BC_MEM_HANDLER node_prev_;           //节点链表前指针
	BC_MEM_HANDLER node_next_;           //节点链表后指针
	BC_MEM_HANDLER add_prev_;            //附加链表前指针
	BC_MEM_HANDLER add_next_;            //附加链表后指针
	time_t add_info_1_;                     //最后访问时间
	int add_info_2_;                     //访问次数
    unsigned char flag_;                 //脏标记

	char key_[0];                        //键值
}THashNode;

typedef struct tagTHashMap
{
    int node_total_;                //节点总数
    int bucket_size_;               //HASH桶的大小
	int key_size_;                  //键值大小
    int used_node_num_;             //使用的节点数
    int used_bucket_num_;           //HASH桶使用数
    BC_MEM_HANDLER add_head_;     //附加链表头指针
    BC_MEM_HANDLER add_tail_;     //附加链表尾指针
    BC_MEM_HANDLER free_list_;    //空间节点链表头指针
    BC_MEM_HANDLER bucket[1];     //HASH桶
}THashMap;

#pragma pack()

typedef int (* pHashMapGetBucketID)(void *);
//键值比较 0-表示相同 非0-表示不一致
typedef int (* pHashMapCmpKey)(void *, void *);

class CHashMap
{
public:
	enum HASH_MAP_ERROR
	{
		HASH_MAP_ERROR_BASE = -1000,    
		HASH_MAP_ERROR_INVALID_PARAM = HASH_MAP_ERROR_BASE -1,    //非法参数
		HASH_MAP_ERROR_NODE_NOT_EXIST = HASH_MAP_ERROR_BASE -2,    //节点不存在
		HASH_MAP_ERROR_NODE_HAVE_EXIST = HASH_MAP_ERROR_BASE -3,    //节点已经存在
		HASH_MAP_ERROR_NO_FREE_NODE = HASH_MAP_ERROR_BASE -4,    //没有空闲节点
	};

public:
	CHashMap();
	~CHashMap();    

	//初始化 HASH_MAP 内存块
	int open(char* pool, bool init, int node_total, int bucket_size, int n_chunks, 
		int chunk_size, int key_size, pHashMapGetBucketID get_bucket_id, pHashMapCmpKey cmp_key);

	// 使用 <key> 进行查询.
	THashNode* find_node(void * key);    
	 //插入节点, 如果旧节点存在, 则返回失败
	THashNode* insert_node(void * key, void* new_data, int new_len);
	//修改节点
	THashNode* update_node(THashNode* node, void* new_data, int new_len, 
									char* old_data = NULL, int* old_len = NULL);
	//insert or update
	THashNode* replace_node(void * key, void* new_data, int new_len, char* old_data = NULL, int* old_len = NULL);
	//删除结点. 同时会将节点从附加链表中清除
	//返回值 = 0 表示成功, < 0 表示失败(如节点不存在,也返回失败)
	int delete_node(THashNode* node, char* data = NULL, int* data_len = NULL);

	int merge_node_data(THashNode* node, char* data, int* data_len);

    int purge(int data_len);

	// 返回当前节点使用数
	int used_node_num() { return hash_map_->used_node_num_; }
	int free_node_num() { return hash_map_->node_total_ - hash_map_->used_node_num_; }
	int get_node_total() { return hash_map_->node_total_; }
	int get_bucket_used() { return hash_map_->used_bucket_num_; }
	int free_bucket_num() {return hash_map_->bucket_size_ - hash_map_->used_bucket_num_; }
	int get_bucket_size() {return hash_map_->bucket_size_;}
	int get_key_size() {return hash_map_->key_size_;}

	CChunkAllocator* chunks() {return &allocator_; };

	// 计算HASH_MAP所需求的内存块尺寸
	static int get_pool_size(int node_total, int bucket_size, int key_size)
	{
		int head_size = sizeof(THashMap) - sizeof(BC_MEM_HANDLER[1]);
		int bucket_total_size = bucket_size * sizeof(BC_MEM_HANDLER);
		int node_total_size = node_total * (sizeof(THashNode) + key_size);

		int pool_size = head_size + bucket_total_size + node_total_size;
		return pool_size;        
	}
	// 取HASH_MAP 和CHUNK的内存块尺寸
	static int get_total_pool_size(int node_total, int bucket_size, int n_chunks, int chunk_size, int key_size)
	{
		return get_pool_size(node_total, bucket_size, key_size) + CChunkAllocator::get_pool_size(n_chunks, chunk_size);
	}

	//transform handler to address
	THashNode *handler2ptr(BC_MEM_HANDLER handler);

	//transform address to handler
	BC_MEM_HANDLER ptr2handler(THashNode* ptr);

	//附加链表操作方法
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

	int get_bucket_list_len(int bucket_id); //取HASH桶的碰撞数

	//将节点插入到空闲链表
	void free_list_insert(THashNode *node);
	//从空闲链表中取节点
	THashNode *free_list_remove();

	//节点链表操作方法
	void insert_node_list(THashNode* node);
	void delete_from_node_list(THashNode* node);

	//初始化节点
	void init_node(THashNode* node);
	//将节点置为空闲模式
	void free_node(THashNode *node);
	//将节点置为使用模式
	void use_node(THashNode *node, void * key, int chunk_len, BC_MEM_HANDLER chunk_head);

	char *pool_;        //内存块起始地址
	char *pool_tail_;   //内存块结束地址

	THashMap* hash_map_;   //内存块中的HASHMAP 结构
	THashNode* hash_node_; //内存块中的HASH节点数组
	CChunkAllocator allocator_; //CHUNK分配器

	//根据索引计算HASH桶值
	pHashMapGetBucketID m_get_bucket_id;

	//键值比较 0-表示相同 非0-表示不一致
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
