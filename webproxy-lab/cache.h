/*얘는 이해를 못하겠음...*/
#ifndef __CACHE_H__
#define __CACHE_H__

#include <stdlib.h>
#include "csapp.h"

/*최댓값 설정*/
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/*캐시 구조체*/
/*data: 실제 바이너리로 저장됨, *next/*prev: 이전/이후 탐색용*/
typedef struct _cache_obj{
    char *uri;
    unsigned char *data;
    long size;

    struct _cache_obj *next;
    struct _cache_obj *prev;
} cache_obj;

/*static 변수 선언*/
static int cache_size = 0; 
static cache_obj *head = NULL;
static cache_obj *tail = NULL;

/*함수 선언*/
cache_obj *find_cache(char *uri);
void insert_cache(char *uri, char *data, long size);
void remove_cache(cache_obj obj);
void evcit_cache(long needed_size);
void send_cache(int fd, cache_obj obj);

#endif