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
#define FDSIZE 6000
#define EPOLLEVENTS 6000

#define SOCK_S "sock_server"
#define SOCK_C "sock_client"

#define err_log(errlog) do{perror(errlog);exit(EXIT_FAILURE);}while(0)

static void handle_events(int ep_fd,struct epoll_event *events,int n,
        int ser_fd,char *recv_buff,char *send_buff,unsigned int* counts);

int main(int agrc,char *argv[])
{
        int ser_fd,cli_fd,ret;
        struct sockaddr_un un;
        char recv_buff[BUFFER_SIZE],send_buff[BUFFER_SIZE];
        int reuse_val = 1;

        struct epoll_event events[EPOLLEVENTS],ev;
        int ep_fd,i;
        unsigned int conn_counts = 0;

        memset(&un, 0, sizeof(un));
        un.sun_family = AF_UNIX;
        unlink(SOCK_S);
        strcpy(un.sun_path,SOCK_S);

        ser_fd = socket(AF_UNIX,SOCK_STREAM,0);
        if (ser_fd < 0)
                err_log("Server socket() failed");
        //Set addr reuse
        setsockopt(ser_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&reuse_val,sizeof(reuse_val));
        ret = bind(ser_fd,(struct sockaddr *)&un,sizeof(un));
        if (ret < 0)
                err_log("Server bind() failed");
        ret = listen(ser_fd,MAX_CONNECT_NUM);
        if (ret < 0)
                err_log("Srevet listen() failed");
        printf("Server start\n");
        
        ep_fd = epoll_create(FDSIZE);
        memset(events,0,EPOLLEVENTS);
        ev.events = EPOLLIN;
        ev.data.fd = ser_fd;
        epoll_ctl(ep_fd,EPOLL_CTL_ADD,ser_fd,&ev);

        while (1){
                ret = epoll_wait(ep_fd,events,EPOLLEVENTS,-1);
                if (ret < 0)
                        err_log("Server epoll_wait() failed");
                handle_events(ep_fd,events,ret,ser_fd,recv_buff,
                                         send_buff,&conn_counts);
                printf("Current connect counts: [%d]\n",conn_counts);
        }
        close(ser_fd);
        close(ep_fd);
        return 0;
}

static void handle_events(int ep_fd,struct epoll_event *events,int n,
      int ser_fd,char *recv_buff,char *send_buff,unsigned int *counts)
{
        int i,cli_fd;
        int recv_bytes,send_bytes;
        struct epoll_event ev;
        for (i = 0; i < n; i++){
                if (events[i].data.fd == ser_fd){
                        cli_fd = accept(ser_fd,NULL,NULL);
                        if (cli_fd < 0)
                                continue;
                        ev.events = EPOLLIN;
                        ev.data.fd = cli_fd;
                        epoll_ctl(ep_fd,EPOLL_CTL_ADD,cli_fd,&ev);
                        printf("connect client: %d\n",cli_fd);
                        (*counts)++;
                }
                else{
                        memset(recv_buff,0,BUFFER_SIZE);
                        recv_bytes = recv(events[i].data.fd,recv_buff,
                                        BUFFER_SIZE,0);
                        if (recv_bytes < 0)
                                err_log("Server recv() failed");
                        else if (recv_bytes == 0){
                                epoll_ctl(ep_fd,EPOLL_CTL_DEL,
                                                events[i].data.fd,NULL);
                                close(events[i].data.fd);
                                printf("closed client: %d\n",
                                                events[i].data.fd);
                                (*counts)--;
                        }
                        else{
                                recv_buff[recv_bytes] = '\0';
                                printf("Server recv: %s\n",recv_buff);
                                sprintf(send_buff,"Welcome client: %d",
                                                events[i].data.fd);
                                printf("Server send: %s\n",send_buff);
                                send_bytes = send(events[i].data.fd,
                                         send_buff,strlen(send_buff),0);
                                if (send_bytes < 0)
                                        err_log("Server send() failed");
                        }
                }
        }
}
