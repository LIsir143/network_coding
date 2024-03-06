#include "server.h"

CTcpServer TcpServer;
 
// SIGINT和SIGTERM的处理函数
void EXIT(int sig)
{
  printf("程序退出，信号值=%d\n",sig);
 
  close(TcpServer.m_listenfd);  // 手动关闭m_listenfd，释放资源
 
  exit(0);
}
 
// 与客户端通信线程的主函数
void *pth_main(void *arg);
 
int main()
{
  // 忽略全部的信号
  for (int ii=0;ii<50;ii++) signal(ii,SIG_IGN);
 
  // 设置SIGINT和SIGTERM的处理函数
  signal(SIGINT,EXIT); signal(SIGTERM,EXIT);
 
  if (TcpServer.InitServer(5001)==false)
  { printf("服务端初始化失败，程序退出。\n"); return -1; }
 
  while (1)
  {
    if (TcpServer.Accept() == false) continue;
 
    pthread_t pthid;   // 创建一线程，与新连接上来的客户端通信
    if (pthread_create(&pthid,NULL,pth_main,(void*)((long)TcpServer.m_clientfd))!=0)
    { printf("创建线程失败，程序退出。n"); return -1; }
 
    printf("与客户端通信的线程已创建。\n");
  }
}
