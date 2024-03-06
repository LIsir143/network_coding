#include "client.h"
int main()
{
  CTcpClient TcpClient;
 
  // 向服务器发起连接请求
  if (TcpClient.ConnectToServer("192.168.26.131",5001)==false)
  { printf("TcpClient.ConnectToServer(\"172.16.0.15\",5051) failed,exit...\n"); return -1; }
 
  char strbuffer[1024];
 
  for (int ii=0;ii<50;ii++)
  {
    memset(strbuffer,0,sizeof(strbuffer));
    sprintf(strbuffer,"这是第%d个学生，编号%03d。",ii+1,ii+1);
    if (TcpClient.Send(strbuffer,strlen(strbuffer))<=0) break;
    printf("发送：%s\n",strbuffer);
   
    memset(strbuffer,0,sizeof(strbuffer));
    if (TcpClient.Recv(strbuffer,sizeof(strbuffer))<=0) break;
    printf("接收：%s\n",strbuffer);
 
    sleep(1);  // sleep一秒，方便观察程序的运行。
  }
}
