#include <stdio.h>
#include <stdlib.h>

#include "pocky.h"


int pocky_seeker(const void *e, const void *key)
{
    const struct pocky_ev *ev = (struct pocky_ev *)e;
    int target = *(int *)key;
    if(ev->fd == target || ev->parent_fd ==target)
    {
        return 1;
    }
    return 0;
}

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

int pocky_add_ev(int fd, struct pocky_base *base, void *pdata)
{

    return 0;
}

void pocky_destroy_ev(struct pocky_ev *ev)
{
    if(ev){
        free(ev);
    }else{
        LOG("pocky event destroy error\n");
    }
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
