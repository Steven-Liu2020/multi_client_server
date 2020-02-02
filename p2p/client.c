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
#include "sys/epoll.h"

#define BUFFER_SIZE 128
#define MAX_CONNECT_NUM 10
#define FDSIZE 1000
#define EPOLLEVENTS 20

#define SOCK_S "sock_server"
#define SOCK_C "sock_client"

#define err_log(errlog) do{perror(errlog);exit(EXIT_FAILURE);}while(0)

static void handle_events(int ep_fd,struct epoll_event *events,int n,
                        int ser_fd,char *recv_buff,char *send_buff);

static void read_data(int ep_fd,int fd,int cli_fd,
                char *recv_buff,char *send_buff);

int main(int agrc,char *argv[])
{
        int ser_fd,cli_fd,ret;
        struct sockaddr_un ser_addr;
        char recv_buff[BUFFER_SIZE],send_buff[BUFFER_SIZE];
        int reuse_val = 1;

        struct epoll_event events[EPOLLEVENTS],ev;
        int ep_fd,i;
        unsigned int conn_counts = 0;

        memset(&ser_addr, 0, sizeof(ser_addr));
        ser_addr.sun_family = AF_UNIX;
        strcpy(ser_addr.sun_path,SOCK_S);

        cli_fd = socket(AF_UNIX,SOCK_STREAM,0);
        if (cli_fd < 0)
                err_log("Client socket() failed");
        ret = connect(cli_fd,(struct sockaddr *)&ser_addr,
                        sizeof(ser_addr));
        if (ret < 0)
                err_log("Client connect() failed");
        ep_fd = epoll_create(FDSIZE);
        memset(events,0,EPOLLEVENTS);

        ev.events =  EPOLLIN;
        ev.data.fd = cli_fd;
        epoll_ctl(ep_fd,EPOLL_CTL_ADD,cli_fd,&ev);

        ev.data.fd = STDIN_FILENO;
        epoll_ctl(ep_fd,EPOLL_CTL_ADD,STDIN_FILENO,&ev);

        while (1){
                ret = epoll_wait(ep_fd,events,EPOLLEVENTS,-1);
                if (ret < 0)
                        err_log("Client epoll_wait() failed");
                handle_events(ep_fd,events,ret,cli_fd,recv_buff,send_buff);
        }
        close(cli_fd);
        close(ep_fd);
        return 0;
}

static void handle_events(int ep_fd,struct epoll_event *events,int n,
                        int cli_fd,char *recv_buff,char *send_buff)
{
        int i,fd;
        int recv_bytes,send_bytes;
        struct epoll_event ev;
        for (i = 0; i < n; i++){
                fd = events[i].data.fd;
                if (events[i].events & EPOLLIN)
                        read_data(ep_fd,fd,cli_fd,recv_buff,send_buff);
        }
}

static void read_data(int ep_fd,int fd,int cli_fd,
                char *recv_buff,char *send_buff)
{
        int recv_bytes,ret;
        struct epoll_event ev;
        memset(recv_buff,0,BUFFER_SIZE);
        recv_bytes = read(fd,recv_buff,BUFFER_SIZE);
        if (recv_bytes < 0)
                err_log("Client read() failed");
        else if (recv_bytes == 0){
                close(fd);
                printf("server closed: %d\n",fd);
        }
        else{
                if (fd == cli_fd){
                        recv_buff[recv_bytes] = '\0';
                        printf("Client recv: %s\n",recv_buff);
                }else if(fd == STDIN_FILENO){
                        if (strncmp(recv_buff,"end",3) == 0){
                                close(cli_fd);
                                close(ep_fd);
                                err_log("Exit");
                        }
                        printf("Client send: %d\n",recv_bytes);
                        strcpy(send_buff,recv_buff);
                        ret = send(cli_fd,send_buff,strlen(send_buff)-1,0);
                        if (ret < 0)
                                err_log("Send failed");
                }
        }
        return;
}

