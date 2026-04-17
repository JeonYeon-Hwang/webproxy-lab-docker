#define _GNU_SOURCE  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>   
#include <sys/types.h>
#include <sys/socket.h>
#include "../csapp.h"

void echo(int connfd);
int my_open_listenfd(char *port);

// 서버 실행부
int main(int argc, char **argv) 
{
    // listenfd: 서버에 1개만 존재, 클라이언트의 접속을 대기
    // connfd: 각 연결된 클라이언트의 소켓에 접근할 수 있는 인덱스
    int listenfd, connfd;
    // 클라이언트의 주소 구조체 총 size를 담는 변수
    socklen_t clientlen;
    // 클라이언트 주소 구조체를 담음(임시)
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if(argc != 2){
        fprintf(stderr, "input 형식이 올바르지 않습니다: %s <port> \n", argv[0]);
        exit(0);
    }

    // listen할 창구를 생성
    listenfd = my_open_listenfd(argv[1]);
    printf("서버가 %s 주소로 접속 대기 상태입니다...", argv[1]);
    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        // listen 소켓에서 받은 클라이언트 addrinfo를 바탕으로 대화용 소켓 생성 (통로: connfd)
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        // 접속한 클라이언트의 정보 가져오기
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                        client_port, MAXLINE, 0);
        printf("다음 클라이언트가 접속하였습니다: %s, %s \n", client_hostname, client_port);
        
        // 해당 클라이언트 대화용 소켓(connfd)으로 소통 진행
        Close(connfd);
    }  
    exit(0);
    printf("서버가 종료되었습니다.");
}



void echo(int connfd)
{

}


// 서버가 listen 소켓 생성을 하는 함수
int my_open_listenfd(char *port)
{
    // 본인의 addrinfo를 저장하는 구조체 변수(서버는 수동적)
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    // 구조체를 초기화
    memset(&hints, 0, sizeof(struct addrinfo));
    // 이 연결은 TCP
    hints.ai_socktype = SOCK_STREAM;
    // 아무 주소나 접속 가능 & 내 형식에 맞는 것말 수용
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    // 클라이언트와 다름
    // 내부망에 들어가 가능한 ip 리스트를 listp를 root로 하여 linkedlist로 반환
    Getaddrinfo(NULL, port, &hints, &listp);

    // 가능한 ip 리스트를 순회하면서 listen 소켓을 만듦
    for(p = listp; p; p = p->ai_next){
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        char *ip_string = inet_ntoa(ipv4->sin_addr);
        printf("%s 주소로 listen 소켓을 만듭니다... \n", ip_string);

        // 해당 주소로 listen 소켓 생성
        if((listenfd = socket(p->ai_family, p-> ai_socktype, p->ai_protocol)) < 0){
            printf("%s 주소 소켓 생성 실패. \n", ip_string);
            continue;
        }

        // 해당 포트를 강제로 열게 하는 권한을 listenfd에 줌
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                    (const void *)&optval, sizeof(int));


        // 생성 성공 시 => bind를 함(소켓에 해당 listenfd를 연결)
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0){
            printf("%s 주소에 bind 실패. \n");
            break;
        }
        Close(listenfd);
    }

    // 힙 할당해제
    Freeaddrinfo(listp);
    if(!p){
        printf("open_listenfd가 연결상태 생성에 실패하였습니다. \n");
        return -1;
    }

    // 사용 가능한 소켓인지 검증
    if(listen(listenfd, LISTENQ) < 0){
        printf("생성된 listen 소켓이 유효하지 않습니다. \n");
        Close(listenfd);
        return -1;
    }

    return listenfd;
}