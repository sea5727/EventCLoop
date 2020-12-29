#include "EventCLoop/EventCLoop.hpp"

#include <string>
#include <thread>
#include <chrono>
#include <bitset>


void
make_sockaddr_struct(struct sockaddr_in & server_addr, const std::string & ip, uint16_t port){
    if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
        throw std::logic_error("socket create fail" + std::string{strerror(errno)});
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
}

int connecttest(const std::string & ip, uint16_t port){
    int sessionfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sessionfd < 0){
        throw std::logic_error("socket create fail" + std::string{strerror(errno)});
    }
    std::cout << "[CONNECT] fd : " << sessionfd << std::endl;

    int send_buffer;
    int recv_buffer;
    socklen_t len;
    if (getsockopt(sessionfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, &len) < 0){
        throw std::runtime_error("getsockopt SO_SNDBUF error\n");
    }
    if (getsockopt(sessionfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, &len) < 0){
        throw std::runtime_error("getsockopt SO_RCVBUF error\n");
    }

    std::cout << "send_buffer : " << send_buffer << std::endl;
    std::cout << "recv_buffer : " << recv_buffer << std::endl;

    recv_buffer = 10;
    send_buffer = 10;
    if( setsockopt(sessionfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, sizeof(recv_buffer)) < 0 ){
        throw std::runtime_error("getsockopt SO_RCVBUF error\n");
    }
    if (setsockopt(sessionfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, sizeof(send_buffer)) < 0 ){
        throw std::runtime_error("getsockopt SO_SNDBUF error\n");
    }

    recv_buffer = 0;
    send_buffer = 0;
    if (getsockopt(sessionfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, &len) < 0){
        throw std::runtime_error("getsockopt SO_SNDBUF error\n");
    }
    if (getsockopt(sessionfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, &len) < 0){
        throw std::runtime_error("getsockopt SO_RCVBUF error\n");
    }

    std::cout << "new send_buffer : " << send_buffer << std::endl;
    std::cout << "new recv_buffer : " << recv_buffer << std::endl;




    struct sockaddr_in server_addr;
    make_sockaddr_struct(server_addr, ip, port);

    int flags = fcntl(sessionfd, F_GETFL, 0);
    fcntl(sessionfd, F_SETFL, flags | O_NONBLOCK );

    auto ret = ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    std::cout << "connect... ret : " << ret << std::endl;
    return sessionfd;
}
int main(int argc, char * argv[]){
    std::cout << "[TEST1] EPOLL " << std::endl;

    int epollfd = epoll_create1(0);
    if(epollfd == -1){
        throw std::logic_error("epoll_create1 fail");
    }

    int connectfd = connecttest("192.168.0.35", 12345);

    struct epoll_event acceptev;
    acceptev.data.fd = connectfd;
    acceptev.events = EPOLLIN | EPOLLOUT | EPOLLERR;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, connectfd, &acceptev);

    int readfd = -1;

    struct sockaddr_in server_addr;
    char buffer[1024];
    struct epoll_event ev[1024];

    int flag = 0;
    while(1){

        EAGAIN;
        auto count = epoll_wait(epollfd, ev, 1024, 1000);
        for(int i = 0 ; i < count ; ++i){
            std::cout << "[" << i << "] Epoll event.. event: " << ev[i].events << std::endl;
            int fd = ev[i].data.fd;
            if(fd == readfd){
                std::cout << "read event : " << ev[i].events << std::endl;
                int ret = read(readfd, buffer, 1);
                std::cout << "read ret : " << ret << std::endl;
                if(ret == 0){
                    close(connectfd);
                    readfd = -1;
                    flag = 0;
                    connectfd = connecttest("192.168.0.35", 12345);

                    struct epoll_event acceptev;
                    acceptev.data.fd = connectfd;
                    acceptev.events = EPOLLIN | EPOLLOUT | EPOLLERR;

                    epoll_ctl(epollfd, EPOLL_CTL_ADD, connectfd, &acceptev);

                }
            }
            else if(fd == connectfd){
                std::bitset<8> x(ev[i].events);
                std::cout << "connect? , event : "<< x << ", dec:" << ev[i].events << std::endl;
                
                if(ev[i].events & EPOLLIN ){
                    std::cout << "event is EPOLLIN " << std::endl;
                }
                if(ev[i].events & EPOLLPRI ){
                    std::cout << "event is EPOLLPRI " << std::endl;
                }
                if(ev[i].events & EPOLLOUT ){
                    std::cout << "event is EPOLLOUT " << std::endl;
                }
                if(ev[i].events & EPOLLMSG ){
                    std::cout << "event is EPOLLMSG " << std::endl;
                }
                if(ev[i].events & EPOLLERR ){
                    std::cout << "event is EPOLLERR " << std::endl;
                }
                if(ev[i].events & EPOLLHUP ){
                    std::cout << "event is EPOLLHUP " << std::endl;
                }
                
                if( ev[i].events & EPOLLERR ){
                    int nerror;
                    socklen_t len = sizeof(nerror);
                   
                    if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &nerror, &len) < 0){
                        throw std::logic_error("TcpConnect EPOLLERR getsockopt error:" + std::string{strerror(errno)});
                    }
                    std::cout << "strerror(errno) : " << strerror(errno) << std::endl;
                    close(fd);
                    int connectfd = connecttest("192.168.0.35", 12345);
                    struct epoll_event acceptev;
                    acceptev.data.fd = connectfd;
                    acceptev.events = EPOLLIN | EPOLLOUT | EPOLLERR;

                    epoll_ctl(epollfd, EPOLL_CTL_ADD, connectfd, &acceptev);
                }
                else {
                    flag = 1;
                    struct epoll_event connectev;
                    connectev.data.fd = connectfd;
                    connectev.events = EPOLLIN;

                    epoll_ctl(epollfd, EPOLL_CTL_MOD, connectfd, &connectev);
                    readfd = connectfd;
                }
            }
        }
        if(flag){
            char data[1024];
            int ret = read(connectfd, data, 1);
            std::cout << "read ret : " << ret << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

}