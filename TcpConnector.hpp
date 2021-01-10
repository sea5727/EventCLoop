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

        std::vector<std::thread> conns;
        std::vector<std::future<void>> conns2;
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

            if(sessionfd < 0){ // socket create fail
                auto error = Error{strerror(errno)};
                async_connect_error_pop(error, callback);
                return ;
            }

            if(true){ //TODO TCP NODLEAY
                
                int nOptVal = 1;
                int ret = setsockopt(sessionfd, IPPROTO_TCP, TCP_NODELAY, (char *)&nOptVal, sizeof(nOptVal));
                if(ret == -1){
                    close(sessionfd);
                    auto error = Error{strerror(errno)};
                    async_connect_error_pop(error, callback);
                    return ;
                }
            }

            if(true){ //TODO NON_BLOCK -> EINPROGRESS
                int flags = fcntl(sessionfd, F_GETFL, 0);
                int ret = fcntl(sessionfd, F_SETFL, flags | O_NONBLOCK );   
                if(ret == -1){
                    close(sessionfd);
                    auto error = Error{strerror(errno)};
                    async_connect_error_pop(error, callback);
                    return ;
                }
            }

 

            using std::placeholders::_1;

            struct sockaddr_in server_addr;
            make_sockaddr_struct(server_addr, ip, port);

            auto ret = ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

            if(ret == 0){
                auto eventfd = std::make_shared<Eventfd>(epoll);
                eventfd->SendEvent([this, callback, eventfd]{
                    Error error;
                    callback(error, sessionfd);
                });
                return;
            }


            if(errno == EINPROGRESS){
                errno = 0;

                if(true){
                    conns2.emplace_back(std::async([this, callback]{
                        async_connect_timeout(callback);
                    }));
                }


                else { // Timeout check 불가.
                    event.fd = sessionfd;
                    event.pop = std::bind(&TcpConnector::async_connect_pop, this, _1, callback);

                    struct epoll_event ev;
                    ev.data.fd = sessionfd;
                    ev.events = EPOLLIN | EPOLLOUT; 

                    epoll.AddEvent(event, ev);
                }


                return;
            }

            close(sessionfd);
            auto error = Error{strerror(errno)};
            async_connect_error_pop(error, callback);
            
        }
        
    private:
        void
        async_connect_timeout(
            std::function<void(Error & /*error*/, int /*fd*/)> callback ){
            fd_set fdset; 
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            FD_ZERO(&fdset); 
            FD_SET(sessionfd, &fdset);
            auto ret = select(sessionfd+1, NULL, &fdset, NULL, &tv); 
            if(ret < 0){
                close(sessionfd);
                auto error = Error{strerror(errno)};
                async_connect_error_pop_conns_clear(error, callback);
                return;
            }

            if(ret == 0){
                close(sessionfd);
                auto error = Error{"Connection Timeout"};
                async_connect_error_pop_conns_clear(error, callback);
                return;
            }

            int nerror = 0; 
            socklen_t len = sizeof( nerror ); 
            if( getsockopt(sessionfd, SOL_SOCKET, SO_ERROR, &nerror, &len) < 0 ) {  // 111 : ECONNREFUSED
                close(sessionfd);
                auto error = Error{strerror(errno)};
                async_connect_error_pop_conns_clear(error, callback);
                return;
            }

            if(nerror){
                close(sessionfd);
                auto error = Error{strerror(nerror)};
                async_connect_error_pop_conns_clear(error, callback);
                return;
            }

            auto error = Error();
            async_connect_error_pop_conns_clear(error, callback);

        }

        void 
        async_connect_error_pop_conns_clear(
            Error error,
            std::function<void(Error & /*error*/, int /*fd*/)> callback){
            
            auto eventfd = std::make_shared<Eventfd>(epoll);
            eventfd->SendEvent([this, error, callback, eventfd]{
                auto end = std::remove_if(conns2.begin(), conns2.end(), [](std::future<void> & fut){
                    fut.get();
                    return true;
                });
                conns2.erase(end, conns2.end());
                
                auto _error = error;
                callback(_error, sessionfd);
            });
        }

        void 
        async_connect_error_pop(
            Error error,
            std::function<void(Error & /*error*/, int /*fd*/)> callback){
            
            auto eventfd = std::make_shared<Eventfd>(epoll);
            eventfd->SendEvent([this, error, callback, eventfd]{
                auto _error = error;
                callback(_error, sessionfd);
            });
        }

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

                    // EWOULDBLOCK;
                    error = Error{strerror(errno)}; 
                    epoll.DelEvent(ev.data.fd);
                    close(ev.data.fd);
                    event.clear();
                    callback(error, ev.data.fd);
                    return;
                }
                
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