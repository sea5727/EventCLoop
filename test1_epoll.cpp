#include "EventCLoop/EventCLoop.hpp"

int main(int argc, char * argv[]){
    std::cout << "[TEST1] EPOLL " << std::endl;

    auto epoll = EventCLoop::Epoll{};

    // auto timer1 = EventCLoop::Timer{epoll};

    // timer1.initOneTimer(1, 0);
    // timer1.async_wait([](){
    //     std::cout << "timer1 callback!!" << std::endl;
    // });

    // // auto timer2 = EventCLoop::Timer{epoll};

    // // timer2.initIntervalTimer(2, 0, 5, 0);
    // // timer2.async_wait([](){
    // //     std::cout << "timer2 callback!!" << std::endl;
    // // });

    // EventCLoop::Timer * newtimer = new EventCLoop::Timer{epoll};
    // newtimer->initOneTimer(1, 0);
    // newtimer->async_wait([](){
    //     std::cout << "async callabck!!! " << std::endl;
    // });

    // auto timers = std::vector<std::shared_ptr<EventCLoop::Timer>>{};

    auto p_timer = std::make_shared<EventCLoop::Timer>(epoll);
    p_timer->initIntervalTimer(1, 0, 5, 0);
    p_timer->async_wait([p_timer](){
        std::cout << "ptimer callback!! " << std::endl;
        p_timer;

    });

    int i = 0;
    while(1){
        std::cout << "i : " << i << std::endl;
        epoll.Run();
        i++;
        if(i == 10){
            p_timer->clear();
            p_timer = nullptr;
        }
            
    }
    

}