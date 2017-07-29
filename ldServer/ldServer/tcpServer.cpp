//
//  tcpServer.cpp
//  ldServer
//
//  Created by Shabre on 2017. 7. 20..
//  Copyright © 2017년 Shabre. All rights reserved.
//

#include "tcpServer.h"
#define MAXLINE 1023

pthread_t tid[10000];
bool thread_usable[10000];

struct t_arg{
    int accept_socket;
    int thread_num;
};

int getThreadID(){//사용가능한 thread id 검색
    for(int i=0; i<10000; i++){
        if(thread_usable[i]){
            thread_usable[i]=false;
            return i;
        }
    }
    return -1;
}

void *connect_client(void *arg){
    struct t_arg args=*((struct t_arg *)arg);
    int nbyte;
    char buf[MAXLINE+1];
    puts("Client connected..");
    std::queue<struct Pos_packet *> pQueue;
    std::queue<char *> tQueue;
    while((nbyte = read(args.accept_socket, buf, MAXLINE))>0){//연결 끊길때까지 계속 수신
        std::cout<<nbyte<<" byte received"<<std::endl;
        bufToPacket(buf, nbyte, &pQueue, &tQueue);//수신한 데이터를 packet으로 변경
        sendPacket(args.accept_socket, &pQueue, &tQueue);//packet을 처리
    }
    close(args.accept_socket);
    pthread_exit(NULL);
}


void tcp_server(char *port){
    int avail;
    struct t_arg pass_arg;
    for(int i=0; i<10000; i++)
        thread_usable[i]=true;
    struct sockaddr_in servaddr, cliaddr;
    int tcp_listen_sock, accp_sock;
    socklen_t addrlen=sizeof(cliaddr);
    
    if((tcp_listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail"); exit(0);
    }
    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr= htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(port));
    // binding
    if(bind(tcp_listen_sock, (struct sockaddr *)&servaddr, sizeof(servaddr))<0)
    {
        perror("bind fail"); exit(0);
    }
    listen(tcp_listen_sock, 5);
    while(1) {
        //puts("Waiting clients to connect.."); // 연결요청을 기다림
        accp_sock=accept(tcp_listen_sock, (struct sockaddr *)&cliaddr, &addrlen);
        if(accp_sock < 0) {
            perror("accept fail");
            exit(0);
        }
        avail=getThreadID();//사용가능한 thread id 가져온다
        if(avail>=0){//사용가능한 thread id가 있으면
            pass_arg.accept_socket=accp_sock;
            pass_arg.thread_num=avail;
            pthread_create(&tid[avail], NULL, connect_client, &pass_arg);
            //pthread_join(tid[avail], NULL);
        }
        else{//사용가능한 thread id가 없으면
            perror("no avail thread");
            close(accp_sock);
            continue;
        }
    }
    close(tcp_listen_sock);
}