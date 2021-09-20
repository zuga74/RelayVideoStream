#include "reader.h"
#include "def.h"
#include "more.h"
//#include "ringbuf.h"


SOCKET reader_init(char * url)
{
  char sheme[64];
  char host[128];
  char path[256];
  unsigned short port;
  fd_set readset;
  timeval timeout;
  SOCKET sock;
  int rc;
  char buf[MAX_PACKET_SIZE];
  int http_header_len;

  if (!parse_url(url, sheme, host, path, &port)) return SOCKET_ERROR;
  if (port == 0) port = 80;
  sock = socket_create_and_connect(host, port);
  if (!IS_VALID_SOCKET(sock)) return SOCKET_ERROR;
  snprintf(buf, MAX_PACKET_SIZE, "GET %s HTTP/1.1\r\nHost: %s:%d\r\nUser-Agent: Mozilla/5.0\r\nRange: bytes=0-\r\nConnection: close\r\nIcy-MetaData: 1\r\n\r\n", path, host, port);
  rc = send(sock, buf, strlen(buf), 0);
  if (rc != (int)strlen(buf)) { CLOSESOCKET(sock); return SOCKET_ERROR; }

  FD_ZERO(&readset);
  FD_SET(sock, &readset);

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  if (select(FD_SETSIZE, &readset, NULL, NULL, &timeout) == SOCKET_ERROR) { CLOSESOCKET(sock); return SOCKET_ERROR; }

  if (!FD_ISSET(sock, &readset)) { CLOSESOCKET(sock); return SOCKET_ERROR; }


  rc = recv(sock, buf, MAX_PACKET_SIZE, 0);
  if (rc <= 0 ) { CLOSESOCKET(sock); return SOCKET_ERROR; }

  if (memcmp(buf, "HTTP/", 5) != 0) { CLOSESOCKET(sock); return SOCKET_ERROR; }
  if (memcmp(buf + 8, " 20", 3) != 0) { CLOSESOCKET(sock); return SOCKET_ERROR; }


  http_header_len = get_http_header_len(buf, rc, "\r\n\r\n");
  if (http_header_len == 0) http_header_len = get_http_header_len(buf, rc, "\n\n");
  if (http_header_len == 0) { CLOSESOCKET(sock); return SOCKET_ERROR; }

  if (rc > http_header_len) reader_recv(buf + http_header_len, rc - http_header_len);

  return sock;
}

bool reader_process(SOCKET sock)
{
  fd_set readset;
  timeval timeout;
  int rc;
  char buf[MAX_PACKET_SIZE];

  FD_ZERO(&readset);
  FD_SET(sock, &readset);

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  if (select(FD_SETSIZE, &readset, NULL, NULL, &timeout) == SOCKET_ERROR) return false;

  if (!FD_ISSET(sock, &readset)) return false;

/*
  int minindex;

  while (true) {
    minindex = ringbuf_get_min_index();
    if (ringbuf[minindex].busy) { SLEEP(1); continue; }
    else break;
  }


  ringbuf[minindex].busy = true;
  rc = recv(sock, ringbuf[minindex].buf, MAX_PACKET_SIZE, 0);
  if (rc > 0) {

    ringbuf_max_num++;
    if (ringbuf_max_num == RINGBUF_MAX_NUM) {
      ringbuf_zero();
      return true;
    }
    ringbuf[minindex].len = rc;
    ringbuf[minindex].num = ringbuf_max_num;

  }
  ringbuf[minindex].busy = false;
  if (rc <= 0) return false;
*/

  rc = recv(sock, buf, MAX_PACKET_SIZE, 0);
  if (rc <= 0 ) return false;
  reader_recv(buf, rc);

  return true;
}

