# socket

## 개요
소켓프로그래밍에 대해선 많은 자료가 있기 때문에 기본적인것은 패스

---
### 비고
1. 리눅스 socket 프로그래밍은 blocking + select/poll/epoll 을 사용하여 IOCP로 주로 구현한다.   
이를 디자인패턴으로는 Proactor 패턴이라 부르나 보다.    
Boost asio가 epoll을 래핑한 IOCP 형태의 라이브러리이다.

2. sendfile 시 NO_DELAY 옵션때문에 고생하였다. sendfile을 360byte전송하였는데 40ms 주기로 전송됨을 확인하였고, TCP_NODELAY 옵션을 주어 해소하였다.

3. write, send 에서 non-blocking은 소켓버퍼가 가득 찼을때 실패(-1)로 떨어지는지, 대기하는지를 구분한다. (전송성공에 대한 통지가 아님을 유의)

3. ::connect는 비동기 프로그램시 Timeout 이 까다롭다. non-blocking 옵션과 ::connect 수행이후 아래에 select, epoll을 사용하여 timeout을 구할 수 있지만 select와 epoll에서 blocking 되기때문에 맞는 방법인지도 모르겠다.   

4. Boost의 asio 를 사용해보았고, 공부한 내용을 구현해보기위해 EventCLoop를 만들어보았다.   




