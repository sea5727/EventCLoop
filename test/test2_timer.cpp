#include "EventCLoop.hpp"


// shared timer 정상 기능 확인

void 
handle_timer(int & i){
    std::cout << "timer callback!! i : " << i++  << std::endl;
}
void
make_shared_interval_timer(EventCLoop::Epoll & epoll){
    EventCLoop::Error error;
    auto p_timer = std::make_shared<EventCLoop::Timer>(epoll, error);
    if(error){
        std::cout << error.what() << std::endl;
        return;
    }


    p_timer->initOneTimer(5, 0, error);
    if(error){
        std::cout << error.what() << std::endl;
        return;
    }
    
    p_timer->async_wait([i = 0, p_timer, &epoll](EventCLoop::Error & error) mutable { // 무한 반복
        if(error){
            std::cout << "Timer Error : " << error.what() << std::endl;
            return;
        }
        handle_timer(i);
        make_shared_interval_timer(epoll);
    });
}


// shared timer 가 해제되면서 클리어가 자동적으로 되는지 확인
void 
make_shared_interval_timer_clear(EventCLoop::Epoll & epoll){
    auto p_timer = std::make_shared<EventCLoop::Timer>(epoll);
    p_timer->initOneTimer(10, 0);
    p_timer->async_wait([](EventCLoop::Error & error){
        std::cout << "ptimer callback!! " << std::endl;
    });
}

int main(int argc, char * argv[]){
    std::cout << "[TEST1] EPOLL " << std::endl;

    auto epoll = EventCLoop::Epoll{};

    make_shared_interval_timer(epoll);

    int i = 0;
    while(1){
        std::cout << "i : " << i << std::endl;
        epoll.Run();
        i++;
    }
    

}