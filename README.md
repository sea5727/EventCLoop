# EventCLoop


### C++ Class Type Library that wraps existing c type system programming objects

1. epoll
2. socket
    - accept
    - connect
    - read
    - write
3. tcp buffer class
3. timerfd
4. signalfd
5. eventfd
---
### 1. Epoll
```cpp
#include "EventCLoop.hpp"
auto epoll = EventCLoop::Epoll{};

while(1){
    epoll.Run();
}
```
### 2. Timer
```cpp
#include "EventCLoop.hpp"

auto p_timer = std::make_shared<EventCLoop::Timer>(epoll);

p_timer->initOneTimer(1, 0); // timer ( 1sec )
p_timer->async_wait([](EventCLoop::Error & error) {
    if(error){ // error control
        return;
    }
    //proc
});
```

### 3.Signal
```cpp
auto signal = EventCLoop::Signal<3>{epoll, {SIGHUP,SIGUSR1, SIGUSR2}};

signal.AsyncSignal([](int signalno){ 
    switch(signalno){
        case SIGHUP: // proc sighup
            break;
        case SIGUSR1: // proc sigusr1
            break;
        case SIGUSR2: // proc sigusr2
            break;
        default: // error ? 
            break;
    }
});
```

### 4. Eventfd
```cpp
auto epoll = EventCLoop::Epoll{};
auto threadpool = std::vector<std::thread>{};

//example for event in another thread
threadpool.emplace_back([&epoll]{
    auto event_fd = EventCLoop::Eventfd{epoll}; // epoll 이 실행중인 thread에서 event 호출
    while(1){
        event_fd.SendEvent([]{ // main thread 에서 실행된다.
            std::cout << "proc in main thread per 1seconds" << std::endl;
        });
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}); 

while(1){
    epoll.Run();
}
```

### 5. Socket

### connect :    

non-blocking 인경우 connect 호출시 -1 을 리턴한다. epoll_event를 EPOLLIN | EPOLLOUT 으로 등록하면된다.   
connect 실패인경우 EPOLLERR 를 포함하는데, getsockopt 으로 에러정보를 알아내야한다.   
또한 reconnect를 시도할경우 소켓을 재사용한다면 connect를 한번 더 임의로 호출해줘야한다.    
2번째로 호출되는 connect는 실제로 connect를 요청하지않고 ECONNABORTED 에러를 발생하였다.   
또한 connect timeout을 확인하기위해서는 nonblocking 으로 connect 요청후 select 로 대기하여야하는데 이또한 블럭이 되기때문에 다른방안을 모색해야한다.

```cpp
auto ret = ::connect(sessionfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

struct epoll_event ev;
ev.data.fd = sessionfd;
ev.events = EPOLLIN | EPOLLOUT;

if(epoll_ctl(epollfd, EPOLL_CTL_ADD, sessionfd, &ev) == -1){
    return; // epoll_ctl fail
}
...
epoll_wait
...
if(ev.events & EPOLLERR){ // fail connect
    int nerror = 0; 
    socklen_t len = sizeof( nerror ); 
    if( getsockopt(ev.data.fd, SOL_SOCKET, SO_ERROR, &nerror, &len) < 0 ) {  // 111 : 
        return; // getsockopt fail
    }

    close(ev.data.fd);
}
else{
    struct epoll_event ev;
    ev.data.fd = sessionfd;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, ev.data.fd, &ev); // for read/recv
}
```
### write :
send buffer가 충분한경우 write함수는 항상 성공한다.   
send buffer가 부족한경우, write함수의 리턴값은 전송한 길이만큼이다. 따라서 이경우 추가로 전송해주는 로직이 필요하다.   
또한 TCP_NODELAY 옵션을 사용하도록 한다. 약 40ms 정도의 Nagel 알고리즘 텀이 존재하는것을 확인했다.
```cpp
auto result = write(sessionfd, data, len);
if(result == -1){
    sleep_for(milliseconds(100));
    write(sessionfd, data + result, len - result);
}
```



---
### 이 라이브러리의 장점
1. Proactor Pattern
    - 기존 thread에서 while 루프를 돌면서 하는 방식에서 개선.
2. read io inturrupt minimize
    - tcp length만큼 읽는 방식이 아닌, 전체를 읽은 후 length 만큼 application에 callback해주는 방식
3. c언어를 래핑하여 다른 라이브러리보다 빠르지 않을까? 

---
### 이 라이브러리의 단점
1. async_write callback infinite loop
    - socket의 write/send 함수는 비동기형식이 아닌 결과를 직접 반환하기 때문에 콜백함수에서 다시 send를 하게된다면 무한 loop와 stack overflow에 빠짐. ( asio에서는 async_write는 어떻게 성공유무를 판단하고 callback을 전달하는것일까?? )
2. Error Control
    - Error 시 Exception을 throw하고 있다. error를 리턴하는 방향으로 하면 좋을듯.
    아직 구현량이 적을뿐, 혹시 재사용할경우 추가하도록 하자.
---
### 비고

boost asio 를 공부하고 사용하면서 익힌 내용을 직접 구현해보는것에 의의를 둠.   
다른 프로젝트에도 재사용이 가능할것같다.
