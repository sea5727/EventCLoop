#include "EventCLoop.hpp"
#include <thread>
#include <execinfo.h>
#include <sys/wait.h>
void
CreateCore(){
    char s_log_name[512] = { 0, };
    auto pid = getpid();

    snprintf(s_log_name, sizeof(s_log_name), "./down_TMF_%d.core", pid);
    printf("[CORE] Core file path[%s] \n", s_log_name);

    FILE *s_log = NULL;

    s_log = fopen(s_log_name, "w");

    if (!s_log) {
        printf(" s_log == NULL -> return\n");
        return;
    }

    printf(" s_log != NULL\n");
    char cmd[128] = { 0, };
    snprintf(cmd, sizeof(cmd), "/usr/bin/pstack %d", pid);

    char str[512] = { 0, };
    FILE *ptr = popen(cmd, "r");
    printf("popen\n");
    if (ptr != NULL) {
        while (1) {
            memset(str, 0x00, sizeof(str));
            if (fgets(str, 512, ptr) == NULL) break;
            if (s_log) fprintf(s_log, "[PSTACK] %s", str);
        }
        pclose(ptr);
    }
    if (s_log) fclose(s_log);
    printf("end\n");


}

char* __Con_Time2Str(time_t tTime, char* pcBuf, const int nMaxBufLen){
	if(pcBuf == NULL) return NULL;
    char *      pszPtr;
    pszPtr = pcBuf;
    if( tTime )    {
        struct tm c_tm;
        localtime_r(&tTime, &c_tm);
        snprintf(pszPtr, nMaxBufLen, "%04d-%02d-%02d %02d:%02d:%02d", (c_tm.tm_year+1900), c_tm.tm_mon+1, c_tm.tm_mday, c_tm.tm_hour,
        		c_tm.tm_min, c_tm.tm_sec);
    }
    else strcpy(pszPtr, "   -");
    return pszPtr;
}

void SigHandler(int nSignal) {
    std::cout << "[" << std::this_thread::get_id() << "]SignalHandler start " << nSignal << std::endl;
    if(nSignal == SIGUSR1 || nSignal == SIGUSR2){
        std::cout << "thi is user signal.." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "thi is user signal.. end " << std::endl;
        return;
    }
    if(nSignal == SIGCHLD){
        int childPid, childStatus;
        if((childPid = waitpid(-1, &childStatus, WNOHANG)) > 0){
            std::cout << "waitpid succ " << std::endl;
        }
        printf("Child %d \n", childPid);
        return;

    }
    else if(nSignal == SIGSEGV){

    }
    std::cout << "SigHandler : " << nSignal << std::endl;
	if(nSignal != 15){
		char szBuf[1024];
		struct tm c_tm;
		time_t tNow = time(NULL);
		localtime_r(&tNow, &c_tm);

		snprintf(szBuf, sizeof(szBuf), "/tmp/down_%04d%02d%02d_%02d%02d%02d_%d.log",
				c_tm.tm_year + 1900, c_tm.tm_mon + 1, c_tm.tm_mday, c_tm.tm_hour, c_tm.tm_min, c_tm.tm_sec, getpid());
		FILE* pFile = fopen(szBuf, "w+");
		if(pFile != NULL){
			fprintf(pFile, "%s Error : signal %d\n", __Con_Time2Str(time(NULL), szBuf, sizeof(szBuf)), nSignal);
			void *callstack[128];
			int frames = backtrace(callstack, 128);
			char** strings = backtrace_symbols(callstack, frames);

			if(strings != NULL){
				int for_i = 0;
				for(for_i = 0; for_i < frames; for_i++){
					fprintf(pFile, "%s [%d]%s\n", __Con_Time2Str(time(NULL), szBuf, sizeof(szBuf)), for_i, strings[for_i]);
				}
				free(strings);
			}
			else{ fprintf(pFile, "Stack is NULL\n"); }
			fclose(pFile);
		}
	}
	if (nSignal != SIGTERM && nSignal != SIGINT) {
		CreateCore();
        // exit(0);
	}
    // exit(0);
}
void print(int id, int count ){
    std::cout << "[THREAD:" << id << "] run " << count << std::endl;
}
int main(int argc, char * argv[]){
    std::cout << "[" << std::this_thread::get_id() << "] TEST 6 start "  << std::endl;

    auto threadpool = std::vector<std::thread>{};

    auto epoll = EventCLoop::Epoll{};

    int nMinSig = SIGHUP;
    int nMaxSig = SIGTSTP;

    int thread_count = 1;

    for(int i = 0 ; i < thread_count ; ++i){
        threadpool.emplace_back([=]{
            int tid = i;
            int count = 0;
            while(1){
                print(tid, count);
                if(count == 5){
                    char buffer[1024];
                    buffer[-1] = 'a';
                    char *p = nullptr;

                    p[1] = 1;

                    std::cout << "buffer : " << buffer << std::endl;
                }
                count ++ ;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    for(int for_i = nMinSig; for_i < nMaxSig; for_i++) {
        signal(for_i, SigHandler);

    }

    for(int i = 0 ; i < thread_count ; ++i){
        threadpool[i].join();
    }
    std::cout << "join end " << std::endl;

    int i = 0;
    while(1){
        print(-1, i);
        std::cout << "i : " << i << std::endl;
        epoll.Run();
        i++;
 
    }
    

}