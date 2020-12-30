#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <execinfo.h>
#include <sys/wait.h>


void 
SignalHandler(int signal){
    std::cout << "[" << std::this_thread::get_id() << "] SignalHandler signal : " << signal << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "[" << std::this_thread::get_id() << "] SignalHandler signal : end " << std::endl;
    
}

int main(int argc, char * argv[]){

    
    for(int i = SIGHUP ; i < SIGTSTP; ++i){
        signal(i, SignalHandler); 
        // Ctrl + C 를 누르면 SIGINT, 
        // kill -10 pid 는 SIGUSR1 등의 시그널을 주면 mainthread 에서 실행된다.
    }
    

    using std::this_thread::sleep_for;
    using std::chrono::seconds;

    while(1){
        std::cout << "[" << std::this_thread::get_id() << "] Main Thread Sleep 1" << std::endl;
        sleep_for(seconds(1));
        std::cout << "[" << std::this_thread::get_id() << "] Main Thread Sleep 2" << std::endl;
        sleep_for(seconds(1));
        std::cout << "[" << std::this_thread::get_id() << "] Main Thread Sleep 3" << std::endl;
        sleep_for(seconds(1));
    }
    

}