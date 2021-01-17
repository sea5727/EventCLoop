#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class TcpConnector{
        Epoll & epoll;
        int sessionfd = -1 ;
        std::string ip = "";
        uint16_t port = 0;

        std::optional<std::future<void>> conn3;
    public:
        TcpConnector() = default;
        TcpConnector(Epoll & epoll)
            : epoll{epoll} { }

        void
        async_connect(
            const std::string & ip, 
            const uint16_t      port, 
            std::function<void(Error & /*error*/, int /*fd*/)> callback) noexcept {

            this->ip = ip;
            this->port = port;

            sessionfd = socket(AF_INET, SOCK_STREAM, 0);

            if(sessionfd < 0){ // socket create fail
                auto error = Error{strerror(errno)};
                async_callback(error, callback);
                return ;
            }

            if(true){ //TODO TCP NODLEAY
                int nOptVal = 1;
                int ret = setsockopt(sessionfd, IPPROTO_TCP, TCP_NODELAY, (char *)&nOptVal, sizeof(nOptVal));
                if(ret == -1){
                    auto error = Error{strerror(errno)};
                    close(sessionfd);
                    sessionfd = -1;
                    async_callback(error, callback);
                    return ;
                }
            }

            if(true){ //TODO NON_BLOCK -> EINPROGRESS
                int flags = fcntl(sessionfd, F_GETFL, 0);
                int ret = fcntl(sessionfd, F_SETFL, flags | O_NONBLOCK );   
                if(ret == -1){
                    auto error = Error{strerror(errno)};
                    close(sessionfd);
                    sessionfd = -1;
                    async_callback(error, callback);
                    return ;
                }
            }         

            auto error = EventCLoop::Error{};
            struct sockaddr_in server_addr = make_sockaddr_struct(ip, port, error);
            if(error){
                close(sessionfd);
                sessionfd = -1;
                async_callback(error, callback);
            }

            auto ret = ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

            if(ret == 0){ // connect success
                async_callback(error, callback);
                return;
            }

            if(errno == EINPROGRESS){ // non blocking
                errno = 0;

                if(true){ //TODO async Connection Timeout 
                    conn3.emplace(std::async([this, callback]{
                        async_connect_timeout(callback);
                    }));
                }
                else { // Uncheck Timeout
                    using std::placeholders::_1;
                    auto event = Event{};
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
            sessionfd = -1;
            error = Error{strerror(errno)};
            async_callback(error, callback);
            
        }
        
    private:
        void
        async_connect_timeout(
            std::function<void(Error & /*error*/, int /*fd*/)> callback ) noexcept {

            fd_set fdset; 
            struct timeval tv;
            tv.tv_sec = 5;
            tv.tv_usec = 0;
            FD_ZERO(&fdset); 
            FD_SET(sessionfd, &fdset);
            auto ret = select(sessionfd+1, NULL, &fdset, NULL, &tv); 
            if(ret < 0){ // connect fail
                auto error = Error{strerror(errno)};
                close(sessionfd);
                sessionfd = -1;
                async_callback_and_clear_future(error, callback);
                return;
            }

            if(ret == 0){
                close(sessionfd);
                sessionfd = -1;
                auto error = Error{"Connection Timeout:" + ip + ":" + std::to_string(port)};
                async_callback_and_clear_future(error, callback);
                return;
            }

            int nerror = 0; 
            socklen_t len = sizeof( nerror ); 
            if( getsockopt(sessionfd, SOL_SOCKET, SO_ERROR, &nerror, &len) < 0 ) {  // 111 : ECONNREFUSED
                close(sessionfd);
                sessionfd = -1;
                auto error = Error{strerror(errno)};
                async_callback_and_clear_future(error, callback);
                return;
            }

            if(nerror){
                close(sessionfd);
                sessionfd = -1;
                auto error = Error{strerror(nerror)};
                async_callback_and_clear_future(error, callback);
                return;
            }

            auto error = Error();
            async_callback_and_clear_future(error, callback);

        }

        void 
        async_callback_and_clear_future(
            Error error,
            std::function<void(Error & /*error*/, int /*fd*/)> callback) noexcept {
            
            auto eventfd = std::make_shared<Eventfd>(epoll);
            eventfd->SendEvent([this, error, callback, eventfd]{
                if(conn3){
                    conn3.value().get();
                    conn3.reset();
                }                
                auto _error = error;
                callback(_error, sessionfd);
            });
        }

        void 
        async_callback(
            Error error,
            std::function<void(Error & /*error*/, int /*fd*/)> & callback) noexcept {
            
            auto eventfd = std::make_shared<Eventfd>(epoll);
            eventfd->SendEvent([this, eventfd,  error, callback]{
                auto _error = error;
                callback(_error, sessionfd);
            });
        }

        void
        async_connect_pop(
            const struct epoll_event & ev, 
            std::function<void(Error & /*error*/, int /*fd*/)> callback) noexcept {

            auto error = EventCLoop::Error{};
            // struct sockaddr_in server_addr = make_sockaddr_struct(ip, port, error);
            if(error){
                close(sessionfd);
                sessionfd = -1;
                async_callback(error, callback);
                return;                
            }
            

            if(ev.events & EPOLLERR){ // fail connect
                int nerror = 0; 
                socklen_t len = sizeof( nerror ); 
                if( getsockopt(ev.data.fd, SOL_SOCKET, SO_ERROR, &nerror, &len) < 0 ) {  // 111 : ECONNREFUSED

                    // EWOULDBLOCK;
                    error = Error{strerror(errno)}; 
                    epoll.DelEvent(ev.data.fd);
                    close(ev.data.fd);
                    callback(error, ev.data.fd);
                    return;
                }
                
                error = Error{strerror(nerror)};
                epoll.DelEvent(ev.data.fd);
                close(ev.data.fd);
                callback(error, ev.data.fd);

            }
            else{
                epoll.DelEvent(ev.data.fd);
                // event.clear();
                callback(error, ev.data.fd);
            }
        }

        struct sockaddr_in
        make_sockaddr_struct (
            const std::string & ip, 
            const uint16_t port,
            EventCLoop::Error & error) noexcept {

            struct sockaddr_in server_addr;
            if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0){
                error = EventCLoop::Error{std::string{"inet_pton fail : "} + strerror(errno)};
                return server_addr;
            }
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            return server_addr;
        }
    };
}