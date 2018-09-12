/*
********************************************************************************
*  Copyright (C), 2009-2013, 合众思壮深圳研发中心
*
*  项目名称：E7759A
*  
*  文件名称: UartProcess.c
*
*  文件描述：串口数据处理程序
*             
*             
*  创 建 者: 韦哲轩
*
*  创建日期：2017-10-9 
*
*  版 本 号：V1.0
*
*  修改记录： 
*             
*      1. 日    期： 
*         修 改 人： 
*         所作修改： 
*      2. ...
********************************************************************************
*/

#include "Obd.h"
#include "Shell.h"
#include <stdlib.h>
#include "Public.h"
#include "Can.h"
#include "CanDrv.h"
#include "Flash.h"
#include "Adc.h"
#include "Delay.h"
#include "string.h"
#include "Delay.h"
#include "UartProcess.h"
//#include "sleep.h"
#include "J1939.h"
/*
********************************************************************************                                                                  
*  外部变量声明                                                                                                       
********************************************************************************
*/
extern u8 batteryvolt[10];
extern bool is_shutdown_can;
extern bool cannelfilter;
/*
********************************************************************************                                                                 
*  内部变量定义                                                                                                         
********************************************************************************
*/

DiagnosticRequestHandle RequestHandle;

CANPACKET sendcanpacket;
PACKET UART3Recvpacket; //定义UART3接收到的报文
UART3DATASTRUCT  UART3Data;//定义接收UART3原始数据

WORK_MODE_T work_mode = COMMAND_MODE;
CHANNEL_TYPE_T channel = CAN_CHANNEL_1;

u8 accon_ack = 0;

extern bool is_open_timer4;
extern bool is_open_timer6;

bool is_accoff_ack  = FALSE;
bool is_accon_ack  = FALSE;
bool is_E9631_Boot  = FALSE;
bool is_Cancel_shutdown = FALSE;
//版本
u8 version[10] = {0x21, 0x00, 0x04,'2','.','1','8'};

//UART波特率
u8 ackbaudrate[6] = {0x31,0x00, 0x01};
//GPIO
u8 gpioack[7] = {0x91,0x00, 0x02};
//模式
u8 modeack[6] = {0x81,0x00, 0x01};
//can 通道
u8 channelack[6] = {0x83,0x00, 0x01};  

