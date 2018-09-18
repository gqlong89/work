/**
  ******************************************************************************
  * @file    IAP_Main/Src/ymodem.c 
  * @author  MCD Application Team
  * @version 1.0.0
  * @date    8-April-2015
  * @brief   This file provides all the software functions related to the ymodem 
  *          protocol.
  ******************************************************************************
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/** @addtogroup STM32F1xx_IAP
  * @{
  */ 
  
/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"
#include "menu.h"
#include  <Uart.h>
#include <stdlib.h>
#include "Shell.h"
#include "Delay.h"
#include "Flash.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* activate the CRC16 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
UTCPTRANSDEC packet;
u8 file_name[FILE_NAME_LENGTH];
static uint32_t flashdestination =APPLICATION_ADDRESS ;
static u8 packets_received=0;
static u32 j = 0;
typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
u32 JumpAddress;
RUN_MODE flag_1 = NORMALMODE;
/* @note ATTENTION - please keep this variable 32bit alligned */
extern bool is_upgrade;
/* Private function prototypes -----------------------------------------------*/
static void yReceivePacket(UTCPTRANSDEC *packet);
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte);
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size);
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size);


static void YModem_SendByte(COMX * pcom,u8 c)
{
	  ComxPutChar(pcom,c);
}

void c32toa_2(u32* data, u8* c)
{
	  if(data==NULL&&c==NULL)
		{
			return;
		}
    c[0] = (*data >> 24) & 0xff;
    c[1] = (*data >> 16) & 0xff;
    c[2] = (*data >>  8) & 0xff;
    c[3] =  *data & 0xff;
}

