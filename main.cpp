#include "def.h"
#include "more.h"
#include "acceptor.h"
#include "ringbuf.h"
#include "reader.h"


const char HTTP_OK[] = "HTTP/1.1 200 OK\r\nServer: Astra\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nContent-Type: application/octet-stream\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
const char HTTP_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\nServer: Astra\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nContent-Type: text/plain\r\nAccess-Control-Allow-Origin: *\r\nContent-Encoding: identity\r\nContent-Length: 9\r\nConnection: close\r\n\r\nNot Found";

bool terminated;

bool out_thread_execute;
int out_thread_cnt;

#define MAX_URL 10
char urls[MAX_URL][255];
int urls_cnt;

char path[255];

bool in_thread_execute;

// ------------------------ In -------------------------------------------------

void reader_recv(char * buf, int len)
{
  //RINGBUF_NUM_TYPE num =
  ringbuf_put(buf, len);
  //Dbg("Relay2: In ringbuf_put num=%d len=%d\r\n", (int)num, len);
  //printf("I:%d\r\n", (int)num);
}

void InProcess(char * url)
{
  SOCKET sock = reader_init(url);
  if (!IS_VALID_SOCKET(sock)) return;

  while (in_thread_execute)
  {
    if (!reader_process(sock)) break;
    SLEEP(1);
  }

  CLOSESOCKET(sock);
}


THREAD_FUNCTION(InThread)
{
  int i;


  Dbg("Relay2: In begin\r\n");

  for (i = 0; i < urls_cnt; ++i)  {
      if (!in_thread_execute) break;
      Dbg("Relay2: In process begin url: %s\r\n", urls[i]);
      InProcess(urls[i]);
      Dbg("Relay2: In process end url: %s\r\n", urls[i]);
  }

  Dbg("Relay2: In end\r\n");
  in_thread_execute = false;
  out_thread_execute = false;
  ringbuf_zero();

  return 0;
}

// ------------------------ Out ------------------------------------------------

THREAD_FUNCTION(OutThread)
{
  SOCKET sock = *(SOCKET *)lpParam;
  int len, rc;
  RINGBUF_NUM_TYPE num = 1;
  fd_set readset, writeset;
  timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 1000;
  RINGBUF_NUM_TYPE next_num;
  char buf[MAX_PACKET_SIZE];


  Dbg("Relay2: Out begin sock=%d\r\n", sock);


  send(sock, HTTP_OK, strlen(HTTP_OK), 0);


  while (out_thread_execute)
  {
    //if (in_thread_execute) break;

    FD_ZERO(&readset);
    FD_ZERO(&writeset);
    FD_SET(sock, &readset);

    rc = select(FD_SETSIZE, &readset, &writeset,  NULL, &timeout);
    if (rc > 0) {
      if (FD_ISSET(sock, &readset)) {
        rc = recv(sock, buf, MAX_PACKET_SIZE, MSG_PEEK);
        if (rc <= 0) {
          Dbg("Relay2: Out disconnect sock=%d\r\n", sock);
          break;   //отсоединен
        }
      }
      if (FD_ISSET(sock, &writeset)) {
        SLEEP(1);
        continue;
      }
    }


    len = ringbuf_get(num, buf, MAX_PACKET_SIZE); //ищем необходимый пакет с номером num
    if (len <= 0) {
      next_num = ringbuf_get_next_num(num);
      if (next_num == RINGBUF_MAX_NUM) { SLEEP(10); continue; }  //новых покетов пока нет
      len = ringbuf_get(next_num, buf, MAX_PACKET_SIZE); //ищем необходимый пакет с номером num
      num = next_num;
      if (len <= 0) {
        continue;
      }
    }


    rc = send(sock, buf, len, 0);
    if (rc != len) {
        Dbg("Relay2: Out not send sock=%d rc=%d len=%d\r\n", sock, rc, len);
        break;
    }

    //Dbg("Relay2: Out send sock=%d num=%d len=%d\r\n", sock, (int)num, len);
    //printf("O:%d:%d\r\n", sock, (int)num);

    num++;
    //SLEEP(1);  //мешает не нужен
  }

  CLOSESOCKET(sock);

  if (out_thread_cnt > 0) out_thread_cnt--;
  if (out_thread_cnt == 0) in_thread_execute = false;

  Dbg("Relay2: Out end sock=%d out_thread_cnt=%d\r\n", sock, out_thread_cnt);

  return 0;
}

// ------------------------ Acceptor -------------------------------------------

void acceptor_on_accept(SOCKET sock)
{
  Dbg("Relay2: new client connected socket=%d\r\n", sock);
}

