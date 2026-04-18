/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /*input 유효성 검증*/
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  /*listen 소켓 생성 후 => listenfd에 바인딩*/
  listenfd = Open_listenfd(argv[1]);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    /*connect 성공 => 전용 소켓 생성 및 addr 할당, connfd에 바인딩*/
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); 
    /*클라이언트 정보 가져오기*/
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("다음 클라이언트의 접속 요청을 수락함: (%s, %s)\n", hostname, port);
    /*하나의 HTTP 트랜잭션을 처리함*/
    doit(connfd);  
    Close(connfd); 
  }
}



void doit(int fd){
  /*변수 선언부: 헤더를 구조적 분해 할당함*/
  int is_static;
  /*상태 저장 구조체*/
  struct stat sbuf;
  /*파싱된 데이터 저장 변수들*/
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE];
  /*uri 파싱 데이터 저장 변수들*/
  char filename[MAXLINE], cgiargs[MAXLINE];
  /*rio 읽기 버퍼 구조체*/
  rio_t rio;
  

  /*rio에 fd 바인딩(초기화) => buf에 할당*/
  rio_readinitb(&rio, fd);
  rio_readlineb(&rio, buf, MAXLINE);
  printf("다음 헤더를 받았습니다: %s", buf);

  /*buf => 각 변수에 분해 할당*/
  sscanf(buf, "%s %s %s", method, uri);

  /*방어코드: GET만 유효함*/
  if(strcasecmp(method, "GET")){
    clienterror(fd, method, "505", "내장 기능에 없음", "Tiny 서버에서 지원하지 않는 기능입니다.");
    return;
  }

  /*읽고 콘솔에 찍기*/
  read_requesthdrs(&rio);

  /*uri => filename & cgiargs로 파싱 & static 반환*/
  is_static = parse_uri(uri, filename, cgiargs);
  /*파일 존재 여부 확인 => sbuf에 형식 & 권한 저장*/
  if(stat(filename, &sbuf) < 0){
    clienterror(fd, filename, "404", "파일 없음", "Tiny 서버가 해당 파일을 찾지 못했습니다.");
    return;
  }

  /*static => 읽기 권한 확인*/
  if(is_static){
    /*!regular 파일인가 or !읽기 권한이 있는가 */
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "접근 금지됨", "Tiny 서버가 해당 파일을 읽을 수 없습니다.");
      return;
    }
    /*추후 상세 작성 예정*/
    serve_static(fd, filename, sbuf.st_size);
  }
  /*!static => 읽기 권한 확인*/
  else{
      /*!regular 파일인가 or !실행 권한이 있는가 */
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "접근 금지됨", "Tiny 서버가 해당 CGI 프로그램을 실행시킬 수 없습니다.");
      return;
    }
    /*추후 상세 작성 예정*/
    serve_dynamic(fd, filename, sbuf.st_size);
  }

}



void read_requesthdrs(rio_t *rp){
  /*rio의 각 줄을 할당하는 buf 객체*/
  char buf[MAXLINE];

  /*첫 번째 줄 읽기: request line => 별도 출력 필요 없음*/
  rio_readlineb(&rp, buf, MAXLINE);
  /*조건문: 해당 buf에 enter가 있을 때 까지*/
  while(strcmp(buf, "\r\n")){
    rio_readlineb(&rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}