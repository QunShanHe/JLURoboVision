/*
*	@Author: PingLin Zhang
*	@Date:	 2020.04.13
*	@Brief:  Serial
*/

#ifndef  _USART_H
#define  _USART_H
 
//串口相关的头文件    
#include<stdio.h>      /*标准输入输出定义*/    
#include<stdlib.h>     /*标准函数库定义*/    
#include<unistd.h>     /*Unix 标准函数定义*/    
#include<sys/types.h>     
#include<sys/stat.h>       
#include<fcntl.h>      /*文件控制定义*/    
#include<termios.h>    /*PPSIX 终端控制定义*/    
#include<errno.h>      /*错误号定义*/    
#include<string.h>    
     
     
//宏定义    
#define FALSE  -1    
#define TRUE   0
extern int UART0_Open(int fd,char*port);
extern void UART0_Close(int fd) ;
extern int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
extern int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity) ;
extern int UART0_Recv(int fd, char *rcv_buf,int data_len);
extern int UART0_Send(int fd, char *send_buf,int data_len);
//串口主函数
int Serial(int yaw,int pitch,bool fire,bool find);
#endif



