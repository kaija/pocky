#ifndef __POCKY_H
#define __POCKY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simclist.h"

#define     PK_NAME_LEN     16
#define     PK_CTRL_PORT    9999
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
    void        (*destroy_cb)(void *pdata);
};



struct pocky_base{
    int         fd_max;
    int         ctrl_sk;
    int         ctrl_port;
    int         working;
    void        (*destroy_cb)(void *pdata); 	//callback function when pocky_ev destroyed
    void        (*timeout_cb)(void *pdata); 	//callback function when pocky_ev destroyed
    list_t      list;
};




struct pocky_base *pocky_init();
void pocky_destroy_base(struct pocky_base *base);
int pocky_add_ev(struct pocky_base *base,
                int fd,
                void (*event_cb)(int fd, short event, void *pdata),
                void *pdata);
unsigned int pocky_base_size(struct pocky_base *base);
int pocky_udp_socket(int port);
int pocky_del_ev(struct pocky_base *base, int fd);
int pocky_base_loop(struct pocky_base *base);
int pocky_udp_sender(char *addr, int port, char *payload, int len);
int pocky_reg_cb(struct pocky_base *base, int fd, void(*destroy_cb)(void *pdata));
#endif
