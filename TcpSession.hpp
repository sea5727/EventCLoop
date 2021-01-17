#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{
    class TcpSession{
    public:
        Epoll & epoll;
        int sessionfd;
        Buffer buffer;

        TcpSession(Epoll & epoll, int sessionfd)
            : epoll{epoll}
            , sessionfd{sessionfd}
            , buffer{} {  }


        ~TcpSession(){
            clear_session();
        }

        void
        async_read(std::function<void(int, char *, size_t len)> callback){
            using std::placeholders::_1;
            auto event = Event{};
            event.fd = sessionfd;
            event.pop = std::bind(&TcpSession::async_read_pop, this, _1, callback);

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }
        void
        async_read_pop(
            const struct epoll_event & ev, 
            std::function<void(int /*fd*/, char * /*buffer*/, size_t /*len*/)> callback){

            ssize_t len = buffer.read_chunk(ev.data.fd);
            callback(ev.data.fd, buffer.get_buf(), len);
        }
        void
        clear_session(){
            if(sessionfd != -1){
                close(sessionfd);
                sessionfd = -1;
            }
        }
        void
        async_writev(
            const struct iovec *iovecs, 
            int count, 
            std::function<void(Error & /*error*/, int /*fd*/, size_t /*len */)> callback){
                
            Error error;
            auto result = writev(sessionfd, iovecs, count);
            if(result == -1){
                error = Error{strerror(errno)};
            }
            callback(error, sessionfd, result);
        }
        void
        async_write(
            void * data, 
            size_t len, 
            std::function<void(Error & /*error*/, int /*fd*/, size_t /*len */)> callback){
                
            Error error;
            auto result = write(sessionfd, data, len);
            if(result == -1){
                error = Error{strerror(errno)};
            }
            callback(error, sessionfd, result);
        }

    };
}
