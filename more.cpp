#include "more.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>



void Dbg(const char * Format, ...)
{
  va_list argptr;

  va_start(argptr, Format);
  vprintf(Format, argptr);
  va_end(argptr);
}


//поиск в указанной области памяти p, блока памяти s
char * memsearch(const char * p, unsigned long plen, const char * s, unsigned long slen)
{
  char * cp = (char *)p;
  char * result = NULL;

  if (slen == 0) return NULL;

  while ( (cp = (char *)memchr(cp, s[0], plen + (unsigned long)(p - cp) )) != NULL )
  {
    if ( cp + slen > p + plen )
      break;
    else if ( memcmp(cp, s, slen) == 0 )
    {
      result = cp;
      break;
    }
    else cp++;
  }
  return result;
}



char * trim(char * str)
{
  char * res = NULL;
  int i;

  for (i = 0; i < (int)strlen(str); ++i) {
    if ( str[i] > ' ') {
      res = str + i;
      break;
    }
  }

  if (res == NULL) return res;

  int len = strlen(str) - (res - str);

  if (len <= 0) return NULL;


  for (i = len - 1 ; i > -1; --i) {
    if ( str[i] > ' ') {
      len = str + i  + 1 - res;
      str[len] = '\0';
      break;
    }
  }

  return res;
}


int get_http_header_len(char * buf, int len, const char * delimeter)
{
  char * cp;

  if ( NULL != (cp = memsearch(buf, len, delimeter, strlen(delimeter)) ) ) return cp - buf + strlen(delimeter);
  return 0;
}

/*
http_headers_t parse_http_header(char * buf, int len)
{
  int http_header_len, l, str_len;
  char r_n[3], * ptr, * ptr_end, * pv;
  std::string starting_line, key, value;
  key_values_t headers;

  http_header_len = get_http_header_len(buf, len, "\r\n\r\n");
  if (http_header_len) {
    strcpy(r_n, "\r\n");
  }
  else {
    http_header_len = get_http_header_len(buf, len, "\n\n");
    if (http_header_len) strcpy(r_n, "\n");
    else return make_pair(starting_line, headers);
  }

  if ( NULL == (ptr = memsearch(buf, http_header_len, r_n, strlen(r_n)) ) ) return make_pair(starting_line, headers);
  starting_line.assign(buf, ptr - buf);

  ptr += strlen(r_n);
  l = buf + http_header_len - ptr;
  if (l <= 0) {
    return make_pair(starting_line, headers);
  }

  while ( (l > 0) && ( NULL != (ptr_end = memsearch(ptr, l, r_n, strlen(r_n)))) )
  {
     str_len = ptr_end - ptr;

     if ((pv = memsearch(ptr, str_len, ": ", 2)) != NULL) {
       key.assign(ptr, pv - ptr);
       pv += 2;
       value.assign(pv, str_len - (pv - ptr));
       headers.push_back(make_pair(key, value));
     }

     l -= (str_len + strlen(r_n));
     ptr = ptr_end + strlen(r_n);
  }

  return make_pair(starting_line, headers);
}


int parse_http_header(char * buf, int len, PPARSE_HTTP_HEADER pheader)
{
  PARSE_HTTP_HEADER res;
  int http_header_len, str_len, l;
  char * cp, * cpend, *pv;
  char r_n[3];

  http_header_len = get_http_header_len(buf, len, "\r\n\r\n");

  if (http_header_len) {
    strcpy(r_n, "\r\n");
  }
  else {
    http_header_len = get_http_header_len(buf, len, "\n\n");
    if (http_header_len) strcpy(r_n, "\n");
    else return -1;
  }


  if ( NULL == (cp = memsearch(buf, http_header_len, r_n, strlen(r_n)) ) ) return -1;
  memset(pheader->starting_line, 0, STARTING_LINE_SIZE);
  memcpy(pheader->starting_line, buf, cp - buf);

  cp += strlen(r_n);
  l = buf + http_header_len - cp;
  if (l == 0) return 0;


  int cnt = 0;
  while ( (l > 0) && ( NULL != (cpend = memsearch(cp, l, r_n, strlen(r_n)))) )
  {
     str_len = cpend - cp;

     if ((pv = memsearch(cp, str_len, ": ", 2)) != NULL) {
       memset(&pheader->headers[cnt], 0, sizeof(KEY_VAL));
       memcpy(pheader->headers[cnt].key, cp, pv - cp);
       pv += 2;
       memcpy(pheader->headers[cnt].val, pv, str_len - (pv - cp));
       cnt++;
       if (cnt == HEADERS_COUNT) break;
     }

     l -= (str_len + strlen(r_n));
     cp = cpend + strlen(r_n);
  }

  return cnt;
}
*/

bool parse_url(char * url, char * sheme, char * host, char * path, unsigned short * port)
{
  char * pch = url;
  char * pche, * pchb, * pcht;
  int l;
  int len = l = strlen(url);

  sheme[0] = 0;
  host[0] = 0;
  path[0] = 0;
  *port = 0;

  if ( NULL != (pch = memsearch(url, l, "://", 3)) )
  {
     memcpy(sheme, url, pch - url);
     sheme[pch - url] = 0;
     pch += 3;
     l = len - (pch - url);
  } else pch = url;

  if ( NULL == (pche = memsearch(pch, l, "/", 1)) )
    pche = pch + l;

  pchb = pche;
  if ( NULL != (pcht = memsearch(pch, pche - pch, ":", 1)) )
  {
    *port = atoi(pcht + 1);
    pche = pcht;
  }
  else *port = 0;

  memcpy(host, pch, pche - pch);
  host[pche - pch] = 0;
   pch = pchb;
  l = len - (pch - url);

  memcpy(path, pch, l);
  path[l] = 0;
  if (strlen(path) == 0) { path[0] = '/'; path[1] = 0; }

  return true;
}

//из addr->s_addr т.е. www.ya.ru->u_int
bool get_s_addr( char *addr, unsigned long *ps_addr )
{
  struct hostent *hp;



  if (strcmp(addr, "localhost") == 0)
  {
    *ps_addr = inet_addr("127.0.0.1");
    return true;
  }

  if ((*ps_addr = inet_addr(addr)) != INADDR_NONE) return true;


  if((hp = gethostbyname(addr)) != NULL)
  {
     *ps_addr = (( struct in_addr * )hp->h_addr)->s_addr;
     return true;
  }


  return false;
}



SOCKET socket_create_and_connect(char * host, unsigned short port)
{
  SOCKET sock;
  struct sockaddr_in addr;
  unsigned long ss_addr;

  memset(&addr, 0, sizeof(addr));

  if ( !get_s_addr( host, &ss_addr ) ) return SOCKET_ERROR;

  addr.sin_addr.s_addr = ss_addr;

  if (SOCKET_ERROR == (sock = socket(AF_INET, SOCK_STREAM, 0)) ) return SOCKET_ERROR;

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (SOCKET_ERROR == connect(sock, (sockaddr*)&addr, sizeof(addr)) ) //Конектимся к требуемому адресу
  {
    CLOSESOCKET(sock);
    return SOCKET_ERROR;
  }

  return sock;
}


SOCKET socket_create_bind_listen(unsigned long ip, unsigned short port)
{
  SOCKET sock;
  struct sockaddr_in addr;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = ip;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (!IS_VALID_SOCKET(sock)) {
  	return SOCKET_ERROR;
  }

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        CLOSESOCKET(sock);
  	return SOCKET_ERROR;
  }

  if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        CLOSESOCKET(sock);
  	return SOCKET_ERROR;
  }

  return sock;
}

#ifndef _WIN32

bool kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}

#endif
