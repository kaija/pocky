#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>

#include "pocky.h"

#define POCKY_SELECT_TIMEOUT    60
#define POCKY_CTRL_PAYLOAD      128

#define POCKY_LOCALHOST         "127.0.0.1"

#define POCKY_RESET             "reset"

/**
 * @name    pocky_seeker
 * @brief   this seeker is used for seek the fd
 */
int pocky_seeker(const void *e, const void *key)
{
    const struct pocky_ev *ev = (struct pocky_ev *)e;
    int target = *(int *)key;
    if(ev->fd == target)
    {
        return 1;
    }
    return 0;
}

/**
 * @name    pocky_init
 * @brief   init and return pocky_base data structure
 */
struct pocky_base *pocky_init()
{
    struct pocky_base *base = malloc(sizeof(struct pocky_base));
    if(base){
        memset(base, 0, sizeof(struct pocky_base));
        base->fd_max = 0;
        list_init(&base->list);
        list_attributes_seeker(&base->list, pocky_seeker);
        int port = PK_CTRL_PORT, sk = 0;
        while((sk = pocky_udp_socket(port)) < 0){
            port ++;
        }
        LOG("create port %d for control socket\n", port);
        base->ctrl_port = port;
        base->ctrl_sk = sk;
    }else{
        LOG("pocky initial failure\n");
    }
    return base;
}

/**
 * @name    pocky_new_ev
 * @brief   create a pocky_ev and init it
 */
struct pocky_ev *pocky_new_ev()
{
    struct pocky_ev *ev;
    ev = malloc(sizeof(struct pocky_ev));
    if(ev){
        memset(ev, 0, sizeof(struct pocky_ev));
        ev->fd = -1;
        ev->parent_fd = -1;
        ev->pdata = NULL;
        ev->event_cb = NULL;
        ev->accept_cb = NULL;
    }else{
        LOG("pocky event allocate failure\n");
    }
    return ev;
}

int pocky_add_set(struct pocky_base *base, fd_set *set)
{
    if(base){
        int size = list_size(&base->list);
        int i;
        FD_ZERO(set);
        if(base->ctrl_sk > 0){
            FD_SET(base->ctrl_sk, set);
            if(base->ctrl_sk > base->fd_max) base->fd_max = base->ctrl_sk;
        }
        for(i = 0; i<size; i++){
            struct pocky_ev *ev = (struct pocky_ev *)list_get_at(&base->list, i);
            //printf("ev->fd %d\n", ev->fd);
            FD_SET(ev->fd, set);
        }
        return 0;
    }
    return -1;
}

/**
 * @name    pocky_add_ev
 * @brief   add a fd into the pocky base
 */
int pocky_add_ev(struct pocky_base *base,
                int fd,
                void (*event_cb)(int fd, short event, void *pdata),
                void *pdata)
{
    struct pocky_ev *ev = malloc(sizeof(struct pocky_ev));
    if(ev){
        memset(ev, 0, sizeof(struct pocky_ev));
        ev->fd = fd;
        if(fd > base->fd_max) base->fd_max = fd;
        ev->pdata = pdata;
        ev->event_cb = event_cb;
        if(list_append(&base->list, (void *)ev) < 0){
            LOG("list append failure\n");
        }
        //LOG("add event success\n");
        if(base->working == 1){
            //LOG("notify event loop %d\n", base->ctrl_port);
            pocky_udp_sender(POCKY_LOCALHOST, base->ctrl_port, POCKY_RESET, strlen(POCKY_RESET));
        }
        return 0;
    }
    return -1;
}
int pocky_reg_cb(struct pocky_base *base, int fd, void(*destroy_cb)(void *pdata))
{
    if(base){
        struct pocky_ev *ev = (struct pocky_ev *) list_seek(&base->list, &fd);
        ev->destroy_cb = destroy_cb;
        return 0;
    }
    return -1;
}
/**
 * @name    pocky_del_ev
 * @brief   del a fd in the pocky base
 */
