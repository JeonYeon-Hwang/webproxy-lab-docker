#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void doit(int connfd, char *host, char *port);
void parse_uri(char *uri, char *hostname, char *path, char *port);
void generate_header(char *bufs, char *hostname, char *port, char *path);
    
int main(int argc, char **argv)
{
  printf("%s", user_agent_hdr);
  
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];

  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  if(argc != 2){
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /*클라이언트로부터 연결 수락 => 전용 소켓 할당 => 소켓 식별자를 doit 함수에 전달*/
  listenfd = Open_listenfd(argv[1]);
  printf("Server listening on: %s ...\n", argv[1]);
  while(1){
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Server accepts a client: (%s, %s)\n", hostname, port);
    
    doit(connfd, hostname, port);
    Close(connfd);
  }
}



void doit(int connfd, char *hostname, char *port){
  /*tiny에게 클라이언트 처럼 연결 요청을 하기*/
  /*변수 선언부*/
  int serverfd;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char host[MAXLINE], port[MAXLINE], path[MAXLINE];
  char bufs[MAXLINE];
  rio_t rio;

  /*클라이언트 요청 처리*/
  /*buf에 요청 모두 넣기*/
  rio_readinitb(&rio, connfd);
  rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);
  printf("Header accepted: %s", buf);

  /*Tiny와 연결할 소켓 생성(본인은 클라이언트로 act)*/
  serverfd = Open_clientfd(hostname, port);
  if(serverfd < 0){
    printf("Failed to make serverfd");
    return;
  }

  /*Tiny 용 헤더 생성 => bufs에 넣기*/
  generate_header(bufs, hostname, port, path);
  /*bufs 내용을 serverfd 소켓에 보내기*/
  Rio_writen(serverfd, bufs, strlne(bufs));

  Close(serverfd);
}



void generate_header(char *bufs, char *hostname, char *port, char *path){
  char temp_header[MAXLINE];
  
  sprintf(temp_header, "GET /%s HTTP/1.0\r\n", path);
  strcat(bufs, temp_header);
  strcat(bufs, "Host: localhost:8000\r\n");
  strcat(bufs, "Connection: close");
  strcat(bufs, "Proxy-Connection: close");
}