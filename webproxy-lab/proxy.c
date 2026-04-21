#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void doit(int connfd, char *host);
void generate_header(char *request_buf, char *hostname, char *port, char *path);
void read_requesthdrs(rio_t *rp);
    
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
    
    doit(connfd, hostname);
    Close(connfd);
  }
}



void doit(int connfd, char *hostname){
  /*tiny에게 클라이언트 처럼 연결 요청을 하기*/
  /*변수 선언부*/
  int serverfd, n;
  char *tiny_port = "8000";
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], path[MAXLINE];
  char request_buf[MAXLINE], response_buf[MAXLINE];
  rio_t rio, server_rio;

  /*클라이언트 요청 처리*/
  /*buf에 요청 모두 넣기 => rio에 할당*/
  rio_readinitb(&rio, connfd);
  rio_readlineb(&rio, buf, MAXLINE);
  sscanf(buf, "%s %s %s", method, uri, version);
  printf("Request headers:\n");
  read_requesthdrs(&rio);   
  printf("\n");

  /*uri => path*/
  strcpy(path, uri);

  /*Tiny와 연결할 소켓 생성(본인은 클라이언트로 act)*/
  serverfd = Open_clientfd(hostname, tiny_port);
  if(serverfd < 0){
    printf("Failed to make serverfd");
    return;
  }

  /*Tiny 용 헤더 생성 => bufs에 넣기*/
  generate_header(request_buf, hostname, tiny_port, path);
  /*request_buf 내용을 serverfd 소켓에 보내기*/
  Rio_writen(serverfd, request_buf, strlen(request_buf));

  /*Tiny => 프록시 응답 대기 및 읽기*/
  Rio_readinitb(&server_rio, serverfd);
  while ((n = rio_readnb(&server_rio, response_buf, MAXLINE)) != 0){
    /*클라이언트 connfd에 전달(가공 없이)*/
    printf("Received %d bytes from Tiny\n", n);
    Rio_writen(connfd, response_buf, n);
  }
  printf("Response Loop Finished!\n");
  Close(serverfd);
}



void generate_header(char *request_buf, char *hostname, char *port, char *path){
  sprintf(request_buf, "GET /%s HTTP/1.0\r\n", path);
  sprintf(request_buf + strlen(request_buf), "Host: %s:%s\r\n", hostname, port);
  sprintf(request_buf + strlen(request_buf), "%s", user_agent_hdr);
  sprintf(request_buf + strlen(request_buf), "Connection: close\r\n");
  sprintf(request_buf + strlen(request_buf), "Proxy-Connection: close\r\n\r\n");
}



void read_requesthdrs(rio_t *rp){
  /*rio의 각 줄을 할당하는 buf 객체*/
  char buf[MAXLINE];

  /*첫 번째 줄 읽기: request line => 별도 출력 필요 없음*/
  Rio_readlineb(rp, buf, MAXLINE);
  /*조건문: 해당 buf에 enter가 있을 때 까지*/
  while(strcmp(buf, "\r\n")){
    printf("%s", buf);
    Rio_readlineb(rp, buf, MAXLINE); 
  }
  return;
}