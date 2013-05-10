
#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "base_socket.h"

/*
*TCP��������
*/
class CSockAcceptor : public CBaseSocket
{
public:

	CSockAcceptor();
    ~CSockAcceptor();

    /*
	*���ܣ�
			���캯��
	*���������
			const char *ip:�󶨵�IP,ΪNULL��ʾ�󶨱�������IP
			unsigned short port:�󶨵Ķ˿�
	*���������
			��
	*����ֵ��	
			-1:����
			0:�ɹ�
	*/
	CSockAcceptor(const char *ip, unsigned short port, int backlog = 5);

	int Init(const char *ip, unsigned short port, int backlog = 5);

    int Accept(struct sockaddr &addr, socklen_t &len, int timeout = -1);

	int Accept(int timeout = -1);

	int Listen(int backlog);
};

#endif
