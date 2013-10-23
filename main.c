#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "pocky.h"
void recv_cb(int fd, short event, void *pdata)
{
    struct pocky_base *base = (struct pocky_base *)pdata;
    struct sockaddr_in cliaddr;
    char buf[1024];
    socklen_t len;
    memset(buf, 0, 1024);
    int res = recvfrom(fd, buf, 1024, 0, (struct sockaddr *)&cliaddr, &len);
    if(res > 0){
        printf("get %s\n", buf);
    }else{
        printf("remote disconnect\n close it %d\n", fd);
        pocky_del_ev(base, fd);
    }
}

void accept_cb(int fd, short event, void *pdata){
printf("accept callback received\n");
    int sk = accept(fd, NULL, NULL);
    if(sk < 0){
        if(errno == EWOULDBLOCK){
            printf("would block ??\n");
        }else if(errno == EAGAIN){
            printf("again??\n");
        }else{
            printf("accept error\n");
        }
        return;
    }
    struct pocky_base *base = (struct pocky_base *)pdata;
    pocky_add_ev(base, sk, recv_cb, base);
}

void *recer(void *arg)
{
    struct pocky_base *base = (struct pocky_base *) arg;
    pocky_base_loop(base);
    pocky_destroy_base(base);
    return NULL;
}
int main()
{
    pthread_t thread1;
    struct pocky_base *base = pocky_init();
    printf("%s%d\n",__FILE__,__LINE__);
/*
    int a = pocky_udp_socket(6000);
    pocky_add_ev(base, a, recv_cb, NULL);

    pthread_create(&thread1, NULL, recer, (void*) base);
    //pthread_join(thread1, NULL);
    sleep(10);
    printf("%u\n",pocky_base_size(base));
    pocky_del_ev(base, a);
    a = pocky_udp_socket(8000);
    pocky_add_ev(base, a, recv_cb, NULL);
    a = pocky_udp_socket(9000);
    pocky_add_ev(base, a, recv_cb, NULL);
    sleep(100);
*/

    int sk =  pocky_tcp_socket(56789);
    if(sk < 0) {
        printf("create tcp socket error\n");
    }else{
        printf("TCP socket created %d\n", sk);
        pocky_add_ev(base, sk, accept_cb, (void *)base);
        pocky_base_loop(base);
    }
	return 0;
}
