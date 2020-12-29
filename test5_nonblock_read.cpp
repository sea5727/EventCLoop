#include "EventCLoop/EventCLoop.hpp"

#include <thread>
#include <chrono>

int accepttest(uint16_t port){
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1){
        throw std::logic_error(std::string{"Acceptor socket error : "} + std::string{strerror(errno)});
    }



    int send_buffer;
    int recv_buffer;
    socklen_t len2;
    if (getsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, &len2) < 0){
        throw std::runtime_error("getsockopt SO_SNDBUF error\n");
    }
    if (getsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, &len2) < 0){
        throw std::runtime_error("getsockopt SO_RCVBUF error\n");
    }

    std::cout << "listenfd : " << listenfd << ", send_buffer : " << send_buffer << std::endl;
    std::cout << "listenfd : " << listenfd << ", recv_buffer : " << recv_buffer << std::endl;

    recv_buffer = 10;
    send_buffer = 10;
    if( setsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, sizeof(recv_buffer)) < 0 ){
        throw std::runtime_error("getsockopt SO_RCVBUF error\n");
    }
    if (setsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, sizeof(send_buffer)) < 0 ){
        throw std::runtime_error("getsockopt SO_SNDBUF error\n");
    }

    recv_buffer = 0;
    send_buffer = 0;
    if (getsockopt(listenfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, &len2) < 0){
        throw std::runtime_error("getsockopt SO_SNDBUF error\n");
    }
    if (getsockopt(listenfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, &len2) < 0){
        throw std::runtime_error("getsockopt SO_RCVBUF error\n");
    }


    if(true) {
        int opt_value = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_value, sizeof(opt_value));
    }
    struct sockaddr_in _server_addr;
    
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // TODO BIND IP 할당 필요
    _server_addr.sin_port = htons(port);


    if(bind(listenfd, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) == -1) {
        close(listenfd);
        throw std::logic_error(std::string{"Acceptor bind error : "} + std::string{strerror(errno)});
    }

    if(listen(listenfd, 5) == -1) {
        close(listenfd);
        throw std::logic_error(std::string{"Acceptor listen error : "} + std::string{strerror(errno)});
    }
    return listenfd;
}
int main(int argc, char * argv[]){
    std::cout << "[TEST1] EPOLL " << std::endl;

    int epollfd = epoll_create1(0);
    if(epollfd == -1){
        throw std::logic_error("epoll_create1 fail");
    }



    int acceptfd = accepttest(12345);


    int flag = fcntl(acceptfd, F_GETFL, 0);
    fcntl(acceptfd, F_SETFL, flag | O_NONBLOCK);

    struct epoll_event acceptev;
    acceptev.data.fd = acceptfd;
    acceptev.events = EPOLLIN;

    epoll_ctl(epollfd, EPOLL_CTL_ADD, acceptfd, &acceptev);

    int readfd = -1;


    struct epoll_event ev[1024];
    while(1){

        EAGAIN;
        auto count = epoll_wait(epollfd, ev, 1024, 1000);
        for(int i = 0 ; i < count ; ++i){
            std::cout << "[" << i << "] Epoll event.. event: " << ev[i].events << std::endl;
            int fd = ev[i].data.fd;
            if(fd == acceptfd){
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(struct sockaddr_in);
                readfd = ::accept(acceptfd, (struct sockaddr *)&client_addr, &len);
                std::cout << "Accept!! readfd : " << readfd << std::endl;



                int flag2 = fcntl(readfd, F_GETFL, 0);
                fcntl(readfd, F_SETFL, flag2 | O_NONBLOCK);


                int send_buffer;
                int recv_buffer;
                socklen_t len2;
                if (getsockopt(readfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, &len2) < 0){
                    throw std::runtime_error("getsockopt SO_SNDBUF error\n");
                }
                if (getsockopt(readfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, &len2) < 0){
                    throw std::runtime_error("getsockopt SO_RCVBUF error\n");
                }

                std::cout << "sessionfd : " << readfd << ", send_buffer : " << send_buffer << std::endl;
                std::cout << "sessionfd : " << readfd << ", recv_buffer : " << recv_buffer << std::endl;

                recv_buffer = 10;
                send_buffer = 10;
                if( setsockopt(readfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, sizeof(recv_buffer)) < 0 ){
                    throw std::runtime_error("getsockopt SO_RCVBUF error\n");
                }
                if (setsockopt(readfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, sizeof(send_buffer)) < 0 ){
                    throw std::runtime_error("getsockopt SO_SNDBUF error\n");
                }

                recv_buffer = 0;
                send_buffer = 0;
                if (getsockopt(readfd, SOL_SOCKET, SO_SNDBUF, &send_buffer, &len2) < 0){
                    throw std::runtime_error("getsockopt SO_SNDBUF error\n");
                }
                if (getsockopt(readfd, SOL_SOCKET, SO_RCVBUF, &recv_buffer, &len2) < 0){
                    throw std::runtime_error("getsockopt SO_RCVBUF error\n");
                }

                std::cout << "new send_buffer : " << send_buffer << std::endl;
                std::cout << "new recv_buffer : " << recv_buffer << std::endl;


                char buffer[1024];
                while(1){

                    int ret = write(readfd, buffer, 500);
                    std::cout << "write : " << ret << ", errnor : " << errno << ", str : " << strerror(errno) << std::endl;
                    
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

}