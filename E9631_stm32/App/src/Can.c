/*
********************************************************************************
*  Copyright (C), 2009-2013, 合众思壮西安研发中心
*
*  项目名称：xxxx
*  
*  文件名称: xxxx.c
*
*  文件描述：xxxx
*             
*             
*  创 建 者: 皇海明
*
*  创建日期：2013-03-18 
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
#include "Can.h"
#include "Uart.h"
#include "Shell.h"
#include "Delay.h"
#include "CanDrv.h"
#include "Obd.h"
#include "Public.h"
#include "J1939.h"
#include "bsp_iwdg.h"
#include "UartProcess.h"
#include "String.h"
#include "Flash.h"
extern CANPACKET sendcanpacket;
extern CHANNEL_TYPE_T channel;
extern DiagnosticRequestHandle* PrequestHandle;
extern u8 Cantransmitstute;
extern u8 J1939_ack[20];
extern u32 J1939ExtIdArray[10];
extern bool cannelfilter;
void Can1Test(void)
{
	  CANQUEDATA candata;
	
	  candata.Dlc = 0x08;
		candata.Id = 0x7df;
		
	  candata.Data[0] = 0x01;
	  candata.Data[1] = 0x02;
	  candata.Data[2] = 0x03;
	  candata.Data[3] = 0x04;
	  candata.Data[4] = 0x05;
	  candata.Data[5] = 0x06;
	  candata.Data[6] = 0x07;
	  candata.Data[7] = 0x08;
	  CanTransmit(&Can1, &candata);
}

void Can2Test(void)
{
	  CANQUEDATA candata;
	
	  candata.Dlc = 0x08;
		candata.Id = 0x7df;
		
	  candata.Data[0] = 0x01;
	  candata.Data[1] = 0x02;
	  candata.Data[2] = 0x03;
	  candata.Data[3] = 0x04;
	  candata.Data[4] = 0x05;
	  candata.Data[5] = 0x06;
	  candata.Data[6] = 0x07;
	  candata.Data[7] = 0x08;
	  CanTransmit(&Can2, &candata);
}
#if 0
extern u8 frametype;
extern u32 canID[50];

u8 CheckCanReceive(CANQUEDATA* pData)
{
		int ID,i = 0;
		
		//过滤掉帧类型
		if ((pData->Id & 0x06)!=frametype)
		{
			  return 0;
		}
		//标准数据帧或标准远程帧
    if(((pData->Id & 0x06) == 0x00) || ((pData->Id & 0x06) == 0x02))
		{
				ID = pData->Id >> 21;
		}
		//扩展数据帧或扩展远程帧
		else if(((pData->Id & 0x06) == 0x04) || ((pData->Id & 0x06) == 0x06))
		{
				ID = pData->Id >> 3;
			
		}
			
		for(i=0;i<50;i++)
	  {
			 if(ID == canID[i])
			 {
					return 1;
			 }
		}
	  return 0;
}
#endif
/*
********************************************************************************
*  函数名称: CanToUtcp
*
*  功能描述: 将CAN总线数据直接打包成UTCP二进制数据传输协议发送。
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
void CanToUtcp(u8 channel, CANX * pcan, COMX * pcom)
{
    u8 i, IDE, buf[20];
	  u16 CurrentPacketLen = 0;
    CANQUEDATA data;

    if(channel != 1 && channel != 2)
    {
        return;
    }
    
    else if(pcan == NULL || pcom == NULL)
    {
        return;
    }
    
    while(CanReceive(pcan, &data))
    {
			  IWDG_Feed();
				buf[0] = channel;
				//data.RTR若1为远程帧，0为数据帧
				data.RTR = data.Id & 0x00000002;
				buf[1] = (u8)(data.Id >> 24);
				buf[2] = (u8)(data.Id >> 16);
				buf[3] = (u8)(data.Id >> 8);
				buf[4] = (u8)(data.Id >> 0);
				
				buf[5] = data.Dlc;
				for (i = 0; i < data.Dlc; i++)
				{
					 buf[6 + i] = data.Data[i];
				}
			  sendcanpacket.candata[0] = 0x41;
				//数据帧
				if(data.RTR == 0x00000000)
				{
					 CurrentPacketLen = data.Dlc+6;
					 c16toa(&CurrentPacketLen,&sendcanpacket.candata[1]);
					 memcpy(&sendcanpacket.candata[FIXED_LENGTH],buf,14);
				}
				//远程帧
				else if(data.RTR == 0x00000002)
				{
					 CurrentPacketLen = CANCHANNEL+CANID;
					 c16toa(&CurrentPacketLen,&sendcanpacket.candata[1]);
					 memcpy(&sendcanpacket.candata[FIXED_LENGTH],buf,CurrentPacketLen);
				}
				senddatapacket(&COM3,sendcanpacket.candata,FIXED_LENGTH+CurrentPacketLen+CHECKSUM);
    }
}

void UartDataToCanData(u8 len, u8 *Data)
{
	  u8 i;
		CANQUEDATA data;
		if(Data == NULL && len == 0)
		{
			 return;
		}
		memset(&data, 0, sizeof(CANQUEDATA));
		data.Id = ((Data[1] << 24) | (Data[2] << 16) | (Data[3] << 8) | Data[4]);
		data.Dlc = Data[5];
	  
  	//判断是数据帧CAN_RTR_Data,还是远程帧CAN_RTR_Remote
    
		data.RTR = data.Id & 0x00000002;
    data.IDE = data.Id & 0x00000004;
	
		for(i = 0; i <data.Dlc; i++)
		{
			data.Data[i] = Data[i+6];
		}
		if(channel == CAN_CHANNEL_1)
		{
				CanTransmit(&Can1, &data);
			
    }else if(channel == CAN_CHANNEL_2)
		{
				CanTransmit(&Can2, &data);
		}

}

void OBD_SetFilterStd(u16 id)
{
	 u8 i,num;
	 u16 tmp,mask;
	 u16 StdIdArray[1];

   CAN_FilterInitTypeDef  CAN_FilterInitStructure;
  
   StdIdArray[0] = id;

	 if(channel == CAN_CHANNEL_1)
	 {
			CAN_FilterInitStructure.CAN_FilterNumber = 0;
	 }else if(channel == CAN_CHANNEL_2)
	 {
			CAN_FilterInitStructure.CAN_FilterNumber = 14;
	 }
   CAN_FilterInitStructure.CAN_FilterMode   = CAN_FilterMode_IdMask;
   CAN_FilterInitStructure.CAN_FilterScale  = CAN_FilterScale_32bit;
	 CAN_FilterInitStructure.CAN_FilterIdHigh         = (StdIdArray[0] << 5)&0xffff;
	 CAN_FilterInitStructure.CAN_FilterIdLow          = 0;
	
	 num = sizeof(StdIdArray)/sizeof(StdIdArray[0]);
   for(i=0;i<num;i++)
   {
			tmp = (StdIdArray[0] << 5)&(StdIdArray[i] << 5);
			mask = tmp;
   }
				
	 CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = mask;
   CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0xffff; 
   CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
   CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
   CAN_FilterInit(&CAN_FilterInitStructure);
}

void OBD_SetFilterStd_MUTI()
{
	 u8 i,num;
	 u16 tmp,mask;
	 u16 StdIdArray[8] = {0x7E8,0x7E9,0x7EA,0x7EB,0x7EC,0x7ED,0x7EE,0x7EF};

   CAN_FilterInitTypeDef  CAN_FilterInitStructure;
 
   if(channel == CAN_CHANNEL_1)
	 {
			CAN_FilterInitStructure.CAN_FilterNumber = 0;
	 }
	 else if(channel == CAN_CHANNEL_2)
	 {
			CAN_FilterInitStructure.CAN_FilterNumber = 14;
	 }
   CAN_FilterInitStructure.CAN_FilterMode   = CAN_FilterMode_IdMask;
   CAN_FilterInitStructure.CAN_FilterScale  = CAN_FilterScale_32bit;
	 CAN_FilterInitStructure.CAN_FilterIdHigh         = (StdIdArray[0] << 5)&0xffff;
	 CAN_FilterInitStructure.CAN_FilterIdLow          = 0;
	
	 num = sizeof(StdIdArray)/sizeof(StdIdArray[0]);
   for(i=0;i<num;i++)
   {
			tmp = (StdIdArray[0] << 5)&(StdIdArray[i] << 5);
			mask = tmp;
   }		
	 CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = mask;
   CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0xffff; 
	 CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
   CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
   CAN_FilterInit(&CAN_FilterInitStructure);
}
static u32 count1 = 0;
void CAN_SetFilterStd(u16 StdId,u8 flag,u16 MaskId_std)
{
	 u8 i,num;
	 u16 tmp,mask;
   u16 StdIdArray[50];
   CAN_FilterInitTypeDef  CAN_FilterInitStructure;
 
	 //StdIdArray[count1++] = 0x7E8;
	 //if(count1 >=10)
	 //{
		//	count1 = 0;
	 //}

   if(channel == CAN_CHANNEL_1)
	 {
			CAN_FilterInitStructure.CAN_FilterNumber = 0;
	 }
	 else if(channel == CAN_CHANNEL_2)
	 {
			CAN_FilterInitStructure.CAN_FilterNumber = 14;
	 }
   CAN_FilterInitStructure.CAN_FilterMode   = CAN_FilterMode_IdMask;
   CAN_FilterInitStructure.CAN_FilterScale  = CAN_FilterScale_32bit;
	 CAN_FilterInitStructure.CAN_FilterIdHigh         = (StdIdArray[0] << 5);
	 CAN_FilterInitStructure.CAN_FilterIdLow          = 0; 
		
	 mask = 0x7ff;
	 num = sizeof(StdIdArray)/sizeof(StdIdArray[0]);
   for(i=0;i<num;i++)
   {
			tmp = StdIdArray[i]^(~StdIdArray[0]);
			mask &= tmp;
   }	

	 CAN_FilterInitStructure.CAN_FilterMaskIdHigh     = (mask<<5);	 
	 CAN_FilterInitStructure.CAN_FilterMaskIdLow      = 0;
	 CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
   CAN_FilterInitStructure.CAN_FilterActivation     = ENABLE;
   CAN_FilterInit(&CAN_FilterInitStructure);
}

static u32 count = 0;
static u32 Maskidtemp;

void CAN_SetFilterExt(u32 ExtId,u32 MaskId,u8 flag)
{
	  u8 i,num;
	  u16 MaskIdHigh = 0xFFFF;
	  u16 MaskIdLow = 0xFFFF;
	  u32 mask,tmp;
	  u32 ExtIdArray[10];
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;
   
	  ExtIdArray[count++] = ExtId;
	  if(count >=10)
		{
			 count = 0;
		}
	  if(channel == CAN_CHANNEL_1)
		{
				CAN_FilterInitStructure.CAN_FilterNumber=0;
		}else if (channel == CAN_CHANNEL_2)
	  {
				CAN_FilterInitStructure.CAN_FilterNumber=14;
		}
		CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;//屏蔽位模式
		CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
		
		CAN_FilterInitStructure.CAN_FilterIdHigh=((ExtIdArray[0]<<3)>>16)&0xffff;
		if(flag == 0x04)
		{
			CAN_FilterInitStructure.CAN_FilterIdLow=(ExtIdArray[0]<<3)|CAN_ID_EXT|CAN_RTR_DATA;
		}
		else if(flag == 0x06)
		{
			CAN_FilterInitStructure.CAN_FilterIdLow=(ExtIdArray[0]<<3)|CAN_ID_EXT|CAN_RTR_REMOTE;
		}
	  mask = 0x1fffffff; 
		for(i =0; i<count;i++)
		{  
			tmp =ExtIdArray[i] ^ (~ExtIdArray[0]);  
			mask &=tmp;  
		}  
		mask <<=3;                                  
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh= (mask>>16)&0xffff;
		if(flag == 0x04)
		{
			  CAN_FilterInitStructure.CAN_FilterMaskIdLow= (mask&0xffff)|CAN_ID_EXT|CAN_RTR_DATA;
		}
		else if(flag == 0x06)
		{
				CAN_FilterInitStructure.CAN_FilterMaskIdLow= (mask&0xffff)|CAN_ID_EXT|CAN_RTR_REMOTE;
		}

		
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;
		CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
		CAN_FilterInit(&CAN_FilterInitStructure);

}

void OBD_Send_singleframe(void *handle)
{
	
	  bool Is_Filtered = FALSE;
	  u16 FilterId;
		u8 i = 0;
		CANQUEDATA candata;
	  static u8 sendcount = 0;
	  if(handle == NULL)
			 return;

		PrequestHandle = (DiagnosticRequestHandle*)handle;
		candata.Dlc = 0x08;
		candata.Id = PrequestHandle->Request.Id;
		
		candata.RTR = candata.Id & 0x0002;
    candata.IDE = candata.Id & 0x0004;
	
		if(candata.IDE == DIAGNOSTIC_CAN_ID_STANDARD)
		{
				candata.Id = (candata.Id >> 16);
		}
		else if(candata.IDE == DIAGNOSTIC_CAN_ID_EXTENDED)
		{
			  candata.Id = candata.Id -4;
		}
    memcpy(&candata.Data[0], PrequestHandle->Request.DataField,0x08);
		//判断是否设置ID过滤
		Is_Filtered = PrequestHandle->Request.BitField&0x01;
		if(Is_Filtered == 0x01)
		{
			 if(candata.IDE == DIAGNOSTIC_CAN_ID_STANDARD)
			 {
				  //若是功能地址,则不需要过滤
				  if(candata.Id == 0x7df)
					{
						 OBD_SetFilterStd_MUTI();
					}
					else if(candata.Id >= 0x7e0)
					{
						 FilterId = candata.Id + 0x08;
						 OBD_SetFilterStd(FilterId);
					}
			 }
			 else if(candata.IDE == DIAGNOSTIC_CAN_ID_EXTENDED)
			 {
			 }	
			 
		}
		else if(Is_Filtered == 0x00)
		{
			  if(channel == CAN_CHANNEL_1)
				{
					 setFilter(CAN_CHANNEL_1);
				}
				else if(channel == CAN_CHANNEL_2)
				{
					 setFilter(CAN_CHANNEL_2);
				}
		}
    if(channel == CAN_CHANNEL_1)
		{
			 CanTransmit(&Can1, &candata);
		}
		else if(channel == CAN_CHANNEL_2)
		{

			 CanTransmit(&Can2, &candata);
		}
	
}

void OBD_ReceiveMessages(u8 channel, CANX * pcan, COMX * pcom)
{

    u8  IDE;          
    u8 i, buf[20];
    CANQUEDATA data;
    DiagnosticResponse response;
    if(channel != 1 && channel != 2)
    {
        return;
    }
    else if(pcan == NULL || pcom == NULL)
    {
        return;
    }
    while(CanReceive(pcan, &data))
    {
        buf[0] = channel;
				buf[1] = (u8)(data.Id >> 24);
        buf[2] = (u8)(data.Id >> 16);
        buf[3] = (u8)(data.Id >> 8);
        buf[4] = (u8)(data.Id >> 0);

        buf[5] = data.Dlc;
				for (i = 0; i < data.Dlc; i++)
        {
            buf[6 + i] = data.Data[i];
        }
				if (data.Dlc < 8)
				{
						memset(buf + 6 + data.Dlc, 0, 8 - data.Dlc);
				}
				IDE = (u8)0x04 & data.Id;
				if(IDE == CAN_ID_STD)
				{
						data.Id = (u32)0x000007FF & (data.Id >> 21);
				}
				else if(IDE == CAN_ID_EXT)
				{
						data.Id = (u32)0x1FFFFFFF & (data.Id >> 3);
				}		
				response.Id = data.Id;
				response.length = data.Dlc;
				memcpy(response.DataField,data.Data,data.Dlc);
				PrequestHandle->networkcallback(&response);
    }
}
u8 J1939_dataack[20] = {0x71};
void J1939_ReceiveMessages(u8 channel, CANX * pcan, COMX * pcom)
{

    u8  IDE;       //判断是标准ID还是扩展ID   
	  u16 CurrentLen = 0;
    u8 i, buf[20];
	  u32 filterid;
    CANQUEDATA data;
    DiagnosticResponse response;
    if(channel != 1 && channel != 2)
    {
        return;
    }
    
    else if(pcan == NULL || pcom == NULL)
    {
        return;
    }
    while(CanReceive(pcan, &data))
    {
			  if(cannelfilter == TRUE)
				{
					  IDE = (u8)0x04 & data.Id;
						if(IDE == CAN_ID_STD)
						{
								filterid = (u32)0x000007FF & (data.Id >> 21);
						}
						else if(IDE == CAN_ID_EXT)
						{
								filterid = (u32)0x1FFFFFFF & (data.Id >> 3);
						}	
						for(i=0;i<9;i++)
						{
								if(J1939ExtIdArray[i] == filterid)
								{
										buf[0] = channel;
										//data.RTR若1为远程帧，0为数据帧
										data.RTR = data.Id & 0x00000002;
										//data.IDE若1为扩展，0为标准
										data.IDE = data.Id & 0x00000004;

										buf[1] = (u8)(data.Id >> 24);
										buf[2] = (u8)(data.Id >> 16);
										buf[3] = (u8)(data.Id >> 8);
										buf[4] = (u8)(data.Id >> 0);
				
										buf[5] = data.Dlc;
										for (i = 0; i < data.Dlc; i++)
										{
												buf[6 + i] = data.Data[i];
										}
										//数据帧
										if(data.RTR == 0x00)
										{	
												CurrentLen = 8+6;
												c16toa(&CurrentLen,&J1939_dataack[1]);
												memcpy(&J1939_dataack[FIXED_LENGTH],buf,14);
										}
										//远程帧
										else if(data.RTR == 0x00000002)
										{
												CurrentLen = CANCHANNEL+CANID;
												c16toa(&CurrentLen,&J1939_dataack[1]);
												memcpy(&J1939_dataack[FIXED_LENGTH],buf,CurrentLen);
										}	
										senddatapacket(&COM3,J1939_dataack,FIXED_LENGTH+CurrentLen+CHECKSUM);
								}
						}
					}
					else if(cannelfilter == FALSE)
					{
										buf[0] = channel;
										//data.RTR若1为远程帧，0为数据帧
										data.RTR = data.Id & 0x00000002;
										//data.IDE若1为扩展，0为标准
										data.IDE = data.Id & 0x00000004;

										buf[1] = (u8)(data.Id >> 24);
										buf[2] = (u8)(data.Id >> 16);
										buf[3] = (u8)(data.Id >> 8);
										buf[4] = (u8)(data.Id >> 0);
				
										buf[5] = data.Dlc;
										for (i = 0; i < data.Dlc; i++)
										{
												buf[6 + i] = data.Data[i];
										}
										//数据帧
										if(data.RTR == 0x00)
										{	
												CurrentLen = 8+6;
												c16toa(&CurrentLen,&J1939_dataack[1]);
												memcpy(&J1939_dataack[FIXED_LENGTH],buf,14);
										}
										//远程帧
										else if(data.RTR == 0x00000002)
										{
												CurrentLen = CANCHANNEL+CANID;
												c16toa(&CurrentLen,&J1939_dataack[1]);
												memcpy(&J1939_dataack[FIXED_LENGTH],buf,CurrentLen);
										}	
										senddatapacket(&COM3,J1939_dataack,FIXED_LENGTH+CurrentLen+CHECKSUM);
					}
		
   }
}
void J1939_CAN_SetFilterExt(u32 ExtId)
{
		u32 tmp,mask=0;
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;	
	
	  if(channel == CAN_CHANNEL_1)
		{
				CAN_FilterInitStructure.CAN_FilterNumber=0;
		}else if (channel == CAN_CHANNEL_2)
	  {
				CAN_FilterInitStructure.CAN_FilterNumber=14;
		}
		CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
		CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	
		CAN_FilterInitStructure.CAN_FilterIdHigh=((ExtId<<3)>>16)&0xffff;
		CAN_FilterInitStructure.CAN_FilterIdLow=((ExtId<<3)&0xffff)|CAN_ID_EXT;
	
		mask = 0x1fffffff;
	
		tmp = ExtId^(~ExtId);
		mask &=tmp;
		mask <<= 3;
		CAN_FilterInitStructure.CAN_FilterMaskIdHigh=(mask>>16)&0xffff;
		CAN_FilterInitStructure.CAN_FilterMaskIdLow=(mask&0xffff)|0x02;
		CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;
		CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
		CAN_FilterInit(&CAN_FilterInitStructure);
}
u8 count3=0;
u32 J1939ExtIdArray[10];
void J1939_CAN_SetFilterExt_Mutl(u32 ExtId)
{
	  u8 i,num;
		u32 tmp,mask=0;
		CAN_FilterInitTypeDef  CAN_FilterInitStructure;	
	 
	  if(count3<9)
		{
			 J1939ExtIdArray[count3++] = ExtId;
	
			 if(channel == CAN_CHANNEL_1)
		   {
				  CAN_FilterInitStructure.CAN_FilterNumber=0;
			
		   }else if (channel == CAN_CHANNEL_2)
	     {
				  CAN_FilterInitStructure.CAN_FilterNumber=14;
		   }
		   CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
		   CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit;
	
		   CAN_FilterInitStructure.CAN_FilterIdHigh=((J1939ExtIdArray[0]<<3)>>16)&0xffff;
		   CAN_FilterInitStructure.CAN_FilterIdLow=((J1939ExtIdArray[0]<<3))|CAN_ID_EXT;
	
		   mask = 0x1fffffff;
		   for(i=0; i<count3;i++)
		   {  
			    tmp =J1939ExtIdArray[i] ^ (~J1939ExtIdArray[0]);   
		      mask &=tmp;
		   } 

		   mask <<= 3;
		   CAN_FilterInitStructure.CAN_FilterMaskIdHigh=(mask>>16)&0xffff;
		   CAN_FilterInitStructure.CAN_FilterMaskIdLow=(mask&0xffff);
		   CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;
		   CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
		   CAN_FilterInit(&CAN_FilterInitStructure);
		 }
		 else if(count3>=9)
		 {
			   count3 = 0xff;
		 }
}
static u32 J1939_Translate_IdExt (u8 Priority,u8 Pdu_PF,u8 Pdu_Ps,u8 Pdu_Sa)
{
		u32 Id_Ext = 	0x00000000;
		Id_Ext     =    Id_Ext|Priority<<26;
		Id_Ext     =    ((Id_Ext>>16)|Pdu_PF)<<16;
		Id_Ext     =    ((Id_Ext>>8)|Pdu_Ps)<<8;
		Id_Ext     =    Id_Ext|Pdu_Sa;
		return     Id_Ext;
}

void J1939_CAN_Transmit(J1939_MESSAGE_T *MsgPtr,void *handle)
{
	  u32 i,j;
	  u8 SendStatus;
	  CANQUEDATA candata;
	
	  //发送CAN帧之前，需要确定是数据帧、远程帧、标准ID还是扩展ID？
	  candata.RTR = CAN_RTR_Data;
	  candata.IDE = CAN_Id_Extended;
	
	  PrequestHandle = (DiagnosticRequestHandle*)handle;
	
	  candata.Id  = J1939_Translate_IdExt(MsgPtr->Priority,MsgPtr->PDUFormat,MsgPtr->PDUSpecific,MsgPtr->SourceAddress);
	  candata.Dlc = MsgPtr->DataLength;
	  memcpy(&candata.Data[0], &MsgPtr->Data[0],candata.Dlc);
 
  	if(channel == CAN_CHANNEL_1)
    {		
			 CanTransmit(&Can1, &candata);
		}
		else if(channel == CAN_CHANNEL_2)
		{
			 CanTransmit(&Can2, &candata);
		}
}


