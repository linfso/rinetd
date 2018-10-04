//start from the very beginning,and to create greatness
//@author: Chuangwei Lin
//@E-mail：979951191@qq.com
//@brief： 一个epoll的简单例子,服务端
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/epoll.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <vector>
#include <algorithm>
#include <iostream>
//epoll_event结构体如下
//typedef union epoll_data{
//  void* ptr;
//  int fd;
//  uint32_t u32;
//  uint64_t u64;
//}epoll_data_t;
//struct epoll_event{
//  uint32 events;
//  epoll_data_t data;
//}
typedef std::vector<struct epoll_event> EventList;
//错误输出宏
#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int main(void)
{
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    //为解决EMFILE事件，先创建一个空的套接字
    int idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    int listenfd;
    //if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    //创建一个socket套接字
    if ((listenfd = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket");
    //填充IP和端口
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int on = 1;
    //设置地址的重新利用
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt");
    //绑定地址和端口
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind");
    //监听
    if (listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen");

    std::vector<int> clients;
    int epollfd;
    //创建epollfd，epoll_create1函数可以指定一个选项
    epollfd = epoll_create1(EPOLL_CLOEXEC);

    struct epoll_event event;
    event.data.fd = listenfd;
    //默认出发模式是LT模式（电平出发模式），或上EPOLLET变成ET模式（边沿触发）
    event.events = EPOLLIN;
    //把listenfd事件添加到epollfd进行管理
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);
    ///定义事件列表，初始状态为16个，不够时进行倍增
    EventList events(16);
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int connfd;

    int nready;
    while (1)
    {//epoll_wait返回的时间都是活跃的，events为输出参数
        //nready为返回的事件个数
        nready = epoll_wait(epollfd, &*events.begin(), static_cast<int>(events.size()), -1);
        if (nready == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("epoll_wait");
        }
        if (nready == 0)//没有事件发生
            continue;
        //如果事件的数量达到预定义的上限值
        if ((size_t)nready == events.size())
             events.resize(events.size()*2);//扩充为原来的两倍
        for (int i = 0; i < nready; ++i)
        {
            if (events[i].data.fd == listenfd)
            {//如果监听套接字处于活跃的状态
                peerlen = sizeof(peeraddr);
                connfd = ::accept4(listenfd, (struct sockaddr*)&peeraddr,&peerlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
                if (connfd == -1)
                {
                    if (errno == EMFILE)
                    {//EMFILE错误处理，接受然后优雅地断开
                        close(idlefd);
                        idlefd = accept(listenfd, NULL, NULL);
                        close(idlefd);
                        idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
                        continue;
                    }
                    else
                        ERR_EXIT("accept4");
                }
                //打印IP和端口信息
                std::cout<<"ip="<<inet_ntoa(peeraddr.sin_addr)<<" port="<<ntohs(peeraddr.sin_port)<<std::endl;

                clients.push_back(connfd);
                event.data.fd = connfd;
                event.events = EPOLLIN;//或EPOLLET变成ET模式 
                //把新接受的事件加入关注
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event);
            }
            //如果是pollin事件
            else if (events[i].events & EPOLLIN)
            {//取出文件描述符
                connfd = events[i].data.fd;
                if (connfd < 0)
                    continue;
                //缓冲区
                char buf[1024] = {0};
                //读取内容
                int ret = read(connfd, buf, 1024);
                if (ret == -1)//出错
                    ERR_EXIT("read");
                if (ret == 0)
                {//返回0表示对方关闭了
                    std::cout<<"client close"<<std::endl;
                    close(connfd);
                    event = events[i];
                    //把套接字剔除出去
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, connfd, &event);
                    clients.erase(std::remove(clients.begin(), clients.end(), connfd), clients.end());
                    continue;
                }
                //将消息发送回去
                std::cout<<buf;
                write(connfd, buf, strlen(buf));
            }

        }
    }

    return 0;
}
