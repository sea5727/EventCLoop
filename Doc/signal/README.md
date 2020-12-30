# signal/signalfd

## 개요
---
### 1.  signal   
한번의 등록으로 모든 스레드들에서 시그널이 감지된다.    
메인 스레드에서 먼저 감지, 이후 스레드들에서 캐치하였다. 캐치할수 있는 스레드가 없으면, 이후 시그널은 버려졌다.   
모든 스레드들에서 감지가 가능하므로, Critical한 시그널을 등록, core및 exit를 실행시키는쪽으로 사용하면 될것같다.
``` cpp
#include <signal.h>
typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, signalhandler_t handler);
```
---
### 2. signalfd   
file descriptor를 사용, select, poll, epoll을 통해 시그널을 감지한다.   
감지중인 thread에서만 감지되었다.    
즉 메인에서 signalfd를 감지중이라면 스레드에서 SIGSEGV 등이 발생하여도 캐치하지 못한다   
해당 스레드에서만 감지하므로 SIGUSR1, SIGUSR2 와 같은 시그널을 워커 스레드에 등록하여
서비스로직에도 사용가능해 보인다.
```cpp
#include <sys/signalfd.h>
...
auto signals = std::array{SIGHUP,SIGUSR1, SIGUSR2};

sigset_t mask;
::sigemptyset(&mask); // mask initialize

for(auto & signal : signals){ // add signal
    ::sigaddset(&mask, signal);
}

int ret = ::sigprocmask(SIG_SETMASK, &mask, NULL);
if(ret == -1)
    throw std::runtime_error(std::string{"sigprocmask error "} + std::string{strerror(errno)});

signal_fd = ::signalfd(-1, &mask, 0);
if(signal_fd == -1)
    throw std::runtime_error(std::string{"signalfd error "} + std::string{strerror(errno)});

struct epoll_event ev;
ev.data.fd = signal_fd;
ev.events = EPOLLIN;

if(epoll_ctl(epollfd, EPOLL_CTL_ADD, signal_fd, &ev) == -1){
    throw std::logic_error(std::string{"epoll_ctl EPOLL_CTL_ADD fail"} + std::string{strerror(errno)});
}

... epoll wait 

struct signalfd_siginfo fdsi;
int res = read(ev.data.fd , &fdsi, sizeof(fdsi));

if(res != sizeof(fdsi)){
    throw std::runtime_error(std::string{"signal system error:"} + std::string{strerror(errno)});
}

auto no = fdsi.ssi_signo;
std::cout << "signal : " << no << std::endl;

```
---
### 비고
signal은 전체 스레드에서 감지를 하니, crash한 signal들만 등록하여 사용하고,   
signalfd 는 기능을 위한 signal들을 등록하여 사용하는것이 좋아보인다.
