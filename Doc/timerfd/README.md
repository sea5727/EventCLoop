# Timerfd


## 개요
```cpp

#include <sys/timerfd.h>
int timerfd_create(int clockid, int flags);
int timerfd_settime(int fd, int flags, const struct itimerspec * new_value, struct itimerspec * old_value);
int timerfd_gettime(int fd, struct itimerspec * old_value);
...
uint64_t res;
int ret = read(ev[i].data.fd, &res, sizeof(uint64_t));
```
---

## 설명
file descriptor 를 통해 타이머 통지를 해주는 linux system call api.   
nano second 까지 구분가능하며, select, poll, epoll을 통해 타이머를 감지할 수 있다.
통지 매체를 통해 

---

CLOCK_REALTIME : 시스템 설정에 의해 과거로가거나, 윤초로 의해 미래로 갈 수있는 시스템 시간. date커맨드에서 볼수 있는 시간이다.   
CLOCK_MONOTONIC : 시스템 시간에 영향을 받지 않는다. NTP 시간으로 불연속성은 발생하지 않지만, NTP의 로컬 오실레이터와 서버간의 에러가 존재할경우 주기가 변경될 수 있다.   
CLOCK_PROCESS_CPUTIME_ID : 프로세스 시작부터 증가하는 시간이다.   
CLOCK_THREAD_CPUTIME_ID : 스레드 시작부터 증가하는 시간이다.   
CLOCK_MONOTONIC_RAW : NTP에 의해 규율되지않는 단순한 로컬 오실레이터. NTP와 별개로 사용할 때 유용할 수 있다.    

---
