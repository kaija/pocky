#include <stdio.h>
#include <stdlib.h>

#include "pocky.h"

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
		list_init(&base->list);
        list_attributes_seeker(&base->list, pocky_seeker);
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

/**
 * @name    pocky_add_ev
 * @brief   add a fd into the pocky base
 */
int pocky_add_ev(int fd,
                struct pocky_base *base,
                void (*event_cb)(int fd, short event, void *pdata),
                void *pdata)
{
    struct pocky_ev *ev = malloc(sizeof(struct pocky_ev));
    if(ev){
        memset(ev, 0, sizeof(struct pocky_ev));
        ev->fd = fd;
        ev->pdata = pdata;
        ev->event_cb = event_cb;
        if(list_append(&base->list, (void *)ev) < 0){
            LOG("list append failure\n");
        }
        return 0;
    }
    return -1;
}

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

void *pocky_seek_fd(struct pocky_base *base, int fd)
{
    if(base){
        list_seek(&base->list, &fd);
    }
    return NULL;
}

void pocky_destroy_ev(struct pocky_ev *ev)
{
    if(ev){
        free(ev);
    }else{
        LOG("pocky event destroy error\n");
    }
}

void pocky_destroy_base(struct pocky_base *base)
{
    if(base){
        list_destroy(&base->list);
        free(base);
    }
}
unsigned int pocky_base_size(struct pocky_base *base){
    if(base){
        return list_size(&base->list);
    }
    return 0;
}
int pocky_dispatch(struct pocky_base *base)
{
    struct timeval tv;
    fd_set working_set;
    fd_set backup_set;
    int fd_max = 0;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    for(;;)
    {
        int res;
        memcpy(&working_set, &backup_set, sizeof(backup_set));
        res = select(fd_max, &working_set, NULL, NULL, &tv);
        if(res == -1){
            LOG("pocky select event error %d\n", res);
        }else if(res == 0){
            
        }else{
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
