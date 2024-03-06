#include "server.h"

CTcpServer TcpServer;
 
int main()
{
  //signal(SIGCHLD,SIG_IGN);  // 忽略子进程退出的信号，避免产生僵尸进程
 
  if (TcpServer.InitServer(5001)==false)
  { printf("服务端初始化失败，程序退出。\n"); return -1; }
 
  while (1)
  {
    if (TcpServer.Accept() == false) continue;
 
    if (fork()>0) { TcpServer.CloseClient(); continue; }  // 父进程回到while，继续Accept。
 
    // 子进程负责与客户端进行通信，直到客户端断开连接。
    TcpServer.CloseListen();
 
    printf("客户端已连接。\n");
 
    // 与客户端通信，接收客户端发过来的报文后，回复ok。
    char strbuffer[1024];
 
    while (1)
    {
      memset(strbuffer,0,sizeof(strbuffer));
      if (TcpServer.Recv(strbuffer,sizeof(strbuffer))<=0) break;
      printf("接收：%s\n",strbuffer);
 
      strcpy(strbuffer,"ok");
      if (TcpServer.Send(strbuffer,strlen(strbuffer))<=0) break;
      printf("发送：%s\n",strbuffer);
    }
 
    printf("客户端已断开连接。\n");
 
    return 0;  // 或者exit(0)，子进程退出。
  }
}
