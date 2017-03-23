//
// Created by root on 3/22/17.
//

#include "kfifo.h"
#include <algorithm>
int  KFIFO::init() {

    if (!is_power_of_2(this->fifo.size)) {
        this->fifo.size = roundup_pow_of_two(this->fifo.size);
    }
    fifo.buffer = (uint8_t *) malloc(this->fifo.size);
    if (!fifo.buffer)
        return -1;
    fifo.in = fifo.out = 0;
    return 0;

}


uint32_t KFIFO::fifo_put(uint8_t *buffer, uint32_t len) {
    //printf("INFO:fifo_put\n");
    uint32_t l;
    len = std::min(len, fifo.size - fifo.in + fifo.out);

    //__asm__ __volatile__("mfence":::"memory");


    l = std::min(len, fifo.size - (fifo.in & (fifo.size - 1)));
    memcpy(fifo.buffer + (fifo.in & (fifo.size - 1)), buffer, l);
    memcpy(fifo.buffer, buffer + l, len - l);


    //__asm__ __volatile__("sfence"::: "memory");

    fifo.in += len;

    return len;


}


uint32_t KFIFO::fifo_get(uint8_t *buffer, uint32_t len) {
    //printf("INFO:fifo_get\n");
    uint32_t l;
    len = std::min(len, fifo.in - fifo.out);
    //__asm__ __volatile__("lfence":::"memory");
    l = std::min(len, fifo.size - (fifo.out & (fifo.size - 1)));
    memcpy(buffer, fifo.buffer + (fifo.out & (fifo.size - 1)), l);
    memcpy(buffer + l, fifo.buffer, len - l);
    //__asm__ __volatile__("mfence":::"memory");
    fifo.out += len;
    return len;
}