static SOCKET msock;

void acceptor_on_recv(SOCKET sock)
{
  THREAD_HANDLE thread;

  int http_header_len, http_starting_line_len;
  char buf[MAX_PACKET_SIZE];

  int rc = recv(sock, buf, MAX_PACKET_SIZE, 0);
  if (rc <= 0) {
    Dbg("Relay2: client close connection sock=%d\r\n", sock);
    acceptor_closesocket(sock);
  } else {
    Dbg("Relay2: recive client data sock=%d len=%d\r\n", sock, rc);
    if ( memcmp(buf, "GET ", 4) != 0 ) { acceptor_closesocket(sock); return; }

    http_header_len = get_http_header_len(buf, rc, "\r\n\r\n");
    if (http_header_len == 0) http_header_len = get_http_header_len(buf, rc, "\n\n");
    if (http_header_len == 0) { acceptor_closesocket(sock); return; }

    http_starting_line_len = get_http_header_len(buf, http_header_len, "\n");
    if ( memsearch(buf, http_starting_line_len, path, strlen(path)) == NULL ) {
        send(sock, HTTP_NOT_FOUND, strlen(HTTP_NOT_FOUND), 0);
        acceptor_closesocket(sock);
        return;
    }

    if (!in_thread_execute) {
      in_thread_execute = true;
      CREATE_THREAD(thread, InThread, NULL);
    }

    out_thread_execute = true;
    msock = sock;
    CREATE_THREAD(thread, OutThread, &msock);
    ++out_thread_cnt;
    acceptor_closesocket(sock);
  }
}


#ifndef _WIN32


void signal_handler(int signum) {
   printf("Relay2: terminating\r\n");
   terminated = true;
   //exit(signum);
}
#endif

int main(int argc, char* argv[])
{

#ifdef _WIN32
  WSADATA wsadata;
  WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif


  if (argc < 5) {
    printf("relay 2\r\n");
    printf("USE:\r\n");
    printf("relay2 port path no_client_timeout_sec url1 url2 url3 ....\r\n");
    printf("path - path string, no_client_timeout_sec - terminated if no clent (0 - no terminated), url1, url2, url3 - url to alternative video stream\r\n");
    printf("EXAMPLE:\r\n");
    printf("relay2 8000 1BEFC98754AM 60 http://host1.com/video http://host2.com/video\r\n");
    printf("use video stream on http://localhost:8000/1BEFC98754AM\r\n");
    return -1;
  }

  unsigned short port = (unsigned short)atoi(argv[1]);
  if (port == 0) {
    printf("port error\r\n");
    return -1;
  }

  strcpy(path, argv[2]);

  int no_client_timeout_sec = atoi(argv[3]);

  urls_cnt = argc - 4;
  if (urls_cnt > MAX_URL) urls_cnt = MAX_URL;
  for (int i = 0; i < urls_cnt; i++) {
    strcpy(urls[i], argv[i + 4]);
  }

  terminated = false;

#ifndef _WIN32
  struct sigaction sa;
  sigset_t newset;
  sigemptyset(&newset);   // чистимся
  sigaddset(&newset, SIGHUP);  // добавляем сигнал SIGHUP
  sigprocmask(SIG_BLOCK, &newset, 0);   // блокируем его
  sa.sa_handler = signal_handler;  // указываем обработчик
  sigaction(SIGTERM, &sa, 0);    // обрабатываем сигнал SIGTERM
#endif

  ringbuf_zero();


  out_thread_execute = true;
  out_thread_cnt = 0;
  in_thread_execute = false;

  SOCKET listener = socket_create_bind_listen(INADDR_ANY, port);
  if (listener == SOCKET_ERROR) { printf("Relay2: create listener error\r\n"); return 0; }

  Dbg("Relay2: create listener on 0.0.0.0:%d\r\n", port);

  time_t time_to_terminated = time(NULL);

  while (!terminated)
  {
    acceptor_process(listener);
    //if (kbhit()) terminated = true;
    if (no_client_timeout_sec > 0) {
      if (out_thread_cnt > 0) time_to_terminated = time(NULL);
      else if (time(NULL) - time_to_terminated > no_client_timeout_sec) {
        Dbg("Relay2: Clients no more time - terminited\r\n");
        break; //если клиентов нет больше no_client_timeout_sec
      }
    }
  }

  CLOSESOCKET(listener);
  acceptor_close();

  in_thread_execute = false;
  out_thread_execute= false;
  SLEEP(200);

  Dbg("Relay2: exit\r\n", port);
#ifdef _WIN32
  WSACleanup();
#endif
  return 0;
}




