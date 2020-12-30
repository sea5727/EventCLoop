#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <execinfo.h>
#include <sys/wait.h>

void 
SignalHandler(int signal){
    std::cout << "[" << std::this_thread::get_id() << "] SignalHandler signal : " << signal << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "[" << std::this_thread::get_id() << "] SignalHandler signal : end " << std::endl;
    
}
void
init_signal(){
    for(int j = SIGHUP ; j < SIGTSTP; ++j){
            if(j != SIGINT )
                signal(j, SignalHandler); 
    } 
}


int main(int argc, char * argv[]){

    const int THREAD_COUNT = 3;
    auto threadpool = std::vector<std::thread>{};

    // init_signal();// 다른 스레드들이 있어도 역시 main 스레드에서 실행, 다른 thread에서 init을 해도 main 스레드에서 실행.

    for(int i = 0 ; i < THREAD_COUNT ; ++i){

        threadpool.emplace_back([i]{
            
            int count = 0;
            while(1){
                std::cout << "[" << std::this_thread::get_id() << "][" << count << "] Thread Sleep 1" << std::endl;
                count += 1;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if(i == 0 && count == 3){
                    int fault = 10 / 0;
                    std::cout << "fault : " << fault << std::endl;
                }
            }  
        });
    }


    
 
    int count = 0;
    while(1){
        std::cout << "[" << std::this_thread::get_id() << "][" << count << "] main Thread Sleep 1" << std::endl;
        count += 1;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }  
    

}