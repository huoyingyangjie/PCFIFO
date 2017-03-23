//
// Created by root on 3/21/17.
//
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include <thread>
#include "include/PCFIFO.h"
//#include <boost/thread>
struct my_message{
    int a;
};
static uint64_t loopnum=1000L*1000L*300;
PCGroup<my_message> * pcGroup;
static uint64_t worker_counter=0;
void * worker(void *){
    my_message *mm;
    while (1)
    {
        mm=pcGroup->consumer.get_event_wait();

        pcGroup->consumer.put_event_wait(mm);

        ++worker_counter;
    }
}

int main(int argc ,char *argv[])
{


    PCFIFO<my_message> pcfifo;
    pcGroup=pcfifo.create_one_pc(1024*8);
    struct my_message *message ;
    pthread_t  p1;
    pthread_create(&p1,NULL,worker,NULL);


    uint64_t counter=0;
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    my_message *mm;
    while (counter<loopnum)
    {
        mm=pcGroup->producer.get_event_wait();
        
        pcGroup->producer.put_event_wait(mm);

        ++counter;
    }

    while (worker_counter <loopnum)
    {
        std::this_thread::yield();
    }
    gettimeofday(&end_time, NULL);
    double start, end;
    start = start_time.tv_sec + ((double) start_time.tv_usec / 1000000);
    end = end_time.tv_sec + ((double) end_time.tv_usec / 1000000);

    std::cout.precision(15);
    std::cout << "1P-1EP-PIPELINE performance: ";
    std::cout << (loopnum * 1.0) / (end - start)
     << " ops/secs" << std::endl;


}

