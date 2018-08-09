#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event.h>
#include <event2/util.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
using std::cout;
using std::cin;
using std::endl;

typedef struct sockaddr SA;


int tcp_connect_server(const char * server_ip,int port);
void cmd_msg_cb(int fd,short events,void *arg);
void socket_read_cb(int fd,short events,void *arg);


int main(int argc,char**argv)
{
	//让客户选择需要连接的服务器
	if(argc<3)
	{
		cout<<"you need input  IP and PORT"<<endl;
		return -1;
	}

	int ret=tcp_connect_server(argv[1],atoi(argv[2]));
	if(-1==ret)
	{
		cout<<"tcp_connect error"<<endl;
		return -1;
	}

	cout<<"connect to server successful"<<endl;
	
	//开始设置event一系列操作
	struct event_base *base=event_base_new();	
	struct event * ev_sockfd=event_new(base,ret,EV_READ | EV_PERSIST,socket_read_cb,NULL);
	event_add(ev_sockfd,NULL);

	//传入一个stdin句柄
	struct event * ev_cmd=event_new(base,STDIN_FILENO,EV_READ|EV_PERSIST,cmd_msg_cb,(void*)&ret);
	event_add(ev_cmd,NULL);

	event_base_dispatch(base);

	cout<<"finished"<<endl;
	return 0;
}

void cmd_msg_cb(int fd,short events,void *arg)
{
	char msg[1024]={0};

	int ret=read(fd,msg,sizeof(msg));
	if(0>=ret)
	{
		cout<<"read fail"<<endl;
		exit(1);
	}

	int sockfd=*((int*)arg);

	write(sockfd,msg,ret);
}
void socket_read_cb(int fd,short events,void *arg)
{
	char msg[1024];
	int len=read(fd,msg,sizeof(msg)-1);
	if(0>=len)
	{
		cout<<"read fail"<<endl;
		exit(0);
	}
	msg[len]='\0';
	cout<<msg<<endl;
}

int tcp_connect_server(const char * server_ip,int port)
{
	int sockfd,status,save_errno;
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));

	server_addr.sin_port=htons(port);
	server_addr.sin_family=AF_INET;
	status=inet_pton(AF_INET,server_ip,&(server_addr.sin_addr));
	if(0>=status)
	{
		cout<<"server_ip values is error format"<<endl;
		return -1;
	}

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(-1==sockfd)
	{
		cout<<"create sokcet is error"<<endl;
		return -1;
	}

	if(-1==connect(sockfd,(SA*)&server_addr,sizeof(server_addr)))
	{
		cout<<"connect is error"<<endl;
		save_errno=errno;
		close(sockfd);
		errno=save_errno;
		return -1;
	}

	evutil_make_socket_nonblocking(sockfd);

	return sockfd;
}
