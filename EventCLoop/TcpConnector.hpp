#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class TcpConnector{
        Epoll & epoll;
        Event event;
        int sessionfd;
        std::string ip;
        uint16_t port;
        int flag = 0;
    public:
        TcpConnector(Epoll & epoll)
            : epoll{epoll}
            , event{} {


        }

        void
        async_connect(
            const std::string & ip, 
            const uint16_t      port, 
            std::function<void(Error & /*error*/, int /*fd*/)> callback){

            this->ip = ip;
            this->port = port;

            sessionfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sessionfd < 0){
                throw std::logic_error("socket create fail" + std::string{strerror(errno)});
            }
            std::cout << "[CONNECT] fd : " << sessionfd << std::endl;

            int flags = fcntl(sessionfd, F_GETFL, 0);
            fcntl(sessionfd, F_SETFL, flags | O_NONBLOCK );

            using std::placeholders::_1;

            struct sockaddr_in server_addr;
            make_sockaddr_struct(server_addr, ip, port);


            std::cout << "try connect " << std::endl;
            auto ret = ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if(ret == -1){
                std::cout << "async_connect ret == -1, errno : " << errno << std::endl;
                ECONNREFUSED;
            }
            else {
                std::cout << "async_connect ret : " << ret << ", errno : " << errno << std::endl;
                ECONNREFUSED;
            }

            
            event.fd = sessionfd;
            event.pop = std::bind(&TcpConnector::async_connect_pop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN | EPOLLOUT | EPOLLERR; // TODO EPOLLER 은 하지 않아도 될거같은데

            epoll.AddEvent(event, ev);
            
            flag++;
        }
        
    private:

        void
        async_connect_pop(
            const struct epoll_event & ev, 
            std::function<void(Error & /*error*/, int /*fd*/)> callback){

            struct sockaddr_in server_addr;
            make_sockaddr_struct(server_addr, ip, port);
            auto error = Error{};

            if(ev.events & EPOLLERR){ // fail connect
                int nerror = 0; 
                socklen_t len = sizeof( nerror ); 
                if( getsockopt(ev.data.fd, SOL_SOCKET, SO_ERROR, &nerror, &len) < 0 ) {  // 111 : ECONNREFUSED
                    error = Error{strerror(errno)};
                    epoll.DelEvent(ev.data.fd);
                    close(ev.data.fd);
                    event.clear();
                    callback(error, ev.data.fd);
                    return;
                }
                
                /*
                정리
                1. 비동기 connect 실패시 error를 사용하지 말고, gstsockopt 를 사용해라.
                2. 비동기 connect 실패시 소켓을 close하고 재접속해라.
                   ( close하지않고 소켓을 재사용할 경우, connect를 한번더 호출 해주어야 다음 connect시 reconnect 가능.)
                **/

                error = Error{strerror(nerror)};
                epoll.DelEvent(ev.data.fd);
                close(ev.data.fd);
                event.clear();
                callback(error, ev.data.fd);

            }
            else{
                epoll.DelEvent(ev.data.fd);
                event.clear();
                callback(error, ev.data.fd);
            }
        }
        void
        make_sockaddr_struct(
            struct sockaddr_in & server_addr, 
            const std::string & ip, 
            const uint16_t port){

            if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
                throw std::logic_error("socket create fail" + std::string{strerror(errno)});
            }
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
        }
    };
}