#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"



cache_obj *find_cache(char *uri){
    /*머리부터 시작 => 탐색 진행*/
    cache_obj *cur = head;
    if(head == NULL) return NULL;

    while(cur != NULL){
        if(!strcasecmp(cur->uri, uri))
            return cur;
        cur = cur->next;
    }
    return NULL;
}



void insert_cache(char *uri, char *data, long size){

}



void remove_cache(cache_obj obj){

}



void evcit_cache(long needed_size){

}



void send_cache(int fd, cache_obj obj){
    
}