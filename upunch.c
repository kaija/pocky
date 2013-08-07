#include <stdio.h>
#include "pocky.h"

int main()
{
    char buf[100];
    int i = 0;
    while(1){
        sprintf(buf, "packet count %d\n", i);
        i++;
		printf("%s send\n", buf);
        printf("return %d\n",pocky_udp_sender("127.0.0.1", 9000, buf, strlen(buf)));
    }
    return 0;
}
