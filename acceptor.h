#ifndef acceptorH
#define acceptorH

#include "def.h"


void acceptor_on_accept(SOCKET sock);
void acceptor_on_recv(SOCKET sock);

void acceptor_process(SOCKET listener);
void acceptor_closesocket(SOCKET sock);
void acceptor_close(void);

#endif