u8 Packetrecv(COMX * pcom,UTCPTRANSDEC *packet,u8 dat)
{
	  u16 upgrede;
    u32 i, k;
	  u16 crc;
	  static u32 packet_size;
	  u32 index = 0;
    u8 *file_ptr;
	  u8 file_size[FILE_SIZE_LENGTH];
	  u32 size;
	  u8 filesize[4];
	  FLASH_Status temp_stat=FLASH_COMPLETE;
	  u8 data[1024]={0x00};
		u16 datatemp[512]={0x00};
		uint32_t EraseCounter = 0x00; //��������
	  uint32_t NbrOfPage = 0x00;//��¼Ҫ������ҳ��
    
	  switch(packet->State)
    {
				case 0:
						if(packet->Len < 1)
						{	  
								packet->Dat[PACKET_START_INDEX] = dat;
								packet->Len++;
						}
						else
						{
								switch(packet->Dat[PACKET_START_INDEX])
								{
									case SOH:	
										packet->Dat[PACKET_NUMBER_INDEX] = dat;		
										packet->Len    = PACKET_HEADER_SIZE;	
										packet->State  = 1;						
										packet_size = PACKET_SIZE;
										//flag = REMOTEMODE;	
										//TIM_Cmd(TIM2, DISABLE);	
										break;
									case STX:	
										packet->Dat[PACKET_NUMBER_INDEX] = dat;	
										packet->Len    = PACKET_HEADER_SIZE;	
										packet->State  = 1;							 							 
										packet_size = PACKET_1K_SIZE;	
										break;
									//����ļ�����
									case EOT:
										RTU_DEBUG("EOT\r\n",size);
										DelayMS(50);
										YModem_SendByte(pcom,ACK);
										upgrede = 0x02;
										MemWriteData(FLASH_ADDRESS,&upgrede,1);
										DelayMS(50);
										is_upgrade = FALSE;					
										TIM_Cmd(TIM6, ENABLE);	
                    RTU_DEBUG("EOT!!!!\r\n",size);									
									case CA:	
										packet->Len   = 0;
										packet->State = 0;									
										return -1;
									case ABORT1:				
									case ABORT2:	
										packet->Len   = 0;
										packet->State = 0;								 
										return -1;                      
									default:
										packet->Len   = 0;
										packet->State = 0;
										return -1;  
								}							 
						}
						break;
				case 1:
						packet->Dat[packet->Len++] = dat;
		
				    if(packet->Len==packet_size+PACKET_OVERHEAD_SIZE+1)
						{
							if (packet->Dat[PACKET_NUMBER_INDEX] != ((packet->Dat[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
							{
								  YModem_SendByte(pcom,CA);
								  DelayMS(50);
									YModem_SendByte(pcom,CA);
								  packet->Len   = 0;
						      packet->State = 0;
									return -1;
							}
							else
							{			
								 /* Check packet CRC */
								 crc = packet->Dat[ packet_size + PACKET_DATA_INDEX] << 8;
								 crc += packet->Dat[ packet_size + PACKET_DATA_INDEX+1];
	
								 if (Cal_CRC16(&packet->Dat[PACKET_DATA_INDEX], packet_size) != crc )
								 {
									   YModem_SendByte(pcom,CA);
									   DelayMS(50);
									   YModem_SendByte(pcom,CA);
									   packet->Len   = 0;
						         packet->State = 0;
										 return -1;
								 }
		        
							}
 
							if(packet->Dat[PACKET_START_INDEX]==SOH)
						  {
							  
									if(packets_received==0)
									{
							
										for(i = 0, file_ptr = packet->Dat+PACKET_DATA_INDEX; (*file_ptr != 0) && (i < FILE_NAME_LENGTH); )
										{
												file_name[i++] = *file_ptr++;
											  RTU_DEBUG("%c",file_name[i]);
										}
										file_name[i++] = '\0';
					 
										for(i = 0, file_ptr++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH); )
										{
												file_size[i++] = *file_ptr++;
										}
										file_size[i++] = '\0';
										size = atoi((const char *)file_size); 
                    RTU_DEBUG("size=%d\r\n",size);
										c32toa_2(&size,filesize);

										if(size>USER_FLASH_SIZE+1)
										{
											  YModem_SendByte(pcom,CA);
											  DelayMS(50);
											  YModem_SendByte(pcom,CA);
											  packet->Len   = 0;
											  packet->State = 0;
										    return -1;
										}
										packet->Len   = 0;
										packet->State = 0;
							
										FLASH_Unlock();
										/*������Ҫ����FLASHҳ�ĸ���*/
										NbrOfPage = (USER_FLASH_END_ADDRESS-APPLICATION_ADDRESS)/FLASH_PAGE_SIZE;
										FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); 
										for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (temp_stat == FLASH_COMPLETE); EraseCounter++)
										{               
												temp_stat=FLASH_ErasePage(APPLICATION_ADDRESS+(FLASH_PAGE_SIZE*EraseCounter)); 
												if(temp_stat != FLASH_COMPLETE)
												{
														FLASH_Lock();
														return 0;
												}
										}
										FLASH_Lock();
	
										YModem_SendByte(pcom,ACK);
                    DelayMS(50);										
										YModem_SendByte(pcom,CRC16);
										//packets_received++;	
									}
						 }
						 else if(packet->Dat[PACKET_START_INDEX]==STX)
						 {	
									memcpy(datatemp,(u16*)&packet->Dat[PACKET_DATA_INDEX],1024);
									FLASH_Unlock();
									for(i=0;i<512;i++)
									{
										temp_stat = FLASH_ProgramHalfWord((flashdestination +i*2),datatemp[i]); 
										//д��FLASHʱ��������
										if(temp_stat!=FLASH_COMPLETE)
										{
											  YModem_SendByte(pcom,CA);
											  DelayMS(50);
											  YModem_SendByte(pcom,CA);
											  packet->Len   = 0;
											  packet->State = 0;
											  return -1;
										}
									}
									FLASH_Lock();
									
									
									flashdestination = flashdestination + 0x400;
									packet->Len   = 0;
									packet->State = 0;
									DelayMS(50);
									YModem_SendByte(pcom,ACK);	
								}	 								
						}
						break;
				default:
						packet->Len   = 0;
						packet->State = 0;
						break;
    }
    return 0;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Receive a packet from sender
  * @param  data
  * @param  length
  *     0: end of transmission  �������
  *     2: abort by sender  �ɷ�������ֹ
  *    >0: packet length  ������
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */

static void yReceivePacket(UTCPTRANSDEC *packet)
{
   uint8_t temp;
	 //�������ݰ��ĵ�һ���ֽ�
	 if(ComxGetChar(&COM2, &temp))
   {
		  Packetrecv(&COM2,packet,temp);
	 }
}

/**
  * @brief  Update CRC16 for input byte
  * @param  crc_in input value 
  * @param  input byte
  * @retval None
  */
uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
    uint32_t crc = crc_in;
    uint32_t in = byte | 0x100;

    do
    {
      crc <<= 1;
      in <<= 1;
      if(in & 0x100)
        ++crc;
      if(crc & 0x10000)
        crc ^= 0x1021;
    }
  
    while(!(in & 0x10000));

    return crc & 0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t* dataEnd = p_data+size;

    while(p_data < dataEnd)
      crc = UpdateCRC16(crc, *p_data++);
 
    crc = UpdateCRC16(crc, 0);
    crc = UpdateCRC16(crc, 0);
	
    return crc&0xffffu;
}

/**
  * @brief  Calculate Check sum for YModem Packet
  * @param  p_data Pointer to input data
  * @param  size length of input data
  * @retval uint8_t checksum value
  */
uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
    uint32_t sum = 0;
    const uint8_t *p_data_end = p_data + size;

    while (p_data < p_data_end )
    {
       sum += *p_data++;
    }
    return (sum & 0xffu);
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @retval COM_StatusTypeDef result of reception/programming
  */
void Ymodem_Receive()
{
	  yReceivePacket(&packet);	
}

//Զ��������APP�󣬾͸�λMCU�����������µ�APP����
void TIM6_IRQHandler(void)  
{
    u16 flashsec;
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM�ж�Դ
    {
				MemReadData(FLASH_ADDRESS,&flashsec,1);
			  TIM_Cmd(TIM6, DISABLE);
			  //RTU_DEBUG("flashsec=%d\r\n",flashsec);
			
				if(flashsec == 2) 
        {
					 __set_FAULTMASK(1);
				   NVIC_SystemReset();
				}	

    }
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM�ж�Դ
}
/**
  * @
  */

/*******************(C)COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/