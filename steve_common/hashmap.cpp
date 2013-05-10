
#include "general.h"
#include "hashmap.h"

CHashMap::CHashMap()
{
	pool_ = NULL;
	pool_tail_ = NULL;
	hash_map_ = NULL;
	hash_node_ = NULL;

	m_get_bucket_id = NULL;
	m_cmp_key = NULL;
}

// Clear things up.
CHashMap::~CHashMap()
{
    //NOTHING TO DO
}

int CHashMap::open(char* pool, bool init, int node_total, int bucket_size, int n_chunks, 
				int chunk_size, int key_size, pHashMapGetBucketID get_bucket_id, pHashMapCmpKey cmp_key)
{
	int ret = 0;

	m_get_bucket_id = get_bucket_id;
	m_cmp_key = cmp_key;

	int hash_map_pool_size = get_pool_size(node_total, bucket_size, key_size);
	int head_size = sizeof(THashMap) - sizeof(BC_MEM_HANDLER[1]);
	int bucket_total_size = bucket_size * sizeof(BC_MEM_HANDLER);
	
	pool_ = pool;
	pool_tail_ = pool_ + hash_map_pool_size;
	hash_map_ = (THashMap*)pool_;
	hash_node_ = (THashNode*)(pool_ + head_size + bucket_total_size);

	if (init)
	{
		init_pool_data(node_total, bucket_size, key_size);
	}
	else
	{
		if ((ret = verify_pool_data(node_total, bucket_size, key_size)) != 0) 
		{
		    return ret;
		}
	}

	if ((ret = allocator_.open(pool_tail_, init, n_chunks, chunk_size)) != 0) 
	{
		return ret;
	}

	return 0;    
}

void CHashMap::init_pool_data(int node_total, int bucket_size, int key_size)
{
    hash_map_->node_total_ = node_total;
    hash_map_->bucket_size_ = bucket_size;
    hash_map_->key_size_ = key_size;
    hash_map_->used_node_num_ = 0;
    hash_map_->used_bucket_num_ = 0;

    hash_map_->add_head_ = INVALID_BC_MEM_HANDLER;
    hash_map_->add_tail_ = INVALID_BC_MEM_HANDLER;
    hash_map_->free_list_ = INVALID_BC_MEM_HANDLER;

    int i;
    for(i = 0; i < bucket_size; i++)
    {
        hash_map_->bucket[i] = INVALID_BC_MEM_HANDLER;
    }

    //将所有节点插入到空闲链表中
	THashNode* hash_node;
	int offset;
	for(i = 0; i < node_total; i++)
	{
		offset = i * (sizeof(THashNode) + key_size);
		hash_node = (THashNode*)((char*)hash_node_ + offset);
		init_node(hash_node);
		free_list_insert(hash_node);
	}

    return;
}

int CHashMap::verify_pool_data(int node_total, int bucket_size, int key_size)
{
	if (node_total != hash_map_->node_total_)
	{
        return HASH_MAP_ERROR_BASE;
	}
	if (bucket_size != hash_map_->bucket_size_)
	{
        return HASH_MAP_ERROR_BASE;
	}

	int used_bucket_count = 0;
	for (int i = 0; i < hash_map_->bucket_size_; i++)
	{
		if (hash_map_->bucket[i] != INVALID_BC_MEM_HANDLER)
		{
			used_bucket_count ++;
		}
	}
	if (used_bucket_count != hash_map_->used_bucket_num_)
	{
        return HASH_MAP_ERROR_BASE;
	}

	int free_node_count = 0;
	THashNode* free_node = handler2ptr(hash_map_->free_list_);
	while(free_node)
	{
		free_node_count++;
		free_node = handler2ptr(free_node->node_next_);
	}

	if ((hash_map_->used_node_num_ + free_node_count) != hash_map_->node_total_) 
	{
        return HASH_MAP_ERROR_BASE;
	}

	return 0;
}

