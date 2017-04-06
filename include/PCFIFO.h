//
// Created by root on 3/22/17.
//

#ifndef ACEXGE_PCFIFO_H
#define ACEXGE_PCFIFO_H


#include "kfifo.h"
#include <stdint.h>
#include <array>
#include <thread>
#include <pthread.h>
#include <stdio.h>
#include <atomic>
enum ClaimStrategyOption {
    kSingleThreadedStrategy,
    kMultiThreadedStrategy
};

constexpr uint64_t  PointerSize = sizeof(void*);

struct   EVENT{
    void  * t;
    uint64_t sequence;
    uint64_t ringbufferID;
};





template <typename T >
class RingBuffer{
public:
    RingBuffer(size_t events_size,uint64_t ringbufferID ,std::atomic<uint64_t> * sq):p2c_fifo(events_size*PointerSize),c2p_fifo(events_size*PointerSize),sequence(sq){

        events=new EVENT[events_size];
        Tevents=new T[events_size];
        for( size_t i=0;i < events_size;++i)
        {
            events[i].t= &Tevents[i];
            events[i].sequence=0;
            events[i].ringbufferID=ringbufferID;
        }
        //fill c2p ;
        for(size_t i=0;i< events_size;++i)
        {
            c2p_fifo.fifo_put((uint8_t*)&events[i],sizeof(void*));
        }


    }
    ~RingBuffer(){
        delete[](events);
        delete[](Tevents);
}

public:
    //
    //
    //return value: >0 success ,=0 failed
    int  producer_put_event_try(T * t){
        return p2c_fifo.fifo_put((uint8_t*)(&t),sizeof(t));
        //int ret=p2c_fifo.fifo_put((uint8_t*)(&t),sizeof(t));
        //return ret?((EVENT*)t)->sequence=(++(*this->sequence)):0;

    }
    //return value: the same to above

    int  producer_put_event_wait(T *t)
    {
        while (!p2c_fifo.fifo_put((uint8_t*)(&t),sizeof(t)))
        {
            std::this_thread::yield();
        }
        //((EVENT*)t)->sequence=(++(*this->sequence));
        return  sizeof(void*);
    }

    T *  producer_get_event_try()
    {
         T * t;
        return  c2p_fifo.fifo_get((uint8_t*)&t,sizeof(t))?t:NULL;

    }
    T *  producer_get_event_wait()
    {
        T * t;
        while (!c2p_fifo.fifo_get((uint8_t*)&t,sizeof(t)))
        {
            std::this_thread::yield();
        }
        return t;
    }
    int consumer_put_event_try(T * t)
    {
        return c2p_fifo.fifo_put((uint8_t*)(&t),sizeof(t));
        //int ret;
        //ret=c2p_fifo.fifo_put((uint8_t*)(&t),sizeof(t));
        //return ret?((EVENT*)t)->sequence=(++(*this->sequence)):0;
    }
    int consumer_put_event_wait(T *t)
    {
        while(!c2p_fifo.fifo_put((uint8_t*)(&t),sizeof(t))){
            std::this_thread::yield();
        }
        //((EVENT*)t)->sequence=(++(*this->sequence));
        return sizeof(void*);
    }
    T * consumer_get_event_try()
    {
        T * t;
        return p2c_fifo.fifo_get((uint8_t*)&t,sizeof(t))?t:NULL;
    }
    T * consumer_get_event_wait()
    {
        T * t;
        while (!p2c_fifo.fifo_get((uint8_t*)&t,sizeof(t)))
        {
            std::this_thread::yield();
        }
        return t;
    }


private:

    EVENT * events;
    T * Tevents;
    KFIFO p2c_fifo;
    KFIFO c2p_fifo;
    std::atomic<uint64_t > * sequence;


};
template <typename  T>
class Producer{
public:
    Producer(RingBuffer<T> * rb):ringBuffer(rb){};
public:
    T *get_event_try()
    {
        return ringBuffer->producer_get_event_try();
    }
    T *get_event_wait(){
        return ringBuffer->producer_get_event_wait();
    }
    int put_event_try(T *t)
    {
        return ringBuffer->producer_put_event_try(t);
    }
    int put_event_wait(T *t)
    {
        return ringBuffer->producer_put_event_wait(t);
    }
private:
    RingBuffer<T> * ringBuffer;
};
template <typename  T>
class Consumer{
public:
    Consumer(RingBuffer<T> *rb):ringBuffer(rb){};
public:
    T *get_event_try()
    {
        return ringBuffer->consumer_get_event_try();
    }
    T *get_event_wait(){
        return ringBuffer->consumer_get_event_wait();
    }
    int put_event_try(T *t)
    {
        return ringBuffer->consumer_put_event_try(t);
    }
    int put_event_wait(T *t)
    {
        return ringBuffer->consumer_put_event_wait(t);
    }
private:
    RingBuffer<T> * ringBuffer;
};
template <typename T>
struct PCGroup{
    PCGroup(RingBuffer<T> * ringBuffer):producer(ringBuffer),consumer(ringBuffer)
    {

    }
    Producer<T> producer;
    Consumer<T> consumer;
};
#define  MAX_THREAD 1024
template <typename T>
class PCFIFO{
public:
    PCFIFO():pcgroups_num(0),sequence(0){
        mutex_create_pc=PTHREAD_MUTEX_INITIALIZER;

    }
    ~PCFIFO(){
        pthread_mutex_destroy(&this->mutex_create_pc);
        for(int i=0;i<pcgroups_num;++i)
        {
            delete(pcgroups[pcgroups_num]);
            delete(rbs[pcgroups_num]);
        }
    }

public:
    PCGroup<T> * create_one_pc(size_t events_size=1024)
    {
        pthread_mutex_lock(&mutex_create_pc);
        if(pcgroups_num==MAX_THREAD)
            return NULL;
        RingBuffer<T> * ringBuffer=new RingBuffer<T>(events_size,pcgroups_num,&sequence);
        rbs[pcgroups_num]=ringBuffer;
        PCGroup<T> *pcGroup=new PCGroup<T>(ringBuffer);
        pcgroups[pcgroups_num]=pcGroup;
        ++pcgroups_num;
        pthread_mutex_unlock(&mutex_create_pc);

        return (PCGroup<T> *)pcGroup;
    }


private:

    pthread_mutex_t mutex_create_pc;
    PCGroup<T> *  pcgroups[MAX_THREAD];
    RingBuffer<T> * rbs[MAX_THREAD];
    uint64_t pcgroups_num;
    std::atomic<uint64_t> sequence;
};

#endif //ACEXGE_PCFIFO_H
