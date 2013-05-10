/*
 * @File: dbconn.cpp
 * @Desc: implement file of db server interface
 * @Author: stevetang
 * @History:
 *      2007-06-06   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#include "general.h"
#include "buffer.h"

CBuffer::CBuffer(int buflen) : m_buflen(buflen)
{
    m_buf = (char *)malloc(m_buflen);
}

CBuffer::~CBuffer()
{
    if (m_buf)
    {
        free(m_buf);
        m_buf = NULL;
    }
}

CBufferPool::CBufferPool(const char * name, int num, int buflen /* = 50*1024 */, unsigned char block /* = 0 */) : 
    CObjPool<CBuffer>(num, block), m_buflen(buflen)
{
    /* 设置IP地址 */
    memset(m_name, 0, sizeof(m_name));
    strncpy(m_name, name, sizeof(m_name)-1);

    /* 初始化对象池 */
    init();
}

CBufferPool::~CBufferPool()
{
}

/*
 * Function: newobject
 * Desc: 创建对象
 * In: 
 *      none
 * Out: 
 *      none
 * Return code: 
 *      非NULL  -   成功
 *      NULL    -   失败
 */
CBuffer * CBufferPool::newobject()
{
    CBuffer * buff = NULL;

    buff = new CBuffer(m_buflen);

    return buff;
}

/****************************************************************************** 
 * FUNCTION:  CObjPool.get 
 * DESCRIPTION:  get object from the idle queue 
 * Input: 
 * Output: an object quote
 * Returns: 
 * 
 * modification history
 * --------------------
 * 01a, 30may2007, nancyyu written
 * --------------------
 ******************************************************************************/
CBuffer* CBufferPool::get()
{
    CBuffer * buff = CObjPool<CBuffer>::get();

    return buff;
}

/****************************************************************************** 
 * FUNCTION:  CObjPool.release 
 * DESCRIPTION:  release object,and push it to the idle queue 
 * Input: T& elem, an object quote
 * Output: 
 * Returns: 
 * 
 * modification history
 * --------------------
 * 01a, 30may2007, nancyyu written
 * --------------------
 ******************************************************************************/
int CBufferPool::release(CBuffer* elem)
{
    int ret = CObjPool<CBuffer>::release(elem);

    return ret;
}
