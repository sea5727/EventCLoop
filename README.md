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
다만 괜찮게 성능이 나와서 만족함 ( 다른 프로젝트에도 재사용이 가능할것같다 )
