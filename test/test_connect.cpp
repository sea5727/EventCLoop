#include <iostream>
#include <thread>
#include <chrono>
#include "EventCLoop.hpp"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// class MyApp{
// public:
//     EventCLoop::Epoll & epoll;
//     EventCLoop::TcpConnect connector;
//     MyApp(EventCLoop::Epoll & epoll, std::string ip, uint16_t port)
//         : epoll{epoll}
//         , connector{epoll, port, ip} { }

//     void
//     run(){
//         connector.async_connect([this](EventCLoop::Error error){
//             std::cout << "this : " << (void *)this << ", async_connect handler start" << std::endl;
//             if(error){
//                std::cout << "async_connect error !!" << error.what() << std::endl; 
//                return;
//             }

//             connector.async_read(std::bind(&MyApp::read_handler, this, _1, _2, _3));
//             std::cout << "this : " << (void *)this << ", async_connect handler end" << std::endl;
//         });
//     }
//     void
//     read_handler(int fd, char * buffer, size_t len){
//         if(len == 0){
//             std::cout << "this : " << (void *)this << ", read_handler len == 0 clear ! " << std::endl;
//             connector.clear_session();
//             return;
//         }

//         std::cout << "this : " << (void *)this << ", read_handler len: " << len << std::endl;
//     }
// };

void
do_connect(EventCLoop::Epoll & epoll, EventCLoop::TcpConnector & connector);
void
do_read(EventCLoop::Epoll & epoll, EventCLoop::TcpConnector & connector, std::shared_ptr<EventCLoop::TcpSession> session){
    session->async_read([session, &epoll, &connector](int fd, char * data, size_t len){
        std::cout << "session->async_read.. len : " << len << std::endl;
        if(len == 0){
            session->clear_session();
            do_connect(epoll, connector);
            return;
        }
        do_read(epoll, connector, session);
    });
}

void
do_connect(EventCLoop::Epoll & epoll, EventCLoop::TcpConnector & connector){
    connector.async_connect("192.168.0.35", 12345, 
        [&](EventCLoop::Error & error , int fd){
            if(error){
                std::cout << "connecct callback fail " << error.what() << std::endl;
                do_connect(epoll, connector);
                return;
            }

            auto session = std::make_shared<EventCLoop::TcpSession>(epoll, fd);
            do_read(epoll, connector, session);

        });
}

int main(int argc, char * argv[]){

    auto epoll = EventCLoop::Epoll{};
    auto connector = EventCLoop::TcpConnector{epoll};
    // auto session = EventCLoop::TcpSession{epoll};

    do_connect(epoll, connector);



    while(1){
        epoll.Run();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}