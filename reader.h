//---------------------------------------------------------------------------

#ifndef readerH
#define readerH

#include "def.h"

void reader_recv(char * buf, int len);

SOCKET reader_init(char * url);
bool reader_process(SOCKET sock);

#endif
