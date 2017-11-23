/*********************************************************
- Copyright (C), 2017-2018, YU
- File name: tcp_server.c
- Author: YU
- Version: 1.0
- Date: 2017-11-23
- Description: Linux TCP socket通信//模块信息
- Function List: //主要函数及其功能
  1.
  2.
- History: //修改历史记录 
	<author>	<time>		<version>	<desc>
	YU 			17/11/23	1.0			build this module
	YU			17/11/23	2.0			多进程处理并发请求
*********************************************************/
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define portnum 3333

int main(int argc, char const *argv[])
{
	int sockfd, new_sockfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buffer[128];//存放接收到的数据
	int nbyte;
	int addr_size;	
	int ret;
	int pid;

	//1. 创建套接字
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("create socket error!\n");
		return -1;
	}

	//2.1 设置要绑定的地址
	bzero(&server_addr, sizeof(struct sockaddr_in));//将地址清零
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnum);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//监听任意ip

	//2.2 绑定地址
	ret = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (ret == -1)
	{
		printf("bind() error!\n");
		return -1;
	}
	//3. 监听接口
	printf("Wait for connect...\n");
	ret = listen(sockfd, 5);
	if (ret == -1)
	{
		printf("listen() error!\n");
		return -1;
	}

	while(1)
	{
		//4. 等待连接
		addr_size = sizeof(struct sockaddr);
		new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
		if (new_sockfd == -1)
		{
			printf("accept() error!\n");
			return -1;
		}
		printf("server get connection from %s!\n", inet_ntoa(client_addr.sin_addr));

		//由子进程来处理数据通讯
		pid = fork();
		if (pid == 0)
		{
			//5. 接收数据
			nbyte = recv(new_sockfd, buffer, 128, 0);
			if (nbyte == -1)
			{
				printf("recv() error!\n");
				return -1;
			}
			buffer[nbyte] = '\0';//添加字符串结束符，方便打印
			printf("server received: %s\n", buffer);

			close(sockfd);
			close(new_sockfd);

			return 0;
		} else if (pid < 0) {
			printf("fork error!\n");
		}
		//6. 结束连接
		close(new_sockfd);
	}

	close(sockfd);
	return 0;
}
