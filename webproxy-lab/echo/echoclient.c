// #define _GNU_SOURCE  
#include "../csapp.h"

int my_open_clientfd(char *hostname, char *port);

// 클라이언트 메인 실행부 함수
int main(int argc, char **argv) 
{
    // 소켓을 찾아가는 인덱스 역할
    int clientfd;
    // 서버를 찾아가기 위한 정보 & 들고갈 내용물
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    // input 형식 틀렸는지 방어코드
    if(argc != 3){
        fprintf(stderr, "input 형식이 올바르지 않습니다: %s <host> <port> \n", argv[0]);
        exit(0);
    }

    //이제 각각 할당(포인터임, 어딘가에 저장된 argv의 참조 char 주소값을 할당)
    host = argv[1];
    port = argv[2];

    // 연결이 실현되고, 성공시 양의 정수값(인덱스) 반환
    clientfd = my_open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while(Fgets(buf, MAXLINE, stdin) != NULL){
        // 서버로 해당 buf를 발송
        Rio_writen(clientfd, buf, strlen(buf));
        // 서버에서의 응답을 buf에 넣음
        Rio_readlineb(&rio, buf, MAXLINE);
        // 이 buf를 출력
        Fputs(buf, stdout);
    }

    Close(clientfd);
    exit(0);
    printf("서버와 연결이 종료되었습니다.");
}



// 클라이언트가 socket 생성 및 connect 하는 함수
int my_open_clientfd(char *hostname, char *port){
    int clientfd;
    // 할당할 구조체 변수 선언
    struct addrinfo hints, *listp, *p;
    
    // 스택 내 구조체 변수 초기화
    memset(&hints, 0, sizeof(struct addrinfo));
    // 이 연결은 TCP
    hints.ai_socktype = SOCK_STREAM;
    // 포트는 정수 형태로 제공(목적지의 서버 포트 번호)
    hints.ai_flags = AI_NUMERICSERV;
    // 클라이언트 형식에 맞는 것만 제공
    hints.ai_flags |= AI_ADDRCONFIG;
    // DNS서버에 보내서 => 실제 ip주소를 list로 받아와서 listp 포인터에 붙여줌
    Getaddrinfo(hostname, port, &hints, &listp);

    //listp를 root로 하여 포인터 순회
    for(p = listp; p; p = p->ai_next){
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        char *ip_string = inet_ntoa(ipv4->sin_addr);
        printf("%s 주소에 연결 시도 중... \n", ip_string);
        // 클라이언트가 해당 ip에 맞는 socket을 생성
        // 소캣이 생성 실패하여 -1을 반환한다면?
        if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            continue;
        }

        // 연결 시도 및 확인유무
        if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1){
            printf("%s에 연결 성공하였습니다!\n", ip_string);
            break;
        }

        Close(clientfd);
    }

    // 힙 할당해제
    Freeaddrinfo(listp);
    if(!p){
        printf("open_client_fd가 연결에 실패하였습니다. \n");
        return -1;
    }
    else{
        return clientfd;
    }      
}