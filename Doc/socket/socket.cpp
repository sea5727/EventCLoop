#include <iostream>
#include <thread>
#include <chrono>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/tcp.h>

constexpr static int EPOLL_SIZE = 1024;
constexpr static int EPOLL_TIMEOUT = 1000;

int
create_nonblock(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    int ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK );
    return ret;
}


/**
 *  TCP_NODELAY : 패킷 전송시 작은 패킷들은 모아서 한꺼번에 전송하는 Nagle 알고리즘 사용 유무.
 *                실제로 sendfile 을 360byte 전송하는데 40ms간격씩 전송하는 현상이 발생. TCP_NODELAY 옵션을 추가하여 해소.
 */
int
nodelay(int fd){
    int nOptVal = 1;
	int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&nOptVal, sizeof(nOptVal));
    if(ret == -1){
        strerror(errno);
        return -1;
    }
    return 0;
}

int main(int argc, char * argv[]){
    
    // std::string ip = "125.209.222.142";
    // uint16_t port = 3122;

    std::string ip = "192.168.0.35";
    uint16_t port = 12345;

    
    int epollfd = epoll_create1(0);
    if(epollfd == -1){
        throw std::logic_error("epoll_create1 fail");
    }
    std::cout << "epollfd : " << epollfd << std::endl;

    int sessionfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sessionfd < 0){
        throw std::logic_error("socket create fail" + std::string{strerror(errno)});
    }
    std::cout << "[CONNECT] fd : " << sessionfd << std::endl;

    int flags = fcntl(sessionfd, F_GETFL, 0);
    fcntl(sessionfd, F_SETFL, flags | O_NONBLOCK );

    struct sockaddr_in server_addr;
    if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
        throw std::logic_error("socket create fail" + std::string{strerror(errno)});
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    auto connect_epoll = epoll_create1(0);
    std::cout << "connect_epoll : " << connect_epoll << std::endl;

    struct epoll_event connev;
    connev.data.fd = sessionfd;
    connev.events = EPOLLIN | EPOLLOUT | EPOLLERR;

    epoll_ctl(connect_epoll, EPOLL_CTL_ADD, sessionfd, &connev);


    struct epoll_event epollev;
    epollev.data.fd = connect_epoll;
    epollev.events = EPOLLIN | EPOLLOUT | EPOLLERR;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, connect_epoll, &epollev);

    struct epoll_event ev[EPOLL_SIZE];
    struct epoll_event ev2[EPOLL_SIZE];

    // int flag = 0;
    while(1){
        auto count = epoll_wait(epollfd, ev, EPOLL_SIZE, EPOLL_TIMEOUT);
        for(int i = 0 ; i < count ; ++i){
            std::cout << "[" << i << "] Epoll event.. fd : " << ev[i].data.fd << ", event: " << ev[i].events << std::endl;
            int fd = ev[i].data.fd;
            if(fd == connect_epoll){
                std::cout << "connect epoll event " << std::endl;
                auto count2 = epoll_wait(connect_epoll, ev2, EPOLL_SIZE, EPOLL_TIMEOUT);
                std::cout << "count2 : " << count2 << std::endl;
            }
            if(fd == sessionfd){
                std::cout << "event : " << ev[i].events << std::endl;
                if(ev[i].events & EPOLLERR){
                    std::cout << "ERR: " << strerror(errno) << std::endl;
                    close(ev[i].data.fd);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}