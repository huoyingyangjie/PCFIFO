//
// Created by jiey on 3/22/17.
//

#ifndef ACEXGE_KFIFO_H
#define ACEXGE_KFIFO_H
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define is_power_of_2(x)        ((x) != 0 && (((x) & ((x) - 1)) == 0))

inline int fls(int x)
{
    int r;


    __asm__("bsrl %1,%0\n\t"
            "jnz 1f\n\t"
            "movl $-1,%0\n"
            "1:" : "=r" (r) : "rm" (x));
    return r+1;
}

inline unsigned int roundup_pow_of_two(unsigned int x)
{
    return 1UL << fls(x - 1);
}


struct kfifo_t {
    uint8_t *buffer;
    uint32_t size;
    uint32_t in;
    uint32_t out;
};


class KFIFO {
public:
    KFIFO(uint32_t buffer_size) {
        int ret;
        this->fifo.size = buffer_size;
        ret = this->init();
        if (ret)
            throw ;
    };

    ~KFIFO() {
        free(this->fifo.buffer);
    }

public:
    //TODO:remove asm and set inline
    uint32_t fifo_put(uint8_t *buffer, uint32_t len);

    uint32_t fifo_get(uint8_t *buffer, uint32_t len);

private:

    int init();


private:
    kfifo_t fifo;

};
#endif //ACEXGE_KFIFO_H
