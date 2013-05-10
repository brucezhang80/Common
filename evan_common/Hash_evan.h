/*
 * index.h
 *
 *  Created on: 2010-4-22
 *      Author: evantang
 */

#ifndef HASH_H_
#define HASH_H_

#include "HashTbl.h"
#include <list>

using namespace std;

#if 0
template <class T1, class T2>

class CHash
{
public:
	virtual bool Load(const char* filename) = 0;
	virtual bool Dump(const char* dirname) = 0;

	virtual void Clear()
	{
		index.clear();
	}

	virtual bool Get(const T1& a)
	{
		map<T1, T2>::iterator iter = index.find(a);
		if (iter != index.end())
			return true;
		else
			return false;
	}

	virtual void Insert(const T1& a, const T2& b)
	{
		index.insert(map<T1, T2> :: value_type(a, b));
		return;
	}

	virtual void Replace(const T1& a, const T2& b)
	{

	}

private:
	map <T1, T2> index;
};
#endif

/*T1,T2��Ҫ���Լ��Ŀ������캯�����Լ�==,=, <��������أ��Լ�Ĭ�Ϲ��캯��*/
template <typename T1, typename T2>

struct DataNode
{
	HASH_NODE head;
	T1 key;
	T2 value;
	time_t t_now;

	/*T1��T2ģ��=���������*/
	DataNode(const T1& a, const T2& b)
	{
		key = a;
		value = b;
		t_now = time(NULL);
	}

	/*T1��T2ģ��=��������أ���ʱ�����*/
	DataNode(const T1& a, const T2& b, const time_t _time)
	{
		key = a;
		value = b;
		t_now = _time;
	}
};

#if 0
template <typename T1, typename T2>
static int cmpfunc(HASH_NODE *ptMatchNode, HASH_NODE *ptNode)
{
	DataNode<T1, T2> *pMatch, *pt;
	pMatch = (DataNode<T1, T2>*)(ptMatchNode->data);
	pt = (DataNode<T1, T2>*)(ptNode->data);
	if (pMatch->key == pt->key)
		return true;
	else
		return false;
}
#endif

template <typename T1, typename T2>
class CHashTemplate
{
public:
	CHashTemplate()	{}

	virtual ~CHashTemplate()
	{
		if (hash_id != NULL)
		{
			hashtbl_delete(hash_id);
		}
	}

	virtual bool Initi(int bucket_num, FUNCPTR cmpfunc, FUNCPTR hashfunc)
	{
		hash_id = hashtbl_create(bucket_num, cmpfunc, hashfunc);
		if (hash_id == NULL)
			return false;
		return true;
	}

	virtual bool Find(const T1& key)
	{
		if (FindPtr(key) != NULL)
			return true;
		else
			return false;
	}

	virtual bool Add(const T1& key, const T2& value)
	{
		hash_data.push_back(DataNode<T1, T2>(key, value));
		DataNode<T1, T2> &pdata = hash_data.back();
		DataNode<T1, T2> *p = &pdata;
		hashtbl_add(hash_id, &(p->head));
		return true;
	}

	virtual bool Add(const T1& key, const T2& value, const time_t _time)
	{
		hash_data.push_back(DataNode<T1, T2>(key, value, _time));
		DataNode<T1, T2> &pdata = hash_data.back();
		DataNode<T1, T2> *p = &pdata;
		hashtbl_add(hash_id, &(p->head));
		return true;
	}

	virtual void Clear()
	{
		for (int i = 0; i < hash_id->elements; i++)
		{
			SL_NODE *ptNode = (hash_id->pHashTbl[i]).head;
			while(ptNode != NULL)
			{
				SL_NODE *pNode = ptNode;
				ptNode = ptNode->next;
				hashtbl_remove(hash_id, (HASH_NODE*)pNode);
			}
		}

		hash_data.clear();
		return;
	}

	/*Replace������
	 * ���룺key    ��Ҫ�Ƚϵ�Keyֵ
	 *       new_value  ��Ҫ������Valueֵ
	 *       old_value  �滻֮���Valueֵ
	 *
	 * ����� true  �滻�ɹ�
	 *       false û���ҵ�Key��ͬ�ڵ㣬�޷��滻
	 *
	 * ����˵������key��Hash�в��ң���ʽ��cmpfunc�������Ƿ���Keyֵ��ͬ�Ľڵ㣬��ʹ��new_value�滻��Ӧ�����ݣ���Ҫ���ƺ���֧�֣�
	 * 			����ԭ����Value���ݱ�����old_value��
	 *
	 */
	virtual bool Replace(const T1& key, const T2& new_value, T2& old_value)
	{
		DataNode<T1, T2> *ptr = (DataNode<T1, T2> *)FindPtr(key);
		if (ptr == NULL)
			return false;
		/*T2���=���������*/
		old_value = ptr->value;
		ptr->value = new_value;
		/*��ǰʱ�����*/
		ptr->t_now = time(NULL);
		return true;
	}

	virtual void Del(const T1& key)
	{
		void* ptr = FindPtr(key);
		if (ptr == NULL)
			return;
		else
		{
			hashtbl_remove(hash_id, (HASH_NODE*)ptr);
		}
		return;
	}

	virtual int Rebuild(const int t_limit)
	{
		/*������Ľڵ���Ŀ*/
		int cnt = 0;

		for (int i = 0; i < hash_id->elements; i++)
		{
			SL_NODE *ptNode = (hash_id->pHashTbl[i]).head;
			while(ptNode != NULL)
			{
				DataNode<T1, T2> *pNode = (DataNode<T1, T2> *)ptNode;
				ptNode = ptNode->next;

				/*�ж�ʱ���ǲ�����ͬ*/
				time_t t_now = time(NULL);
				if (t_now - pNode->t_now > t_limit)
				{
					hashtbl_remove(hash_id, (HASH_NODE*)pNode);
					cnt++;
				}
			}
		}
		return cnt;
	}

protected:
	virtual void* FindPtr(const T1& key)
	{
		/*T2Ĭ�Ϲ��캯������*/
		T2 value;
		DataNode<T1, T2> p(key, value);
		return (void*)hashtbl_find(hash_id, &(p.head));
	}

protected:
	HASH_ID hash_id;
	list<DataNode<T1, T2> > hash_data;
};

#endif /* INDEX_H_ */