THashNode* CHashMap::find_node(void * key)
{
    int bucket_id = m_get_bucket_id(key);
    BC_MEM_HANDLER node_hdr = hash_map_->bucket[bucket_id];
    while(node_hdr != INVALID_BC_MEM_HANDLER)
    {
        THashNode* node = handler2ptr(node_hdr);
        if (m_cmp_key(node->key_, key) == 0) 
        {
        	//将该节点插入到附加链表头部
        	node->add_info_1_ = time(NULL);
            ++node->add_info_2_;
        	insert_add_list_head(node);
            return node;
        }
        node_hdr = node->node_next_;
    }

    return NULL;
}


THashNode* CHashMap::insert_node(void * key, void* new_data, int new_len)
{
    THashNode* node = free_list_remove();
    if (node == NULL) 
    {
        return NULL;                
    }
    
    int new_chunk_num = allocator_.get_chunk_num(new_len);
    BC_MEM_HANDLER head_hdr = allocator_.malloc(new_chunk_num);
    if(head_hdr == INVALID_BC_MEM_HANDLER)
    {
        free_list_insert(node);
        return NULL;
    }
    allocator_.split(head_hdr, new_data, new_len);
    use_node(node, key, new_len, head_hdr);

	//将该节点插入到附加链表头部
	node->add_info_1_ = time(NULL);
	++node->add_info_2_;
	insert_add_list_head(node);
	return node;
}


THashNode* CHashMap::update_node(THashNode* node, void* new_data, int new_len, 
                            char* old_data, int* old_len)
{
	if(old_data != NULL && old_len != NULL)
	{
		//返回旧数据
		if(allocator_.merge(node->chunk_head_, node->chunk_len_, old_data, old_len) != 0)
		{
			return NULL;
		}
	}
	else if(old_len != NULL)
	{
		*old_len = node->chunk_len_;
	}

	int old_chunk_num = allocator_.get_chunk_num(node->chunk_len_);
	int new_chunk_num = allocator_.get_chunk_num(new_len);

	if (old_chunk_num != new_chunk_num)
	{
		//需要重新分配CHUNK. 先FREE再MALLOC.        
		if (new_chunk_num > old_chunk_num)
		{
			if (allocator_.get_free_chunk_num() < (new_chunk_num - old_chunk_num))
			{
				//剩余CHUNK数不足
                return NULL;
			}
		}

		allocator_.free(node->chunk_head_);

		BC_MEM_HANDLER head_hdr = allocator_.malloc(new_chunk_num);   //CHUNK数足够, 不会失败
		allocator_.split(head_hdr, new_data, new_len);

		node->chunk_len_ = new_len;
		node->chunk_head_ = head_hdr;
	}
	else
	{
		allocator_.split(node->chunk_head_, new_data, new_len);
		node->chunk_len_ = new_len;
	}

	return node;
}
    
THashNode* CHashMap::replace_node(void * key, void* new_data, int new_len, char* old_data, int* old_len)
{
	THashNode* node = find_node(key);
	if(node != NULL)
	{
		return update_node(node, new_data, new_len, old_data, old_len);
	}

	return insert_node(key, new_data, new_len);
}

int CHashMap::delete_node(THashNode* node, char* data, int* data_len)
{
	//旧节点存在
	if(data != NULL && data_len != NULL)
	{
    	//返回旧数据
		if(allocator_.merge(node->chunk_head_, node->chunk_len_, data, data_len) != 0)
		{
		    return -1;
		}
	}
	else if(data_len != NULL)
	{
		*data_len = node->chunk_len_;
	}

	delete_from_add_list(node);

	free_node(node);
	free_list_insert(node);
	return 0;
}