int pocky_del_ev(struct pocky_base *base, int fd)
{
    if(base){
        void *obj = list_seek(&base->list, &fd);
        list_delete_at(&base->list, list_locate(&base->list, obj));
        if(base->working == 1){
            pocky_udp_sender(POCKY_LOCALHOST, base->ctrl_port, POCKY_RESET, strlen(POCKY_RESET));
        }
        return 0;
    }
    return -1;
}
/**
 * @name    pocky_accept_ev
 * @brief   accept a tcp connection and add into pocky base
 */
int pocky_accept_ev(int fd, int child_fd,
                struct pocky_base *base,
                void (*event_cb)(int fd, short event, void *pdata),
                void *pdata)
{
    struct pocky_ev *ev = malloc(sizeof(struct pocky_ev));
    if(ev){
        memset(ev, 0, sizeof(struct pocky_ev));
        ev->fd = child_fd;
        ev->parent_fd = fd;
        ev->pdata = pdata;
        ev->event_cb = event_cb;
        if(list_append(&base->list, (void *)ev) < 0){
            LOG("list append failure\n");
        }
        return 0;
    }
    return -1;
}

/**
 * @name    pocky_seek_fd
 * @brief   seek fd in pocky base
 */
void *pocky_seek_fd(struct pocky_base *base, int fd)
{
    if(base){
        list_seek(&base->list, &fd);
    }
    return NULL;
}

/**
 * @name    pocky_reg_destroy_cb
 * @brief   register a callback, called when pocky ev deleted
 */
void pocky_reg_destroy_cb(struct pocky_base *base, void (*destroy_cb)(void *pdata))
{
    base->destroy_cb = destroy_cb;
}

/**
 * @name    pocky_destroy_ev
 * @brief   destroy pocky ev
 */
void pocky_destroy_ev(struct pocky_base *base, struct pocky_ev *ev)
{
    if(ev){
        if(base->destroy_cb){ // if user register destroy callback to delete private pdata. call it.
            base->destroy_cb(ev->pdata);
        }
        free(ev);
    }else{
        LOG("pocky event destroy error\n");
    }
}

/**
 * @name    pocky_destroy_base
 * @brief   destroy pocky base
 */
void pocky_destroy_base(struct pocky_base *base)
{
    int i, size;
    if(base){
        // check list and remove all pocky_ev
        if((size = list_size(&base->list)) > 0) {
            for(i=0 ; i<size ;i++){
                struct pocky_ev *ev = (struct pocky_ev *)list_get_at(&base->list, i);
                pocky_destroy_ev(base, ev);
            }
        }
        list_destroy(&base->list);
        free(base);
    }
}
/**
 * @name    pocky_base_size
 * @brief   return the size of pocky ev in pocky base
 */
unsigned int pocky_base_size(struct pocky_base *base){
    if(base){
        return list_size(&base->list);
    }
    return 0;
}
/**
 * @name    pocky_fd_isset
 * @brief   check which fd is being set
 */
int pocky_fd_isset(struct pocky_base *base, fd_set *set)
{
    if(base){
        int size = list_size(&base->list);
        int i;
        for(i = 0; i<size; i++){
            struct pocky_ev *ev = (struct pocky_ev *)list_get_at(&base->list, i);
            if(FD_ISSET(ev->fd, set)){
                return i;
            }
        }
        return -1;
    }
    return -1;
}
/**
 * @name    pocky_dispatch
 * @brief   dispatch a event when fd is set
 */
int pocky_dispatch(struct pocky_base *base, int pos)
{
    struct pocky_ev *ev = (struct pocky_ev *)list_get_at(&base->list, pos);
    if(ev){
        if(ev->event_cb){
            ev->event_cb(ev->fd, 0, ev->pdata);
        }
        return 0;
    }
    return -1;
}

