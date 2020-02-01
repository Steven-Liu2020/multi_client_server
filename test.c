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
#define PROCESS 20
#define THREAD_NUMS 300
#define CLIENT_NUMS PROCESS*THREAD_NUMS

#define SOCK_S "sock_server"
#define SOCK_C "sock_client"

#define err_log(errlog) do{perror(errlog);exit(EXIT_FAILURE);}while(0)

void *client_thread(void *arg);
void create_thread(void);

int main(int argc,char *argv[])
{
        int i;
        pid_t pid;
        for (i = 0; i < PROCESS; i++){
                pid = fork();
                if(pid != 0)
                        continue;
                else{
                        create_thread();
                        break;
                }
                usleep(10000);
        }
        while(1);
        return 0;
}

void create_thread(void)
{
        int i,ret;
        for (i = 0; i < THREAD_NUMS; i++){
                pthread_t tid;
                ret = pthread_create(&tid,NULL,client_thread,NULL);
                pthread_detach(tid);
                if (ret != 0)
                        err_log("client thread create failed");
                usleep(10000);
        }
}

void *client_thread(void *arg)//Client Synchronous Blocking
{
        int i,fd,ret;
        int recv_bytes,send_bytes;
        struct sockaddr_un ser_addr;
        char recv_buff[BUFFER_SIZE],send_buff[BUFFER_SIZE];
        memset(&ser_addr,0,sizeof(ser_addr));
        ser_addr.sun_family = AF_UNIX;
        strcpy(ser_addr.sun_path,SOCK_S);

        fd = socket(AF_UNIX,SOCK_STREAM,0);
        if (fd < 0)
                err_log("Client socket() failed");
        while (connect(fd,(struct sockaddr *)&ser_addr,sizeof(ser_addr))<0);
        
        //if (ret < 0)
        //        err_log("Client connent() failed");
        for (;;){
                memset(send_buff,0,sizeof(send_buff));
                //printf("Input: ");
                //fgets(send_buff,BUFFER_SIZE,stdin);
                //if (strncmp(send_buff,"end",3) == 0)
                //        break;
                strcpy(send_buff,"I am client");
                printf("Client send: %s\n",send_buff);
                send_bytes = send(fd,send_buff,BUFFER_SIZE,0);
                if (send_bytes < 0)
                        err_log("Client send() failed");
                memset(recv_buff,0,sizeof(recv_buff));
                recv_bytes = recv(fd,recv_buff,BUFFER_SIZE,0);
                if (recv_bytes < 0)
                        err_log("Client recv() failed");
                recv_buff[recv_bytes] = '\0';
                printf("Client recv: %s\n",recv_buff);
                usleep(1);
        }
        close(fd);
        return NULL;
}
