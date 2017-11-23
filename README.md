# Linux socket 通信 TCP

## 服务器端

### 创建socket（socket）

要创建一个套接字，可以调用socket函数。

```
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

参数domain\(域\)确定通信的特性，包括地址格式。各个域有自己的格式表示地址，而表示各个域的常数都以AF\_开头，意指地址族\(address family\)。多数系统还会定义AF\_LOCAL域，这是AF\_UNIX的别名。

| 域 | 描述 |
| :---: | :---: |
| AF\_INET | IPv4因特网域 |
| AF\_INET6 | IPv6因特网域 |
| AF\_UNIX | UNIX域 |
| AF\_UNSPEC | 未指定 |

参数type确定套接字的类型，进一步确定通信特征。

| 类型 | 描述 |
| :---: | :---: |
| SOCK\_DGRAM\(UDP\) | 长度固定的、无连接的不可靠报文传递 |
| SOCK\_RAW | IP协议的数据报接口 |
| SOCK\_SEQPACKET | 长度固定、有序、可靠的面向连接报文传递 |
| SOCK\_STREAM\(TCP\) | 有序、可靠、双向的面向连接字节流 |

参数protocol通常是0，表示按给定的域和套接字类型选择默认协议。当对同一域和套接字类型支持多个协议时，可以使用protocol参数选择一个特定协议。

在AF\_INET通信域中套接字类型SOCK\_STREAM的默认协议是TCP\(传输控制协议\)。在AF\_INET通信域中套接字类型SOCK\_DGRAM默认协议时UDP（用户数据报协议）。

对于数据报（SOCK\_DGRAM）接口，与对方通信时是不需要逻辑连接的。只需要送出一个报文，其地址是一个对方进程所使用的套接字。字节流（SOCK\_STREAM）要求在交换数据之前，在本地套接字和与之通信的远程套接字之间建立一个逻辑连接。

### 绑定地址\(bind\)

可以用bind函数将地址绑定到一个套接字。地址相当于电话号码，套接字相当于电话，这样，这个套接字就可以被网络中的其他客户端访问。

```
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t len);
//返回值：若成功返回0，若出错则返回-1

//通用的地址（IPv4, IPv6都可以用）
struct sockaddr {
    sa_family_t sa_family;//协议族（如AF_INET等）
    char sa_data[14];//地址
}
//IPv4地址
struct sockaddr_in {
    short int sin_family;//协议族
    unsigned short int sin_port;//端口号（2字节）
    struct in_addr sin_addr;//IP地址（4字节）
    unsigned char sin_zero[8];//填充（8字节）
}

struct in_addr {
    unsigned long s_addr;
}
```

**地址转换**

```
in_addr_t inet_addr(const char *cp)
```

功能：将字符串形式的IP地址转化为整数型的IP地址（**网络字节序**）

范例：in\_addr.saddr = inet\_addr\("192.168.1.1"\);

```
char *inet_ntoa(struct in_addr)
```

功能：将整数形式的IP地址转化为字符串形式的IP地址

**网络字节序：大小端相关，网络中传输都是大端模式**

```
/*发方（主机端）, host to net*/
uint32_t htonl(uint32_t hostlong);
//将32位的数据从主机字节序转换为网络字节序
uint16_t htons(uint16_t hostshort);
//将16位的数据从主机字节序转换为网络字节序

/*接收方，net to host*/
uint32_t ntohl(uint32_t netlong);
//将32位的数据从网络字节序转换为主机字节序
uint16_t ntohs(uint16_t netshort);
//将16位的数据从网络字节序转换为主机字节序
```

### 监听端口\(listen\)

服务器调用listen来宣告可以接受连接请求。

```
#include <sys/socket.h>

int listen(int sockfd, int backlog);
//返回值：若成功则返回0，若出错则返回-1
```

参数backlog提供了一个提示，用于表示该进程所要入队的连接请求数量（客户机client的数目）。其实际值由系统决定，但上限由&lt;sys/socket.h&gt;中的SOMAXCONN指定。对于TCP，其默认值为128。一旦队列满，系统会拒绝多余连接请求，所以backlog的值应该基于服务器期望负载和接受连接请求与启动服务的处理能力来选择。

### 等待连接\(accept\)

一旦服务器调用了listen，套接字就能接收连接请求。使用函数accept获得连接请求并建立连接。

```
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict len);
//返回值：若成功则返回文件（套接字）描述符，若出错则返回-1
//第二个参数是用来存储返回客户机的地址的
```

函数accept所返回的文件描述符是套接字描述符，该描述符连接到调用connect的客户端。这个新的套接字描述符和原始套接字（sockfd）具有相同的套接字类型和地址族。传给accept的原始套接字没有关联到这个连接，而是继续保持可用状态并接受其他连接请求。如果不关心客户端标识，可以将参数addr和len设为NULL；否则，在调用accept之前，应将参数addr设为足够大的缓冲区来存放地址，并且将len设为指向代表这个缓冲区大小的整数指针。返回时，accept会在缓冲区填充客户端的地址并且更新指针len所指向的整数为该地址的大小。如果没有连接请求等待处理，accept会阻塞直到一个请求到来。如果服务器调用accept并且当前没有连接请求，服务器会阻塞直到一个请求到来。另外，服务器可以使用poll或select来等待一个请求的到来。在这种情况下，一个待等待处理的连接请求套接字会以可读的方式出现。

### 收发数据\(send\/receive\)

三个函数用来发送数据，三个用于接受数据。首先，考查用于发送数据的函数。

最简单的是send，它和write很像，但是可以指定标志来改变处理传输数据的方式。

```
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t nbytes, int flags);
//返回值：若成功则返回发送的字节数，若出错则返回-1
//这里的sockfd是用accept建立连接以后返回的新sockfd
//第二个参数，要发送的数据是放在什么地方的
//第三个参数，发送的长度
//第四个参数，标志，不需要就设置为0
```

如果send成功返回，并不必然表示连接另一端的进程接收数据。所保证的仅是当send成功返回时，数据已经无错误地发送到网络上。

函数recv和read很像，但是允许指定选项来控制如何接收数据。

```
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buf, size_t nbytes, int flags);
//返回值：以字节计数的消息长度，若无可用消息或对方已经按序结束则返回0，若出错则返回-1
//第一个参数：accept返回的新的sockfd
//第二个参数：接收数据存放的位置
//第三个参数：接收数据的长度
//第四个参数：标志
```

### 结束连接\(close\)

## 客户端

### 建立连接

在开始交换数据以前，需要在请求服务的进程套接字（客户端）和提供服务的进程套接字（服务器）之间建立一个连接。可以用connect建立一个连接。

```
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t len);
//返回值：若成功则返回0，若出错则返回-1
//第一个参数：客户端上创建的套接字sockfd
//第二个参数：与之通信的服务器地址
```

## TCP通讯程序设计

### 服务端

出现错误：warning: cast to pointer from integer of different size \[-Wint-to-point-cast\]

printf\("server get connection from %s\n", inet\_ntoa\(client\_addr.sin\_addr\)\);

运行程序会报错：Segmentation fault \(core dumped\)

解决方法：添加头文件\#include &lt;arpa/inet.h&gt;