void pocky_clear_socket(int fd)
{
    socklen_t len;
    struct sockaddr_in cliaddr;
    char buf[POCKY_CTRL_PAYLOAD];
    recvfrom(fd, buf, POCKY_CTRL_PAYLOAD, 0, (struct sockaddr *)&cliaddr, &len);

}
int pocky_reg_timeout_cb(struct pocky_base *base, void(*timeout_cb)(void *pdata))
{
    LOG("register timeout callback!\n");
    if(base){
        base->timeout_cb = timeout_cb;
    }
    return 0;
}

void pocky_timeout_ev(struct pocky_base *base)
{
    if(base->timeout_cb){
        base->timeout_cb(NULL);
    }
}
/**
 * @name    pocky_base_loop
 * @brief   start monitor pocky base fd
 */
int pocky_base_loop(struct pocky_base *base)
{
    time_t last_poll = 0;
    time_t timeout = 0;
    struct timeval tv;
    fd_set working_set;
    fd_set backup_set;
    last_poll = time(NULL);
    timeout = last_poll + POCKY_SELECT_TIMEOUT;
    for(;;)
    {
        int res;
        int timer = timeout - time(NULL);
        if(timer < 1)
        {
            goto timeout_handle;
        }
        tv.tv_sec = timer; // set select timeout prevent select stuck
        tv.tv_usec = 0;
        pocky_add_set(base, &backup_set);//refresh backup_set
        printf("");
        memcpy(&working_set, &backup_set, sizeof(backup_set));
        base->working = 1;
        res = select(base->fd_max+1, &working_set, NULL, NULL, &tv);
        if(res == -1){
            //LOG("pocky select event error %d\n", res);
        }else if(res == 0){
timeout_handle:
            //LOG("pocky select timeout\n");
            pocky_timeout_ev(base);
            last_poll = time(NULL);
            timeout = last_poll + POCKY_SELECT_TIMEOUT;
        }else{
            if(FD_ISSET(base->ctrl_sk, &working_set)){
                //LOG("pocky control socket triggered\n");
                pocky_clear_socket(base->ctrl_sk);
            }else{
                int pos = pocky_fd_isset(base, &working_set);
                if(pos > -1){
                    pocky_dispatch(base, pos);
                }
            }
        }
    }
}


void sample(struct pocky_base *base, int fd)
{
    int i;
    int size = list_size(&base->list);
    for(i = 0; i < size ; i++){
        void *tmp = list_get_at(&base->list, i);
        struct pocky_ev *ev = tmp;
        printf("ev fd %d parent fd %d\n", ev->fd, ev->parent_fd);
    }
}

void sample_trigger(struct pocky_base *base , int fd)
{
    struct pocky_ev *ev =  (struct pocky_ev *) list_seek(&base->list, &fd);
    if(ev){
        printf("find you \n");
    }
}
int pocky_socket_set_reuseaddr(int sk)
{
    int on = 1;
    return setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on));
}
int pocky_socket_set_nonblock(int sk)
{
    unsigned long on = 1;
    return ioctl(sk, FIONBIO, &on);
}
int pocky_udp_socket(int port)
{
    int res = 0;
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    res = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(res == -1){
        close(sockfd);
        return 0;
    }
    pocky_socket_set_reuseaddr(sockfd);
    pocky_socket_set_nonblock(sockfd);
    return sockfd;
}



int pocky_udp_sender(char *addr, int port, char *payload, int len)
{
    int sock;
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0) { return -1; }
    if(payload == NULL) { return -1; }
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(addr);
    serv_addr.sin_port = htons(port);
    ssize_t n = sendto(sock, payload, len, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    close(sock);
    return n;
}

int pocky_tcp_socket(int port)
{
    if(port < 0 || port > 65534) return -1;
    int res = 0;
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    res = bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if(res < 0){
        fprintf(stderr,"bind socket error\n");
        close(sock);
    }
    pocky_socket_set_reuseaddr(sock);
    pocky_socket_set_nonblock(sock);
    res = listen(sock, PK_MAX_CONN);
    if(res < 0){
        fprintf(stderr,"listen socket error\n");
        close(sock);
    }
    return sock;
}
