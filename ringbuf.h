#ifndef ringbufH
#define ringbufH

#include "def.h"

typedef unsigned long long RINGBUF_NUM_TYPE;

typedef struct _RINGBUF {
        RINGBUF_NUM_TYPE num;
        char buf[MAX_PACKET_SIZE];
        int len;
        bool busy;
} RINGBUF, *PRINGBUF;

#define RINGBUF_CNT 100
#define RINGBUF_MAX_NUM RINGBUF_NUM_TYPE(~0)


//extern RINGBUF ringbuf[RINGBUF_CNT];
//extern RINGBUF_NUM_TYPE ringbuf_max_num;


void ringbuf_zero(void);
//int ringbuf_get_min_index(void);
//RINGBUF_NUM_TYPE ringbuf_get_min_num(void);
RINGBUF_NUM_TYPE ringbuf_put(char * buf, int len);
//int ringbuf_get_index_by_num(RINGBUF_NUM_TYPE num);
int ringbuf_get(RINGBUF_NUM_TYPE num, char * buf, int buf_len);
RINGBUF_NUM_TYPE ringbuf_get_next_num(RINGBUF_NUM_TYPE num);



#endif
