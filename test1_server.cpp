#include <iostream>
#include <thread>
#include <chrono>
#include "EventCLoop/EventCLoop.hpp"


int main(int argc, char * argv[]){
    std::cout << "Hello test1\n";

    std::map<int, std::shared_ptr<EventCLoop::TcpSession>> sessions;
    auto epoll = EventCLoop::Epoll{};
    auto acceptor = EventCLoop::Acceptor{epoll, 12345, "192.168.0.36"};

    acceptor.async_accept([&](int sessionfd, std::string ip, uint16_t port){
        std::cout << "this is callback\n";
        auto session = std::make_shared<EventCLoop::TcpSession>(epoll, sessionfd);
        session->async_read([=, &sessions](int fd, char * buffer, size_t len){
            if(len == 0){
                close(fd);
                sessions.erase(fd);
                return;
            }
            char * p = nullptr;
            auto mylen = session->buffer.dispatch_chunk(p, 
                [](char * buffer, ssize_t len){
                    if(len < 8) return 0;
                    //proc
                    return 10;
                });
            if(mylen == 0){
                std::cout << "Need More..\n";
            }
            std::cout << "Async_read callback!! len : " << len << std::endl;
        });
        
        sessions.insert(std::make_pair(sessionfd, session));
        // session->buffer.dispatch_chunk()
    });


    while(1){
        epoll.Run();
        // std::this_thread::sleep_for(std::chrono::seconds(1));

    }
    return 0;
}