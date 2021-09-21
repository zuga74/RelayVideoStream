#ifndef moreH
#define moreH

#include "def.h"

void Dbg(const char * Format, ...);

//поиск в указанной области памяти p, блока памяти s
char * memsearch(const char * p, unsigned long plen, const char * s, unsigned long slen);

/**
int get_http_header_len(char * buf, int len, std::string Delimeter);
*/
char * trim(char * str);

int get_http_header_len(char * buf, int len, const char * delimeter);

/*
typedef std::pair<std::string, key_values_t> http_headers_t;
http_headers_t parse_http_header(char * buf, int len);



#define KEY_VAL_SIZE   128
typedef struct _KEY_VAL
{
  char key[KEY_VAL_SIZE];
  char val[KEY_VAL_SIZE];
} KEY_VAL;

#define STARTING_LINE_SIZE   128
#define HEADERS_COUNT   20
typedef struct _PARSE_HTTP_HEADER
{
  char starting_line[STARTING_LINE_SIZE];
  KEY_VAL headers[HEADERS_COUNT];
} PARSE_HTTP_HEADER, * PPARSE_HTTP_HEADER;


int parse_http_header(char * buf, int len, PPARSE_HTTP_HEADER pheader);

*/

bool parse_url(char * url, char * sheme, char * host, char * path, unsigned short * port);

//из addr->s_addr т.е. www.ya.ru->u_int
bool get_s_addr( char *addr, unsigned long *ps_addr );


SOCKET socket_create_and_connect(char * host, unsigned short port);

SOCKET socket_create_bind_listen(unsigned long ip, unsigned short port);

#ifndef _WIN32

bool kbhit();

#endif

#endif
