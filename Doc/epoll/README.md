# Epoll

## 헤더
+ #include <sys/epoll.h>

---

## 생성
+ epoll_create(int size)
    - size : 사용하지 않는 값이다
+ epoll_create1(int flag)
    -  flag : flag가 0이라면 epoll_create와 동일하게 동작한다. 
+ return value : epoll의 File descriptor   
   -1 : error 발생, errno 확인

---

## 사용

등록, 수정, 삭제시 epoll_ctl 에 EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL 플래그를 사용한다

1. 등록 : epollfd에 fd를 등록.
    ```cpp
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    ```
2. 수정 : 이미 등록되어 있는 fd의 event를 변경
    ```cpp
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLOUT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
    ```
3. 삭제
    ```cpp
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
    ```

4. 대기
    epoll 이벤트를 대기할때 epoll_wait 과 결과리스트, timeout 값을 사용한다.
    ```cpp
    const unsigned int MAX_EPOLL_SIZE = 1024;
    ... 
    struct epoll_event ev[MAX_EPOLL_SIZE];
    auto count = epoll_wait(epollfd, ev, MAX_EPOLL_SIZE, 1000); // 1000 ms
    for(int i = 0 ; i < count ; ++i){
        int fd = ev[i].data.fd;
        // TODO Proc
    }
    ```
    
5. 비고
    event의 동작을 정의해주는 필드   
    EPOLLIN : read 와 같은 수신 상황 ( 주로 ctl / wait 모두 사용 )    
    EPOLLOUT : write 와 같은 전송 상황 ( 주로 ctl / wait 모두 사용 )   
    EPOLLERR : 오류를 감지한 상황 ( 주로 wait 의 결과로 사용 )   
