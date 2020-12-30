# eventfd

## 개요

eventfd 는 file descriptor 통해 이벤트를 통지해준다.

---

### 생성
```cpp
int event_fd = eventfd(0, EFD_NONBLOCK);
if(event_fd == -1)
    throw std::runtime_error(std::string{"eventfd create fail "} + std::string{strerror(errno)});

struct epoll_event eventfd_ev;
eventfd_ev.data.fd = event_fd;
eventfd_ev.events = EPOLLIN;

epoll_ctl(epollfd, EPOLL_CTL_ADD, event_fd, &eventfd_ev);
    
```
---
### 전달
```cpp
uint64_t count = 1;
ssize_t ret = write(event_fd, &count, sizeof(uint64_t));
if(ret == -1) 
    throw std::logic_error("write fail");
```
---

### 통지
```cpp
uint64_t res;
int ret = read(event_fd, &res, sizeof(uint64_t));
std::cout << "SendEventPop : " << ret << ", res : " << resstd::endl;
```
---
### 비고
write 개수만큼 read의 res에서 읽게된다.    
read를 하지못할동안 write를 1의 값으로 N회 해주게된다면   
read시 res에 N값을 읽게된다.   

다른 스레드로 task를 전달하여 콜백으로 실행시키고자 할 때 유용해보인다.
