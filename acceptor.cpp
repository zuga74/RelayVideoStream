#include "acceptor.h"
#include "more.h"

SOCKET acceptor_client_sockets[FD_SETSIZE];
int acceptor_client_sockets_cnt = 0;


void acceptor_close(void)
{
   int i;
   
   for (i = 0; i < acceptor_client_sockets_cnt; ++i) {
     CLOSESOCKET(acceptor_client_sockets[i]);
   }
   acceptor_client_sockets_cnt = 0;
}

void acceptor_closesocket(SOCKET sock)
{
   int i;

   for (i = 0; i < acceptor_client_sockets_cnt; ++i) {
     if (sock == acceptor_client_sockets[i]) {
       if (acceptor_client_sockets_cnt > 1) {
         acceptor_client_sockets[i] = acceptor_client_sockets[acceptor_client_sockets_cnt - 1];
       }
       acceptor_client_sockets_cnt--;
       break;
     }
   }
}


void acceptor_process(SOCKET listener)
{

  SOCKET sock;
  timeval timeout;
  fd_set readset;
  int i, rc;

  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;



  FD_ZERO(&readset);
  FD_SET(listener, &readset);

  for (i = 0; i < acceptor_client_sockets_cnt; ++i) {
      FD_SET(acceptor_client_sockets[i], &readset);
  }


  rc = select(FD_SETSIZE, &readset, NULL, NULL, &timeout);
  if ( (rc == SOCKET_ERROR) || (rc <= 0) ) {
      SLEEP(100);
      return;
  }

  if (FD_ISSET(listener, &readset))
  {
    if (acceptor_client_sockets_cnt < FD_SETSIZE) {
      sock = accept(listener, NULL, NULL);
      if (sock != INVALID_SOCKET) {
        acceptor_client_sockets[acceptor_client_sockets_cnt] = sock;
        acceptor_client_sockets_cnt++;
        acceptor_on_accept(sock);
      }
    } //else printf("client broken\r\n");
  }


    //клиенты
  for (i = 0; i < acceptor_client_sockets_cnt; i++)
  {
    sock = acceptor_client_sockets[i];
    if (!FD_ISSET(sock, &readset)) continue;
    acceptor_on_recv(sock);
  }

}