void CHashMap::insert_node_list(THashNode* node)
{
    //插入到节点链表头
    int bucket_id = m_get_bucket_id(node->key_);
    BC_MEM_HANDLER node_hdr = ptr2handler(node);

    node->node_next_ = hash_map_->bucket[bucket_id];
    node->node_prev_ = INVALID_BC_MEM_HANDLER;
    hash_map_->bucket[bucket_id] = node_hdr;
    THashNode* next_node = handler2ptr(node->node_next_);
    if(next_node != NULL)
    {
        next_node->node_prev_ = node_hdr;
    }
    
    //stat
    hash_map_->used_node_num_ ++;    
}

void CHashMap::delete_from_node_list(THashNode* node)
{
    BC_MEM_HANDLER next_node_hdr = node->node_next_;
    BC_MEM_HANDLER prev_node_hdr = node->node_prev_;
    
    if(prev_node_hdr != INVALID_BC_MEM_HANDLER)
    {
        THashNode* prev_node = handler2ptr(prev_node_hdr);
        prev_node->node_next_ = node->node_next_;
    }
    if(next_node_hdr != INVALID_BC_MEM_HANDLER)
    {
        THashNode* next_node = handler2ptr(next_node_hdr);
        next_node->node_prev_ = node->node_prev_;
    }
    
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    
    int bucket_id = m_get_bucket_id(node->key_);
    if (node_hdr == hash_map_->bucket[bucket_id]) 
    {
        //当前节点为链表头节点
        hash_map_->bucket[bucket_id] = next_node_hdr;
        
    }
    
    //将前后链表指针清零
    node->node_next_ = INVALID_BC_MEM_HANDLER;
    node->node_prev_ = INVALID_BC_MEM_HANDLER;    

    //stat
    hash_map_->used_node_num_ --;    
}

void CHashMap::free_node(THashNode *node)
{
    //从链表中删除
    delete_from_node_list(node);
    
    //释放 chunk
    allocator_.free(node->chunk_head_);
    
    //stat
    int bucket_list_len = get_bucket_list_len(m_get_bucket_id(node->key_));
    if (bucket_list_len == 0)
    {
        //the bucket change to unused
        hash_map_->used_bucket_num_ --;
    }    

    //reset member
    init_node(node);
}

int CHashMap::purge(int data_len)
{
	//确认数据所需chunk数
	int row_chunk_num = chunks()->get_chunk_num(data_len);
	if(row_chunk_num > 0)
	{
		++row_chunk_num;//当前会为另一个节点预留空间，防止出现purge不够的情况，eg :需2(chunk)+2(chunk)，但只purge了3(chunk)
	}

	//确认是否需要淘汰，如果空闲数已足够，不用再淘汰
	if (chunks()->get_free_chunk_num() >= row_chunk_num &&  //存在空闲段
	    free_node_num()) //存在空闲节点
	{
		return 0;
	}

	//淘汰最早的非脏节点
	THashNode  *node = get_add_list_tail();
	if (node == NULL) //不存在已使用节点
	{
		return -1;
	}

	while (node)
	{
		//上一节点
		THashNode *prev_node = get_add_list_prev(node);

		//是否为非脏节点
		if (node->flag_)
		{
			//脏节点不淘汰
			node = prev_node;
			continue;
		}

		//删除节点
		delete_node(node);

		//确认是否需要淘汰，如果空闲数已足够，不用再淘汰
		if (chunks()->get_free_chunk_num() >= row_chunk_num &&  //存在空闲段
		    free_node_num()) //存在空闲节点
		{
			return 0;
		}

		node = prev_node;
	}

    return -1;
}

void CHashMap::use_node(THashNode *node, void * key, int chunk_len, 
                              BC_MEM_HANDLER chunk_head)
{
    //set member
    memcpy(node->key_, key, get_key_size());
    node->chunk_len_ = chunk_len;
    node->chunk_head_ = chunk_head;
    node->add_info_1_ = 0;
	node->add_info_2_ = 0;

    int bucket_list_len = get_bucket_list_len(m_get_bucket_id(node->key_));
    if (bucket_list_len == 0)
    {
        //the bucket change from unused
        hash_map_->used_bucket_num_ ++;
    }
    
    insert_node_list(node);
    return;
}

