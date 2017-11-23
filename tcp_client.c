/*********************************************************
- Copyright (C), 2017-2018, YU
- File name: tcp_client.c
- Author: YU
- Version: 1.0
- Date: 2017-11-23
- Description: //模块信息
- Function List: //主要函数及其功能
  1.
  2.
- History: //修改历史记录 
	<author>	<time>		<version>	<desc>
	YU 			17/11/23	1.0			build this module
*********************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define portnum 3333

int main(int argc, char const *argv[])
{
	int sockfd;
	char buffer[128];
	struct sockaddr_in server_addr;
	int ret;
	
	//1. 创建套接字
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("create socket error!\n");
		return -1;
	}

	//2.1 设置要连接的服务器地址
	bzero(&server_addr, sizeof(struct sockaddr_in));//将地址清零
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(portnum);
	server_addr.sin_addr.s_addr = inet_addr("10.10.36.88");
	//2.2. 连接服务器
	ret = connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr));
	if (ret == -1)
	{
		printf("connect() error!\n");
		return -1;
	}
	//3. 发送数据到服务器
	printf("Please input char:\n");
	fgets(buffer, 128, stdin);
	ret = send(sockfd, buffer, strlen(buffer), 0);
	if (ret == -1)
	{
		printf("send() error!\n");
		return -1;
	}

	//4. 关闭连接
	close(sockfd);
	return 0;
}
