#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "pthread.h"
#include "unistd.h"
#include "sys/socket.h"
#include "sys/un.h"
#include "stddef.h"
#include "fcntl.h"
#include "sys/select.h"
#include "aio.h"
#include "errno.h"
#include "signal.h"

#define BUFFER_SIZE 128
#define MAX_CONNECT_NUM 5

#define SOCK_S "sock_server"
#define SOCK_C "sock_client"

#define err_log(errlog) do{perror(errlog);exit(EXIT_FAILURE);}while(0)


int main(int argc,char *argv[])//Client Synchronous Blocking
{
        int fd,ret,recv_bytes,send_bytes;
        struct sockaddr_un ser_addr;
        char recv_buff[BUFFER_SIZE],send_buff[BUFFER_SIZE];
        memset(&ser_addr,0,sizeof(ser_addr));
        ser_addr.sun_family = AF_UNIX;
        strcpy(ser_addr.sun_path,SOCK_S);

        fd = socket(AF_UNIX,SOCK_STREAM,0);
        if (fd < 0)
                err_log("Client socket() failed");
        ret = connect(fd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
        if (ret < 0)
                err_log("Client connent() failed");
        while (1){
                memset(send_buff,0,sizeof(send_buff));
                printf("Input('end' to exit): ");
                fgets(send_buff,BUFFER_SIZE,stdin);
                if (strncmp(send_buff,"end",3) == 0)
                        break;
                //strcpy(send_buff,"I am synchronous block");
                printf("Client send: %s",send_buff);
                send_bytes = send(fd,send_buff,BUFFER_SIZE,0);
                if (send_bytes < 0)
                        err_log("Client send() failed");
                memset(recv_buff,0,sizeof(recv_buff));
                recv_bytes = recv(fd,recv_buff,BUFFER_SIZE,0);
                if (recv_bytes < 0)
                        err_log("Client recv() failed");
                recv_buff[recv_bytes] = '\0';
                printf("Client recv: %s\n",recv_buff);
        }
        close(fd);
        return 0;
}
