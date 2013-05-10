
#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include "base_socket.h"

/*
*TCP服务器类
*/
class CSockAcceptor : public CBaseSocket
{
public:

	CSockAcceptor();
    ~CSockAcceptor();

    /*
	*功能：
			构造函数
	*输入参数：
			const char *ip:绑定的IP,为NULL表示绑定本机所有IP
			unsigned short port:绑定的端口
	*输出参数：
			无
	*返回值：	
			-1:出错
			0:成功
	*/
	CSockAcceptor(const char *ip, unsigned short port, int backlog = 5);

	int Init(const char *ip, unsigned short port, int backlog = 5);

    int Accept(struct sockaddr &addr, socklen_t &len, int timeout = -1);

	int Accept(int timeout = -1);

	int Listen(int backlog);
};

#endif
