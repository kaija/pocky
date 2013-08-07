#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "pocky.h"
void recv_cb(int fd, short event, void *pdata)
{
    struct sockaddr_in cliaddr;
    char buf[1024];
    socklen_t len;
    memset(buf, 0, 1024);
    recvfrom(fd, buf, 1024, 0, (struct sockaddr *)&cliaddr, &len);
    printf("get %s\n", buf);
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
    int a = pocky_udp_socket(6000);
    struct pocky_base *base = pocky_init();
    pocky_add_ev(a, base, recv_cb, NULL);

    pthread_create(&thread1, NULL, recer, (void*) base);
    //pthread_join(thread1, NULL);
    sleep(10);
    printf("%u\n",pocky_base_size(base));
    pocky_del_ev(base, a);
    a = pocky_udp_socket(8000);
    pocky_add_ev(a, base, recv_cb, NULL);
    a = pocky_udp_socket(9000);
    pocky_add_ev(a, base, recv_cb, NULL);
    sleep(100);
	return 0;
}
