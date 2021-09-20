#include "ringbuf.h"


RINGBUF ringbuf[RINGBUF_CNT];
RINGBUF_NUM_TYPE ringbuf_max_num;
int put_index;

//------------------  CLEAR ----------------------------------------------------

void ringbuf_zero(void)
{
   for (int i = 0; i < RINGBUF_CNT; ++i) {
     ringbuf[i].num = 0;
     ringbuf[i].len = 0;
     ringbuf[i].busy = false;
   }
   ringbuf_max_num = 0;
   put_index = 0;
}


//------------------------ PUT -------------------------------------------------
/*
int ringbuf_get_min_index(void)
{
  int minindex;
  RINGBUF_NUM_TYPE minnum;

  while (ringbuf[0].busy) SLEEP(1);
  minnum = ringbuf[0].num;
  minindex = 0;

  for (int i = 1; i < RINGBUF_CNT; ++i) {
    while (ringbuf[i].busy) SLEEP(1);
    if (ringbuf[i].num < minnum) {
      minnum = ringbuf[i].num;
      minindex = i;
    }
  }

  return minindex;

}

RINGBUF_NUM_TYPE ringbuf_put(char * buf, int len)
{
  int minindex;

  while (true) {
    minindex = ringbuf_get_min_index();
    if (ringbuf[minindex].busy) { SLEEP(1); continue; }
    else break;
  }

  ringbuf_max_num++;
  if (ringbuf_max_num == RINGBUF_MAX_NUM) {
      ringbuf_zero();
      minindex = 0;
  }

  ringbuf[minindex].busy = true;
  len = len <  MAX_PACKET_SIZE ? len :  MAX_PACKET_SIZE;
  memcpy(ringbuf[minindex].buf, buf, len);
  ringbuf[minindex].len = len;
  ringbuf[minindex].num = ringbuf_max_num;
  ringbuf[minindex].busy = false;

  return ringbuf_max_num;
}
*/

RINGBUF_NUM_TYPE ringbuf_put(char * buf, int len)
{
  ringbuf_max_num++;
  if (ringbuf_max_num == RINGBUF_MAX_NUM) {
      ringbuf_zero();
  }

  while (ringbuf[put_index].busy) { SLEEP(1); continue; }

  ringbuf[put_index].busy = true;
  len = len <  MAX_PACKET_SIZE ? len :  MAX_PACKET_SIZE;
  memcpy(ringbuf[put_index].buf, buf, len);
  ringbuf[put_index].len = len;
  ringbuf[put_index].num = ringbuf_max_num;
  ringbuf[put_index].busy = false;

  put_index++;
  if (put_index >= RINGBUF_CNT) put_index = 0;


  return ringbuf_max_num;
}


//------------------------ GET -------------------------------------------------

int ringbuf_get_next_index(RINGBUF_NUM_TYPE num)
{
  RINGBUF_NUM_TYPE d, min_d = RINGBUF_MAX_NUM;

  int index = -1;

  for (int i = 0; i < RINGBUF_CNT; ++i) {
    while (ringbuf[i].busy) SLEEP(1);
    if (ringbuf[i].num > num) {
      d = ringbuf[i].num - num;
      if (d < min_d) {
         index = i;
         min_d = d;
      }
    }
  }

  return index;
}

RINGBUF_NUM_TYPE ringbuf_get_next_num(RINGBUF_NUM_TYPE num)
{
  int index;


  while (true) {
    index = ringbuf_get_next_index(num);
    if (index == -1) return RINGBUF_MAX_NUM;
    if (ringbuf[index].busy) { SLEEP(1); continue; }
    else break;
  }

  return ringbuf[index].num;
}




/*
RINGBUF_NUM_TYPE ringbuf_get_min_num(void)
{
  int minindex;

  while (true) {
    minindex = ringbuf_get_min_index();
    if (ringbuf[minindex].busy) { SLEEP(1); continue; }
    else break;
  }

  return ringbuf[minindex].num;
}
*/



int ringbuf_get_index_by_num(RINGBUF_NUM_TYPE num)
{
   for (int i = 0; i < RINGBUF_CNT; ++i) {
     while (ringbuf[i].busy) SLEEP(1);
     if (ringbuf[i].num == num) return i;
   }
   return -1;
}


int ringbuf_get(RINGBUF_NUM_TYPE num, char * buf, int buf_len)
{

  int index, len;


  while (true) {
    index = ringbuf_get_index_by_num(num);
    if (index == -1) return -1;

    if (ringbuf[index].busy) { SLEEP(1); continue; }
    else break;
  }

  if (ringbuf[index].len == 0) return 0;

  ringbuf[index].busy = true;
  len = ringbuf[index].len;
  memcpy(buf, ringbuf[index].buf, len);
  ringbuf[index].busy = false;

  return len;
}




