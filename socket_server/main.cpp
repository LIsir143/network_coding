#include "server.h"
int main()
{
  CTcpServer TcpServer;
 
  if (TcpServer.InitServer(5001)==false)
  { printf("TcpServer.InitServer(5001) failed,exit...\n"); return -1; }
 
  if (TcpServer.Accept() == false) { printf("TcpServer.Accept() failed,exit...\n"); return -1; }
 
  printf("客户端已连接。\n");
 
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
}
