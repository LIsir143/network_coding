#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include<sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAXEVENTS 100
#define SERVER_STRING "Server: jdbhttpd/0.1.0\r\n"

int setnonblocking(int sockfd);
int initserver(int port);
void not_found(int client)
{
 char buf[1024];

 sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, SERVER_STRING);
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "Content-Type: text/html\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "your request because the resource specified\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "is unavailable or nonexistent.\r\n");
 send(client, buf, strlen(buf), 0);
 sprintf(buf, "</BODY></HTML>\r\n");
 send(client, buf, strlen(buf), 0);
}
void send_respond(int cfd, int no, char *disp, char *type, int len)
{
	char buf[4096] = {0};
	
	sprintf(buf, "HTTP/1.1 %d %s\r\n", no, disp);
	send(cfd, buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "Content-Type:%s\r\n", type);
	sprintf(buf+strlen(buf), "Content-Length:%d\r\n", len);
	send(cfd, buf, strlen(buf), 0);
	
	send(cfd, "\r\n", 2, 0);
}
void send_file(int cfd, const char *file)
{
	int n = 0;
	char buf[4096] = {0};
	
	// 打开的服务器本地文件。  --- cfd 能访问客户端的 socket
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		// 404 错误页面
		perror("open error");
		exit(1);
	}
	
	while ((n = read(fd, buf, sizeof(buf))) > 0) {		
		send(cfd, buf, n, 0);
	}
	
	close(fd);		
}
void http_request(int cfd,const char *file)
{
	struct stat sbuf;
	// 判断文件是否存
	int ret = stat(file, &sbuf);
	if (ret == -1) {
    not_found(cfd);    
     return;
		// 回发浏览器 404 错误页面
	}
	
	if(S_ISREG(sbuf.st_mode)) {		// 是一个普通文件
		
		// 回发 http协议应答
		send_respond(cfd, 200, "OK", "text/plain; charset=iso-8859-1", sbuf.st_size);	 
		//send_respond(cfd, 200, "OK", "image/jpeg", -1);
		//send_respond(cfd, 200, "OK", "audio/mpeg", -1);
		
		// 回发 给客户端请求数据内容。
		send_file(cfd, file);
	}	
}
int get_line(int sock, char *buf, int size)
{
 int i = 0;
 char c = '\0';
 int n;

 //读取到的字符个数大于size或者读到\n结束循环
 while ((i < size - 1) && (c != '\n'))
 {
  n = recv(sock, &c, 1, 0); //接收一个字符
  /* DEBUG printf("%02X\n", c); */
  if (n > 0)
  {
   if (c == '\r')//回车字符
   {
	/*使用 MSG_PEEK 标志使下一次读取依然可以得到这次读取的内容，可认为接收窗口不滑动*/  
    n = recv(sock, &c, 1, MSG_PEEK);
    /* DEBUG printf("%02X\n", c); */
	
	//读取到回车换行
    if ((n > 0) && (c == '\n'))
     recv(sock, &c, 1, 0);//还需要读取，因为之前一次的读取，相当于没有读取
    else
     c = '\n';//如果只读取到\r，也要终止读取
   }
   //没有读取到\r,则把读取到内容放在buf中
   buf[i] = c;
   i++;
  }
  else
   c = '\n';
 }
 
 buf[i] = '\0';
 
 return i; 
 //返回读取到的字符个数,不包括\0
}
void disconnect(int cfd,int epfd)
{
  int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
    if(ret == -1) {   
        perror("epoll_ctl del cfd error");
        exit(1);
    }
    close(cfd);
}
void do_read(int cfd,int epfd)
{
  // 将浏览器发过来的数据, 读到buf中 
    char buffer[1024];
    char method[16], path[256], protocol[16];
    memset(buffer,0,sizeof(buffer));
    // 读请求行
    int len=get_line(cfd,buffer,sizeof(buffer));
    if(len == 0) {   
        printf("客户端断开了连接...\n");
        // 关闭套接字, cfd从epoll上del
        disconnect(cfd,epfd);
     } else{
      sscanf(buffer, "%[^ ] %[^ ] %[^ ]", method, path, protocol);	
		  printf("method=%s, path=%s, protocol=%s\n", method, path, protocol);
		
		while (1) {
			char buf[1024] ;
      memset(buf,0,sizeof(buf));
			len = get_line(cfd, buf, sizeof(buf));
      if(len =='\n'){
        break;
      }else if(len == 1){
        break;
      }
      }
      if (strncasecmp(method, "GET", 3) == 0)
	{
		char *file = path+1;   // 取出 客户端要访问的文件名
		http_request(cfd,file);
	}
}
 }
int main(int argc,char *argv[])
{
  if (argc < 3)
  {
    printf("usage:./tcpepoll port path\n"); return -1;
  }
    int listensock = initserver(atoi(argv[1]));
  printf("listensock=%d\n",listensock);
   int ret = chdir(argv[2]);
   if(ret != 0){
    perror("chdir error");
    exit(1);
   }

  if (listensock < 0)
  {
    printf("initserver() failed.\n"); return -1;
  }

  int epollfd;

  char buffer[1024];
  memset(buffer,0,sizeof(buffer));

  // ����һ��������
  epollfd = epoll_create(1);

  // ���Ӽ����������¼�
  struct epoll_event ev,events[MAXEVENTS];
  ev.data.fd = listensock;
  ev.events = EPOLLIN;
  epoll_ctl(epollfd,EPOLL_CTL_ADD,listensock,&ev);

  while (1)
  {
    // �ȴ����ӵ�socket���¼�������
    int infds = epoll_wait(epollfd,events,MAXEVENTS,-1);
    // printf("epoll_wait infds=%d\n",infds);

    // ����ʧ�ܡ�
    if (infds < 0)
    {
      printf("epoll_wait() failed.\n"); perror("epoll_wait()"); break;
    }

    // ��ʱ��
    if (infds == 0)
    {
      printf("epoll_wait() timeout.\n"); continue;
    }

    // �������¼������Ľṹ���顣
    for (int ii=0;ii<infds;ii++)
    {
      if ((events[ii].data.fd == listensock) &&(events[ii].events & EPOLLIN))
      {
        // ��������¼�����listensock����ʾ���µĿͻ�����������
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientsock = accept(listensock,(struct sockaddr*)&client,&len);
        if (clientsock < 0)
        {
          printf("accept() failed.\n"); continue;
        }

        // ���µĿͻ������ӵ�epoll�С�
        memset(&ev,0,sizeof(struct epoll_event));
        ev.data.fd = clientsock;
        ev.events = EPOLLIN;
        epoll_ctl(epollfd,EPOLL_CTL_ADD,clientsock,&ev);

        printf ("client(socket=%d) connected ok.\n",clientsock);

        continue;
      }
      else if (events[ii].events & EPOLLIN)
      {
        do_read(events[ii].data.fd,epollfd);       
      }
    }
  }

  close(epollfd);

  return 0;
}
int initserver(int port)
{
  int sock = socket(AF_INET,SOCK_STREAM,0);
  if (sock < 0)
  {
    printf("socket() failed.\n"); return -1;
  }

  // Linux����
  int opt = 1; unsigned int len = sizeof(opt);
  setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);
  setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,&opt,len);

  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  if (bind(sock,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
  {
    printf("bind() failed.\n"); close(sock); return -1;
  }

  if (listen(sock,5) != 0 )
  {
    printf("listen() failed.\n"); close(sock); return -1;
  }

  return sock;
}
int setnonblocking(int sockfd)
{  
  if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)  return -1;

  return 0;  
}  
