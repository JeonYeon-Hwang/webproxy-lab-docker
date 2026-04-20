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
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  /*uri 파싱 데이터 저장 변수들*/
  char filename[MAXLINE], cgiargs[MAXLINE];
  /*rio 읽기 버퍼 구조체*/
  rio_t rio;
  

  /*rio에 fd 바인딩(초기화) => buf에 할당*/
  rio_readinitb(&rio, fd);
  rio_readlineb(&rio, buf, MAXLINE);
  printf("다음 헤더를 받았습니다: %s", buf);

  /*buf => 각 변수에 분해 할당*/
  sscanf(buf, "%s %s %s", method, uri, version);

  /*방어코드: GET만 유효함*/
  if(strcasecmp(method, "GET")){
    clienterror(fd, method, "501", "Not implemented", "Tiny 서버에서 지원하지 않는 기능입니다.");
    return;
  }

  /*읽고 콘솔에 찍기*/
  read_requesthdrs(&rio);

  /*uri => filename & cgiargs로 파싱 & static 반환*/
  is_static = parse_uri(uri, filename, cgiargs);
  /*파일 존재 여부 확인 => sbuf에 형식 & 권한 저장*/
  if(stat(filename, &sbuf) < 0){
    clienterror(fd, filename, "404", "Not found", "Tiny 서버가 해당 파일을 찾지 못했습니다.");
    return;
  }

  /*static => 읽기 권한 확인*/
  if(is_static){
    /*!regular 파일인가 or !읽기 권한이 있는가 */
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "Forbidden", "Tiny 서버가 해당 파일을 읽을 수 없습니다.");
      return;
    }
    /*추후 상세 작성 예정*/
    serve_static(fd, filename, sbuf.st_size);
  }
  /*!static => 읽기 권한 확인*/
  else{
      /*!regular 파일인가 or !실행 권한이 있는가 */
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "Forbidden", "Tiny 서버가 해당 CGI 프로그램을 실행시킬 수 없습니다.");
      return;
    }
    /*추후 상세 작성 예정*/
    serve_dynamic(fd, filename, cgiargs);
  }

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



int parse_uri(char *uri, char *filename, char *cgiargs){
  /*동적 컨텐츠를 요구하는 uri*/
  if(strstr(uri, "cgi-bin")){
    /*"?"가 있는 주소값을 반환*/
    char *ptr = index(uri, '?');
    if(ptr != NULL){
      strcpy(cgiargs, ptr + 1);
      /*uri를 "?"" 이전 까지만 읽도록 하기 위해*/
      *ptr = '\0';
    }
    else strcpy(cgiargs, "");
    /*filename 입력*/
    snprintf(filename, MAXLINE, ".%s", uri);
    return 0;
  }
  /*정적 컨텐츠를 요구하는 uri*/
  else{
    strcpy(cgiargs, "");
    /*filename 입력*/
    snprintf(filename, MAXLINE, ".%s", uri);
    /*만약 uri가 그냥 "/" 이라면?*/
    if(uri[strlen(uri) - 1] == '/'){
      /*filename에 아래 문자열을 추가*/
      strcat(filename, "home.html");
    }
    return 1;
  }
}



void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
  /*사용할 변수 선언*/
  char buf[MAXLINE], body[MAXBUF];

  /*body 만들기: html 문서 형태*/
  sprintf(body, "<html><title>Tiny Error</title>\r\n"
                "<body bgcolor=\"ffffff\">\r\n"
                "%s: %s\r\n"
                "<p> %s: %s\r\n"
                "<hr><em> The Tiny Web server</em>\r\n",
                errnum, shortmsg, longmsg, cause);

  /*header 작성 => fd를 통해 rio 객체로 소켓에 넣기*/
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));

  /*body 주입 => 마찬가리조 fd와 rio를 통해 넣기*/
  Rio_writen(fd, body, strlen(body));
}



void serve_static(int fd, char *filename, int filesize){
  /*사용할 변수 선언*/
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  char *p;

  /*파일 타입을 먼저 구하기*/
  get_filetype(filename, filetype);
  
  /*header 작성*/
  p = buf;
  p += sprintf(p, "HTTP/1.0 200 OK\r\n");
  p += sprintf(p, "Server: Tiny Web Server\r\n");
  p += sprintf(p, "Connection: Close\r\n");
  p += sprintf(p, "Content-length: %d\r\n", filesize);
  p += snprintf(p, MAXBUF - (p - buf), "Content-type: %s\r\n\r\n", filetype);
  /*한꺼번에 rio를 통해 소켓에 보내기*/
  Rio_writen(fd, buf, strlen(buf));
  /*콘솔에 header 출력하기*/
  printf("Response headers: \n");
  printf("%s", buf);

  /*파일 찾기 => 파일 식별자로 매핑*/
  srcfd = Open(filename, O_RDONLY, 0);
  /*MMU(메모리 장부)에 어떤 형태로 사용될지 기록 => srcp에 매핑*/
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  close(srcfd);
  /*소켓에 해당 내용 보내기*/
  Rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize);
}



void get_filetype(char *filename, char *filetype){
  /*파일 형식 입력*/
  if(strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if(strstr(filename, ".gif"))
    strcpy(filetype, "image.gif");
  else if(strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if(strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}



void serve_dynamic(int fd, char *filename, char *cgiargs){
  char buf[MAXLINE], *emptylist[] = { NULL };

  /*header 작성 => 소켓에 보내기*/
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n\r\n");
  Rio_writen(fd, buf, strlen(buf));

  /*자식 프로세서 만들기: fork*/
  /*if 구문 내: 자식 프로세스 실행부(독립된 공간임)*/
  if(Fork() == 0){
    /*자식 프로세서 환경변수 설정*/
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd, STDOUT_FILENO);
    /*이제 자식 프로세서 실행*/
    Execve(filename, emptylist, environ);
  }
  Wait(NULL);
}