int CHashMap::get_bucket_list_len(int bucket_id)
{
	int num = 0;

    BC_MEM_HANDLER node_hdr;
    node_hdr = hash_map_->bucket[bucket_id];

	while (node_hdr != INVALID_BC_MEM_HANDLER)
	{
		num ++;		
		THashNode* node = handler2ptr(node_hdr);
		node_hdr = node->node_next_;
	}

	return num;
}
void CHashMap::insert_add_list_head(THashNode* node)
{
    delete_from_add_list(node);
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    
    //insert node into head of add list
    node->add_next_ = hash_map_->add_head_;
    hash_map_->add_head_ = node_hdr;

    if (hash_map_->add_tail_ == INVALID_BC_MEM_HANDLER)
    {
        hash_map_->add_tail_ = node_hdr;        
    }

    node->add_prev_ = INVALID_BC_MEM_HANDLER;
    THashNode* next_node = handler2ptr(node->add_next_);
    if(next_node != NULL)
    {
        next_node->add_prev_ = node_hdr;
    }
}

void CHashMap::insert_add_list_tail(THashNode* node)
{
    delete_from_add_list(node);
    //reform add list, insert to head
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    
    node->add_prev_ = hash_map_->add_tail_;
    hash_map_->add_tail_ = node_hdr;

    if (hash_map_->add_head_ == INVALID_BC_MEM_HANDLER)
    {
        hash_map_->add_head_ = node_hdr;        
    }    

    node->add_next_ = INVALID_BC_MEM_HANDLER;
    THashNode* prev_node = handler2ptr(node->add_prev_);
    if(prev_node != NULL)
    {
        prev_node->add_next_ = node_hdr;
    }       
}

void CHashMap::delete_from_add_list(THashNode* node)
{
    //link the prev add node and the next add node
    BC_MEM_HANDLER node_hdr = ptr2handler(node);
    BC_MEM_HANDLER next_add_hdr = node->add_next_;
    BC_MEM_HANDLER prev_add_hdr = node->add_prev_;
    
    if ((next_add_hdr == INVALID_BC_MEM_HANDLER) &&
            (prev_add_hdr == INVALID_BC_MEM_HANDLER) &&
            (hash_map_->add_head_ != node_hdr) &&
            (hash_map_->add_tail_ != node_hdr)) 
    {
        //不在链表中
        return ;
    }

    if(prev_add_hdr != INVALID_BC_MEM_HANDLER)
    {
        THashNode* prev_add = handler2ptr(prev_add_hdr);
        prev_add->add_next_ = node->add_next_;
    }
    if(next_add_hdr != INVALID_BC_MEM_HANDLER)
    {
        THashNode* next_add = handler2ptr(next_add_hdr);
        next_add->add_prev_ = node->add_prev_;
    }
    
    if (hash_map_->add_head_ == node_hdr)
    {
        hash_map_->add_head_ =  next_add_hdr;        
    }
    if (hash_map_->add_tail_ == node_hdr) 
    {
        hash_map_->add_tail_ =  prev_add_hdr;
    }
    
    //将前后链表指针清零
    node->add_prev_ = INVALID_BC_MEM_HANDLER;
    node->add_next_ = INVALID_BC_MEM_HANDLER;
}

THashNode* CHashMap::get_add_list_head()
{
    return handler2ptr(hash_map_->add_head_);
}

THashNode* CHashMap::get_add_list_tail()
{
    return handler2ptr(hash_map_->add_tail_);
}

THashNode* CHashMap::get_add_list_prev(THashNode* node)
{
    return handler2ptr(node->add_prev_);
}

THashNode* CHashMap::get_add_list_next(THashNode* node)
{
    return handler2ptr(node->add_next_);
}
