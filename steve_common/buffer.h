/*
 * @File: buffer.h
 * @Desc: head file of buffer
 * @Author: stevetang
 * @History:
 *      2007-07-17   Create
 * @Copyright: Shenzhen Tencent Co.Ltd.
 */

#ifndef _BUFFER_H
#define _BUFFER_H

#include "objpool.h"

class CBuffer
{
    char * m_buf; /* ���ݻ��� */
    int m_buflen; /* ���ݳ��� */

public:
    CBuffer(int buflen);
    ~CBuffer();

public:
    /*
     * Function: GetBuffer
     * Desc: ��ȡ����
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      ��NULL  -   �ɹ�
     *      NULL    -   ʧ��
     */
    char * GetBuffer() { return m_buf; };

    /*
     * Function: GetBufferLen
     * Desc: ��ȡ���峤��
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      ��NULL  -   �ɹ�
     *      NULL    -   ʧ��
     */
    int GetBufferLen() { return m_buflen; };
};

class CBufferPool : public CObjPool<CBuffer>
{
    char m_name[100];
    int m_buflen; /* ���ݳ��� */

public:
    CBufferPool(const char * name, int num, int buflen = 50*1024, unsigned char block = 0);
    ~CBufferPool();

protected:
    /*
     * Function: newobject
     * Desc: ��������
     * In: 
     *      none
     * Out: 
     *      none
     * Return code: 
     *      ��NULL  -   �ɹ�
     *      NULL    -   ʧ��
     */
    CBuffer * newobject();

public:
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
	CBuffer* get();

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
	int release(CBuffer* elem);

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
	const char * get_name() { return m_name; };
};

#endif
