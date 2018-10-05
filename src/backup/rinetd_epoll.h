#ifndef RINETD_EPOLL_H
#define RINETD_EPOLL_H 1


// 读取配置文件
extern int read_config();

// 启动监听
extern int start_listen();


// 启动本地监听,连接远程监听
//extern int selectPass_epoll();


#endif /* RINETD_EPOLL_H */

