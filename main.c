#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "pocky.h"
int recv_cb(int fd, short event, void *pdata)
{
    struct sockaddr_in cliaddr;
    char buf[1024];
    socklen_t len;
    memset(buf, 0, 1024);
    recvfrom(fd, buf, 1024, 0, (struct sockaddr *)&cliaddr, &len);
    printf("get %s\n", buf);
    return 0;
}
int main()
{
    int a = pocky_udp_socket(6000);
    struct pocky_base *base = pocky_init();
    pocky_add_ev(a, base, recv_cb, NULL);

    pocky_base_loop(base);
    pocky_destroy_base(base);
	return 0;
}
