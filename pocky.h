#ifndef __POCKY_H
#define __POCKY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simclist.h"

#define     PK_NAME_LEN 16

#ifdef DEBUG
#define LOG(fmt,args...) printf("[%s:%d]  "fmt,__FILE__,__LINE__,##args)
#else
#define LOG
#endif

struct pocky_ev{
    int         fd;                     // socket or file descriptor
    int         parent_fd;              // socket accept by 
    char        name[PK_NAME_LEN];      // the pocky event name
    void        *pdata;                 // the private data for callback
    void        (*event_cb)(int fd, short event, void *pdata);
    void        (*accept_cb)(int fd, short event, void *pdata);
};



struct pocky_base{
    list_t      list;
};




struct pocky_base *pocky_init();
void pocky_destroy_base(struct pocky_base *base);
int pocky_add_ev(int fd,
                struct pocky_base *base,
                void (*event_cb)(int fd, short event, void *pdata),
                void *pdata);
unsigned int pocky_base_size(struct pocky_base *base);
#endif
