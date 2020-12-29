#pragma once

#include "Epoll.hpp"
#include "Event.hpp"
#include "TcpBuffer.hpp"



namespace EventCLoop
{
    class TcpSession{
    public:
        Epoll & epoll;
        int sessionfd;
        TcpBuffer buffer;

        TcpSession(Epoll & epoll, int sessionfd)
            : epoll{epoll}
            , sessionfd{sessionfd} {
                std::cout << "default TcpSession Create\n";
        }

        TcpSession(Epoll & epoll)
            : epoll{epoll}
            , sessionfd{sessionfd} {
            
            sessionfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sessionfd < 0){
                throw std::logic_error("socket create fail " + std::string{strerror(errno)});
            }
        }

        ~TcpSession(){
            std::cout << "default TcpSession Delete\n";
            // epoll.DelEvent(sessionfd);
        }

        void
        async_read(std::function<void(int, char *, size_t len)> callback){
            auto event = std::make_shared<Event>();
            event->fd = sessionfd;
            event->pop = [this, callback](struct epoll_event ev){
                std::cout << "session pop.. event : " << ev.events << std::endl; 
                ssize_t len = buffer.read_chunk(ev.data.fd);
                std::cout << "session pop readlen : " << len << std::endl;
                callback(ev.data.fd, buffer.get_buf(), len);
                if(len == 0){
                    std::cout << "len == 0..." << std::endl;
                    auto ret = epoll.DelEvent(ev.data.fd);
                    std::cout << "close start" << std::endl;
                    close(ev.data.fd);
                    std::cout << "close end.." << std::endl;
                    return;
                }
            };

            struct epoll_event ev;
            ev.data.fd = sessionfd;
            ev.events = EPOLLIN;

            epoll.AddEvent(event, ev);
        }

        void
        async_write(char * data, size_t len, std::function<void(Error, int)> callback){
            Error error;
            auto result = write(sessionfd, data, len);
            if(result == -1){
                error = Error{strerror(errno)};
            }
            callback(error, result);
        }
    };
}
