#pragma once

#include "EventCLoop.hpp"

namespace EventCLoop
{

    std::string now_str()
    {
        time_t t; 
        struct tm *lt; 
        char time_str[26]; 
        if((t = time(NULL)) == -1) { 
            perror("time() call error"); 
            return ""; 
        } 
        
        if((lt = localtime(&t)) == NULL) { 
            perror("localtime() call error"); 
            return ""; 
        } 

        struct timespec tp;
        int ts;
        ts = clock_gettime(CLOCK_MONOTONIC, &tp);
        long msec = tp.tv_nsec / 1000000 ;

        char buffer[64] = "";
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d %03d", 
            lt->tm_year + 1900, 
            lt->tm_mon + 1, 
            lt->tm_mday, 
            lt->tm_hour, 
            lt->tm_min, 
            lt->tm_sec,
            msec);
        return buffer;
    }
}