u8 ackfitter[10] = {0x50,0x00, 0x01};
u8 ackcancelfitter[6] = {0x51,0x00, 0x01};
/*
********************************************************************************                                                                  
*  内部函数声明                                                                                                        
********************************************************************************
*/
void senddatapacket(COMX * pcom,u8* data,u16 length);
void recvpacketparse(PACKET* Precvpacket);
void SetUartBaudrate(u8 data);
/*
********************************************************************************                                                                  
*  外部函数声明                                                                                                        
********************************************************************************
*/
extern u16 Timer6Count;
extern u8 turnoncmd[6];
extern u8 turnoffcmd[6];
/*
********************************************************************************
*  函数名称: StructInit
*
*  功能描述: 结构体初始化
*            
*
*  输入参数: 无
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void StructInit(void)
{
    UART3Data.Len   = 0;
    UART3Data.State = 0;
	
	  UART3Recvpacket.candata = NULL;
	  //UART3Recvpacket.data = NULL;
	
}

void gpiotest(u8 command)
{
	  u8 status;
	
	  switch(command)
    {

				//RADAR_IN  测试OK
				case 0x12:
				      //RTU_DEBUG("RADAR_IN test!\r\n");
			 				status = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12);
					
							gpioack[3] = 0x12;
              gpioack[4] = status;								
							senddatapacket(&COM3,gpioack,FIXED_LENGTH+2+CHECKSUM);	
							break;

			 //MILEAGE_PWE_EN //测试OK
			 case 0x16:
							//RTU_DEBUG("MILEAGE_PWE_EN test!\r\n");	
							status = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6);
						
							gpioack[3] = 0x16;	
              gpioack[4] = status;								
							senddatapacket(&COM3,gpioack,FIXED_LENGTH+2+CHECKSUM);
							break;
			//MILEAGE_mcuin
			case 0x22:
						  //RTU_DEBUG("MILEAGE_mcuin test!\r\n");
							status = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15);
			
							gpioack[3] = 0x22;
              gpioack[4] = status;								
						  senddatapacket(&COM3,gpioack,FIXED_LENGTH+2+CHECKSUM);
							break;
		 //ACC status
		 case 0x30:	
							//ACC ON
							//RTU_DEBUG("ACC status test!\r\n");
							status = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
							if(0 == status){							
									senddatapacket(&COM3,turnoncmd,FIXED_LENGTH+1+CHECKSUM);
							}
							//ACC OFF
							else if(0 != status)
							{
									senddatapacket(&COM3,turnoffcmd,FIXED_LENGTH+1+CHECKSUM);
							}
							break;
			//gpio input
			case 0x44:
						  status = GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_0);
			
							gpioack[3] = 0x44;
              gpioack[4] = status;								
						  senddatapacket(&COM3,gpioack,FIXED_LENGTH+2+CHECKSUM);
							break;
			}               								
}

/*
********************************************************************************
*  函数名称: UART3_Data_Pro
*
*  功能描述: 处理UART3数据
*            
*
*  输入参数: dat
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void UART3_Data_Pro(u8 dat)
{
    u8 dle;
    u16 i, k;
	  u16 index = 0;
	  u8 cha,chb;
	  u16 CurrentPacketLen = 0;
	  static u8 flag = 0;
	 
    switch(UART3Data.State)
    {
			//处理BINTRANS_DLE和BINTRANS_STX
			case 0:
		       if(UART3Data.Len<1)
					 {
								UART3Data.Dat[UART3Data.Len++] = dat;
					 }
					 //若遇到BINTRANS_DLE，则往下判断
		       if(UART3Data.Dat[0] == BINTRANS_DLE)
					 {			
        			 if(flag==0)
							 {
								  flag=1;
							 }
							 else
							 {
									UART3Data.Dat[UART3Data.Len++] = dat;
									if(UART3Data.Len == 2)
									{
										  //若遇到第二个字节是BINTRANS_STX，则表明可能是一帧数据。
											if(UART3Data.Dat[1] == BINTRANS_STX)
											{
													flag = 0;
													UART3Data.State  = 1;
											}
											else
											{
													flag = 0;
													UART3Data.Len = 0;
											}
									}											
							}
					}
					else
					{
							UART3Data.Len = 0;
					}
          break;
	   case 1:
					UART3Data.Dat[UART3Data.Len++] = dat;
					// 找结尾字节
					if(UART3Data.Dat[UART3Data.Len - 2] == BINTRANS_DLE && 
            UART3Data.Dat[UART3Data.Len - 1] == BINTRANS_ETX)
					{  
            // 转义字节
            for(dle = 0, k = 0, i = 2; i < UART3Data.Len - 2; i++)
            {
							  //将<DLE><DLE>转换为<DLE>
                if(UART3Data.Dat[i] == BINTRANS_DLE)
                {
                    if(dle == 1)
                    {
                        dle = 0;
                        continue;
                    }
                    else
                    {
                        dle = 1;
                    }
                }
                else
                {
                    dle = 0;
                }
                UART3Data.Esc[k++] = UART3Data.Dat[i];
            }
						
						//取当前长度
						ato16(&UART3Data.Esc[1],&CurrentPacketLen);
						if(k == FIXED_LENGTH+CurrentPacketLen+CHECKSUM)
						{
								UART3Data.Len = k;
								UART3Recvpacket.cha = UART3Data.Esc[UART3Data.Len-2];
								UART3Recvpacket.chb = UART3Data.Esc[UART3Data.Len-1];	
						    
								CalculateChecksum(UART3Data.Esc,UART3Data.Len-2,&cha,&chb);
								//如果校检和一致，则对报文进一步处理		 
								if(UART3Recvpacket.cha == cha && UART3Recvpacket.chb == chb)
								{
									 //当前数据包长度(不包括校验)
									 UART3Recvpacket.CurrentLen = UART3Data.Len-2;
									 //将数据包拷贝到recvpacket.data
									 memcpy(UART3Recvpacket.data,UART3Data.Esc,UART3Recvpacket.CurrentLen);
									 UART3Recvpacket.index = 0;	
									 UART3Recvpacket.codecState = CODEC_STATE_PACKET_TYPE;
									 //对UART3报文进行处理
									 recvpacketparse(&UART3Recvpacket);
									 is_shutdown_can = FALSE;
									 UART3Data.Len = 0;
									 UART3Data.State = 0;
							 }
							 else
							 {
									 UART3Data.Len = 0;
									 UART3Data.State = 0;
							 }
						}
						else
						{
								UART3Data.Len = 0;
								UART3Data.State = 0;
						}
			  }
        // 超出最大长度
        if(UART3Data.Len >= BINTRANS_MAXLEN)
        {
            UART3Data.Len   = 0;
            UART3Data.State = 0;
        }
        break;
    default:
        UART3Data.Len   = 0;
        UART3Data.State = 0;
        break;
    }	
    return;
}


/*
********************************************************************************
*  函数名称: senddatapacket
*
*  功能描述: 往串口发送协议数据包
*            
*
*  输入参数: pcom，data，length
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
//length=type(一个字节）+当前长度（两个字节) +数据 +校检和（两个字节）

void senddatapacket(COMX * pcom,u8* data,u16 length)
{
	  u16 i;
	  u8 cha,chb;
  
    if(pcom == NULL && data == NULL && length == 0)
		{
			return;
		}
		
		CalculateChecksum(data,length-2,&cha,&chb);
		data[length-2] = cha;
		data[length-1] = chb;
		
	  ComxPutChar(pcom, BINTRANS_DLE);
		ComxPutChar(pcom, BINTRANS_STX);
		for(i = 0; i < length; i++)
		{
				if(data[i] == BINTRANS_DLE)
				{
						ComxPutChar(pcom, BINTRANS_DLE);
				}
				
				ComxPutChar(pcom, data[i]);
		}				
    ComxPutChar(pcom, BINTRANS_DLE);
	  ComxPutChar(pcom, BINTRANS_ETX);
		
}



/*
********************************************************************************
*  函数名称: recvpacketparse
*
*  功能描述: 接收数据包，并处理。
*            
*
*  输入参数: Precvpacket
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
extern u8 count3;
void recvpacketparse(PACKET* Precvpacket)
{
	  u8 i,temp,Accstatus = 0;
	  u8 iterator = 0;
	  u16 index = 0;
	  u16 canbandrate = 0;
	  u16 CurrentPacketLen = 0;
	  u32 canID,MaskId_t = 0;
	  u8 CanIDTemp[4],MaskIdTemp[4];
	  u32 n=0;
	  u8 MaskID[9];
	  u16 canidstd,MaskId_std;
	
	  //当指针为空，且长度数据长度小于等于0时，退出函数。
    if(Precvpacket == NULL && Precvpacket->CurrentLen <= 0)
		{
			  return;
		}
	 	for(index = 0; index < Precvpacket->CurrentLen; index++)
		{
				iterator = Precvpacket->data[index];
			
			  switch(Precvpacket->codecState)
		    {
					 //报文类型
					 case CODEC_STATE_PACKET_TYPE:
								Precvpacket->PacketType = iterator;
					      Precvpacket->codecState = CODEC_STATE_CURRENT_PACKET_LENGTH;
					  break;
					 //报文当前长度
					 case CODEC_STATE_CURRENT_PACKET_LENGTH:
							  Precvpacket->CurrentPacketLen[Precvpacket->index++] = iterator;
								if(Precvpacket->index == 2)
								{
										Precvpacket->index = 0;
										CurrentPacketLen = 0;
										ato16(Precvpacket->CurrentPacketLen,&CurrentPacketLen);
									
									  if(Precvpacket->PacketType == 0x60 || Precvpacket->PacketType == 0x70)
										{
												Precvpacket->candata = (u8*)malloc(CurrentPacketLen);
											  if(Precvpacket->candata == NULL)
													return;
										}
										Precvpacket->codecState = CODEC_STATE_PAYLOAD;								 
							  }			
							break;
					//有效载荷（应用数据）
					case CODEC_STATE_PAYLOAD:
						 //CAN
					  if(Precvpacket->PacketType == 0x40)
						{		
							  
								Precvpacket->can[Precvpacket->index++] = iterator;
								if(Precvpacket->index == CurrentPacketLen && CurrentPacketLen >= 0)
							  {
									  IWDG_Feed();
									  Accstatus = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
									  if(Accstatus ==0)
										{
											 UartDataToCanData(CurrentPacketLen,Precvpacket->can);
										}
										Precvpacket->index = 0;  
								}

						}
	
						//OBD
						else	if(Precvpacket->PacketType == 0x60)
						{		
								Precvpacket->candata[Precvpacket->index++] = iterator;
								if(Precvpacket->index == CurrentPacketLen && CurrentPacketLen >= 0)
							  {
									  IWDG_Feed();
									  Accstatus = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
									  if(Accstatus ==0)
										{
												RequestHandle.Request.buf = (u8*)malloc(CurrentPacketLen);
												if(RequestHandle.Request.buf == NULL)
													 return;
												memcpy(RequestHandle.Request.buf,Precvpacket->candata,CurrentPacketLen);
												diagnostic_request(&RequestHandle,DiagnosticResponsePro);
												free(Precvpacket->candata);
										}
										Precvpacket->index = 0;  
								}

						}
						//J1939
						else if(Precvpacket->PacketType == 0x70)
						{
								Precvpacket->candata[Precvpacket->index++] = iterator;
							  if(Precvpacket->index == CurrentPacketLen && CurrentPacketLen >= 0)
							  {
									 IWDG_Feed();
									 Accstatus = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);
									 if(Accstatus ==0)
									 {
											RequestHandle.Request.buf = (u8*)malloc(CurrentPacketLen);
											if(RequestHandle.Request.buf == NULL)
												return;
											memcpy(RequestHandle.Request.buf,Precvpacket->candata,CurrentPacketLen);
											J1939_request(&RequestHandle,J1939_ResponsePro);
									 
											free(Precvpacket->candata);
											Precvpacket->index = 0;  
										}											
								}
						}
						//设置ID过滤
					  else if(Precvpacket->PacketType == 0x50)
						{		

								Precvpacket->canid[Precvpacket->index++] = iterator;
								if(Precvpacket->index == CurrentPacketLen && CurrentPacketLen >= 0)
							  {
									  for(i=1;i<9;i++)
									  {
											 if(Precvpacket->canid[i]>='0' && Precvpacket->canid[i]<='9')
											 {
													Precvpacket->canid[i] = Precvpacket->canid[i] -48;
											 }
                       if(Precvpacket->canid[i]>='A' && Precvpacket->canid[i]<='F')
											 {
													Precvpacket->canid[i] = Precvpacket->canid[i] -55;
											 }
											 if(Precvpacket->canid[i]=='X')
											 {
													Precvpacket->canid[i] = 0x00;
												  MaskID[i] = 0x0;
											 }
											 else if(Precvpacket->canid[i]!='X')
											 {
													MaskID[i] = 0xF;
											 } 
										}
										CanIDTemp[0] = Precvpacket->canid[1]<<4 | Precvpacket->canid[2];
										CanIDTemp[1] = Precvpacket->canid[3]<<4 | Precvpacket->canid[4];
									  CanIDTemp[2] = Precvpacket->canid[5]<<4 | Precvpacket->canid[6];
									  CanIDTemp[3] = Precvpacket->canid[7]<<4 | Precvpacket->canid[8];
										
										MaskIdTemp[0] = MaskID[1]<<4 | MaskID[2];
										MaskIdTemp[1] = MaskID[3]<<4 | MaskID[4];
									  MaskIdTemp[2] = MaskID[5]<<4 | MaskID[6];
									  MaskIdTemp[3] = MaskID[7]<<4 | MaskID[8];

										canID = ((CanIDTemp[0] << 24) | (CanIDTemp[1] << 16) | (CanIDTemp[2] << 8) | CanIDTemp[3]);
										MaskId_t = ((MaskIdTemp[0] << 24) | (MaskIdTemp[1] << 16) | (MaskIdTemp[2] << 8) | MaskIdTemp[3]);
										ackfitter[3] = CanIDTemp[0];
										ackfitter[4] = CanIDTemp[1];
										ackfitter[5] = CanIDTemp[2];
										ackfitter[6] = CanIDTemp[3];
										//扩展ID
										if(Precvpacket->canid[0] == 0x04 || Precvpacket->canid[0] == 0x06)
										{
												CAN_SetFilterExt(canID,MaskId_t,Precvpacket->canid[0]);
										}//标准ID									
										else if(Precvpacket->canid[0] == 0x00)
										{
											  canidstd = (u16)(CanIDTemp[2]<<8)|CanIDTemp[3];
											  MaskId_std = (u16)(MaskIdTemp[2]<<8)|MaskIdTemp[3];
												CAN_SetFilterStd(canidstd,Precvpacket->canid[0],MaskId_std);
										}
										else if(Precvpacket->canid[0] == 0x08)
										{
											   J1939_CAN_SetFilterExt_Mutl(canID);
											   cannelfilter = TRUE;
										}
									  ackfitter[3] = count3;
									  senddatapacket(&COM3,ackfitter,FIXED_LENGTH+1+CHECKSUM);
										Precvpacket->index = 0;  
								}

						}
						//取消ID过滤
					  else if(Precvpacket->PacketType == 0x51)
						{	
							  if(channel == CAN_CHANNEL_1)
								{
										setFilter(CAN_CHANNEL_1);
								}
								else if(channel == CAN_CHANNEL_2)
								{
										setFilter(CAN_CHANNEL_2);
								}
							  ackcancelfitter[3] = 0x01;
							  senddatapacket(&COM3,ackcancelfitter,FIXED_LENGTH+1+CHECKSUM);
						}
						else
						{ 
					      Precvpacket->command = iterator;
								switch(Precvpacket->PacketType)
								{
									  
										case 0x31:
												switch(Precvpacket->command)
												{
													//接收到到ACC ON应答
													case 0x01:										
															 is_accon_ack = TRUE;
															 is_accoff_ack = FALSE;
                    					 break;
													//接收到ACC OFF应答
													case 0x00:
														  is_accoff_ack = TRUE;
													    is_accon_ack = FALSE;
												      break;													
												}
												break;
										//接收E7759的控制命令
										case 0x20:
										  switch(Precvpacket->command)
											{
												//获取软件版本命令
												case 0x33:		
                             //RTU_DEBUG("get version!\r\n");													
												   	 senddatapacket(&COM3,version,FIXED_LENGTH+4+CHECKSUM);
												     break;
												//获取电池电压
												case 0x34:	
													   //RTU_DEBUG("batteryvolt!\r\n");
                             ADC1_SAMPLING();
                             if(batteryvolt[6] == 0x00)
                             {															 
																batteryvolt[6] += 0x30;
                             }													 
														 senddatapacket(&COM3,batteryvolt,FIXED_LENGTH+4+CHECKSUM);
														 break;
								
										 }               								
								     break;
								case 0x30:
										  switch(Precvpacket->command)
											{	
												//接收到Android应用取消关机
												case 0x02:	
                             //RTU_DEBUG("Cancel the shutdown!\r\n");
                             is_Cancel_shutdown = TRUE;		
                 					   break;
												//关机
												case 0x03:
													   //RTU_DEBUG("Shut down immediately commond!\r\n");
												     TIM_Cmd(TIM4, ENABLE);		
                             is_open_timer4 = TRUE;	
														 TIM_Cmd(TIM6, DISABLE);
                             is_open_timer6 = FALSE;	
                             Timer6Count = 0;												
														 break;
												//远程升级
												case 0x04:
													   TIM_Cmd(TIM3, DISABLE);
														 TIM_Cmd(TIM5, ENABLE);
														 break;
												//切换到500K命令
												case 0x50:
												     if(work_mode == CAN_MODE)
														 {
															  if(channel == CAN_CHANNEL_1)
																{
																		//RTU_DEBUG("CHANNEL_1  CAN_MODE:Setting 500K!\r\n");
																		canbandrate = 0x1503;  //其中0x1503的1表示通道1,50表示500K，3表示CAN_MODE
																		CAN1Init(&Can1, CANX_BAUD_500K,1);
																}
																else if(channel == CAN_CHANNEL_2)
																{
																	  //RTU_DEBUG("CHANNEL_2  CAN_MODE:Setting 500K!\r\n");
																	  canbandrate = 0x2503;
																	  CAN2Init(&Can2, CANX_BAUD_500K,1);
																}
														 }
														 else if(work_mode == OBD_MODE)
														 {
															 if(channel == CAN_CHANNEL_1)
																{
																	  //RTU_DEBUG("CHANNEL_1  OBD_MODE:Setting 500K!\r\n");
																	  canbandrate = 0x1502;
																		CAN1Init(&Can1, CANX_BAUD_500K,0);
															  }
															  else if(channel == CAN_CHANNEL_2)
																{
																	 	//RTU_DEBUG("CHANNEL_2  OBD_MODE:Setting 500K!\r\n");
																	  canbandrate = 0x2502;
																		CAN2Init(&Can2, CANX_BAUD_500K,0); 
																}
														 }
														 MemWriteData(PAGE_ADDR+8,&canbandrate,1);
												     DelayMS(4000);
												     ackbaudrate[3] = 0x50;
														 senddatapacket(&COM3,ackbaudrate,FIXED_LENGTH+1+CHECKSUM);
														 break;
												//切换到250K命令
												case 0x25:	 
												     if(work_mode == CAN_MODE)
														 {
															  if(channel == CAN_CHANNEL_1)
																{
																	  //RTU_DEBUG("CHANNEL_1   CAN_MODE:Setting 250K!\r\n");
																	  canbandrate = 0x1253;
																		CAN1Init(&Can1, CANX_BAUD_250K,1);
																}
																else if(channel == CAN_CHANNEL_2)
																{
																	  //RTU_DEBUG("CHANNEL_2   CAN_MODE:Setting 250K!\r\n");
																	  canbandrate = 0x2253;
																		CAN2Init(&Can2, CANX_BAUD_250K,1);
																}
														 }
														 else if(work_mode == OBD_MODE)
														 {
															 	if(channel == CAN_CHANNEL_1)
																{
																	  //RTU_DEBUG("CHANNEL_1  OBD_MODE:Setting 250K!\r\n");
																	  canbandrate = 0x1252;
																		CAN1Init(&Can1, CANX_BAUD_250K,0);
															  }
															  else if(channel == CAN_CHANNEL_2)
																{
																	 	//RTU_DEBUG("CHANNEL_2  OBD_MODE:Setting 250K!\r\n");
																	  canbandrate = 0x2252;
																		CAN2Init(&Can2, CANX_BAUD_250K,0); 
																}
														 }
														 else if(work_mode == J1939_MODE)
														 {
															  if(channel == CAN_CHANNEL_1)
																{
																	  //RTU_DEBUG("CHANNEL_1   J1939_MODE:Setting 250K!\r\n");
																		canbandrate = 0x1251;
																		CAN1Init(&Can1, CANX_BAUD_250K,0);
																}
																else if(channel == CAN_CHANNEL_2)
																{
																	  //RTU_DEBUG("CHANNEL_2   J1939_MODE:Setting 250K!\r\n");
																		canbandrate = 0x2251;
																	  CAN2Init(&Can2, CANX_BAUD_250K,0);
																}
														 }
												     MemWriteData(PAGE_ADDR+8,&canbandrate,1);
												     DelayMS(4000);
												     ackbaudrate[3] = 0x25;
												     senddatapacket(&COM3,ackbaudrate,FIXED_LENGTH+1+CHECKSUM);
														 break;
														 //切换到125K命令
												case 0x12:	 
												     if(work_mode == CAN_MODE)
														 {
															  if(channel == CAN_CHANNEL_1)
																{
																	  //RTU_DEBUG("CHANNEL_1   CAN_MODE:Setting 125K!\r\n");
																	  canbandrate = 0x1123;
																		CAN1Init(&Can1, CANX_BAUD_125K,1);
																}
																else if(channel == CAN_CHANNEL_2)
																{
																	  //RTU_DEBUG("CHANNEL_2   CAN_MODE:Setting 125K!\r\n");
																	  canbandrate = 0x2123;
																		CAN2Init(&Can2, CANX_BAUD_125K,1);
																}
														 }

												     MemWriteData(PAGE_ADDR+8,&canbandrate,1);
												     DelayMS(4000);
												     ackbaudrate[3] = 0x12;
												     senddatapacket(&COM3,ackbaudrate,FIXED_LENGTH+1+CHECKSUM);
														 break;
											 //9600波特率
											 case 0x9:
											 //19200波特率
											 case 0x19:
											 //57600波特率
										 	 case 0x57:
											 //115200波特率
											 case 0x11:
											 //230400波特率
											 case 0x23:
											 //460800波特率
											 case 0x46:
												     SetUartBaudrate(Precvpacket->command);
														 break;
										}               								
								    break;
								 case 0x80:
										 switch(Precvpacket->command)
								     {
											  //查询模式
											  case 0x00:
												    //RTU_DEBUG("Inquiry mode!\r\n");
												    switch(work_mode)
												    {
																case COMMAND_MODE:
																	modeack[3] = 0x00;
																	senddatapacket(&COM3,modeack,FIXED_LENGTH+1+CHECKSUM);
																	break;
																case J1939_MODE:
																	modeack[3] = 0x01;
																	senddatapacket(&COM3,modeack,FIXED_LENGTH+1+CHECKSUM);
																	break;
																case OBD_MODE:
																	modeack[3] = 0x02;
																	senddatapacket(&COM3,modeack,FIXED_LENGTH+1+CHECKSUM);
																	break;
																case CAN_MODE:
																	modeack[3] = 0x03;
																	senddatapacket(&COM3,modeack,FIXED_LENGTH+1+CHECKSUM);
																	break;
														}
														break;
											  //CAN模式
												case 0x03:
													  //RTU_DEBUG("Entry CAN mode!\r\n");
													  work_mode = CAN_MODE;
														modeack[3] = 0x03;											
														senddatapacket(&COM3,modeack,FIXED_LENGTH+1+CHECKSUM);
														break;	
											  //OBD模式
												case 0x02:
													  //RTU_DEBUG("Entry OBD mode!\r\n");
													  work_mode = OBD_MODE;
														modeack[3] = 0x02;											
														senddatapacket(&COM3,modeack,FIXED_LENGTH+1+CHECKSUM);
														break;
											  //J1939模式
												case 0x01:
													  //RTU_DEBUG("Entry J1939 mode!\r\n");
													  work_mode = J1939_MODE;
														modeack[3] = 0x01;												
														senddatapacket(&COM3,modeack,FIXED_LENGTH+1+CHECKSUM);
														break;																
										 }
										 break;
								//查询can通道
								case 0x82:
										 switch(Precvpacket->command)
								     {
												case 0x00:
													//RTU_DEBUG("Inquiry can channel!\r\n");
													switch(channel)
													{
															case CAN_CHANNEL_1:
																channelack[3] = CAN_CHANNEL_1;
																senddatapacket(&COM3,channelack,FIXED_LENGTH+1+CHECKSUM);
																break;
															case CAN_CHANNEL_2:
																channelack[3] = CAN_CHANNEL_2;
																senddatapacket(&COM3,channelack,FIXED_LENGTH+1+CHECKSUM);
																break;
													}
													break;
											 case CAN_CHANNEL_1:
													//RTU_DEBUG("Set CAN channel 1!\r\n");
												  channel = CAN_CHANNEL_1;
												  channelack[3] = CAN_CHANNEL_1;												
													senddatapacket(&COM3,channelack,FIXED_LENGTH+1+CHECKSUM);
												  break;
											 case CAN_CHANNEL_2:
													//RTU_DEBUG("Set CAN channel 2!\r\n");
													channel = CAN_CHANNEL_2;
													channelack[3] = CAN_CHANNEL_2;												
													senddatapacket(&COM3,channelack,FIXED_LENGTH+1+CHECKSUM);
													break;
										}
										break;
								//GPIO
								case 0x90:
                     gpiotest(Precvpacket->command);            								
								     break;
							 }
							 //free(Precvpacket->data);
					}			
          break;
				}
		}
}
/*
********************************************************************************
*  函数名称: SetUartBaudrate
*
*  功能描述: 设置UART波特率
*            
*
*  输入参数: data
*
*  输出参数: 无
*
*  返 回 值: 无
*
********************************************************************************
*/
void SetUartBaudrate(u8 data)
{
	  u16 Baud;
	  u32 Baudrate = 0;
	
    if(data == 0x9)
		{
				Baud = 0x9600;
				Baudrate = 9600;
				ackbaudrate[3] = 0x9;
		}
		else if(data == 0x19)
		{
				Baud = 0x1920;
				Baudrate = 19200;
				ackbaudrate[3] = 0x19;
		}
		else if(data == 0x57)
		{
				Baud = 0x5760;
				Baudrate = 57600;
				ackbaudrate[3] = 0x57;
		}
		else if(data == 0x11)
		{
				Baud = 0x1152;
				Baudrate = 115200;
				ackbaudrate[3] = 0x11;
		}
		else if(data == 0x23)
		{
				Baud = 0x2304;
				Baudrate = 230400;
				ackbaudrate[3] = 0x23;
		}
		else if(data == 0x46)
		{
				Baud = 0x4608;
				Baudrate = 460800;
				ackbaudrate[3] = 0x46;
		}
		Com3Init(&COM3, Baudrate, COM_PARA_8N1);
		
		MemWriteData(PAGE_ADDR,&Baud,1);
		DelayMS(1000);
	  senddatapacket(&COM3,ackbaudrate,FIXED_LENGTH+1+CHECKSUM);
}



