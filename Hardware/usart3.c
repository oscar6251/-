#include "Delay.h"
#include "usart3.h"
#include <stdio.h>
#include <stdarg.h>	 	 
#include "string.h"	 
#include "timer.h"
#include "LED.h" 
#include "OLED.h"
#include "PWM.h"
#include "SERVO.h"


//���ڽ��ջ����� 	
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; 			  //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�

//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
vu16 USART3_RX_STA=0;   	




//��ʼ��IO ����2
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������	  
void usart3_init(u32 bound)
{  

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //����2ʱ��ʹ��

		 //USART3_TX   PA2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART3_RX	  PA3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
  
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
  	
	//ʹ�ܽ����ж�
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�   
	
	//�����ж����ȼ�
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	
	
	TIM2_Int_Init(99,7199);		//10ms�ж�
	USART3_RX_STA=0;		//����
	TIM_Cmd(TIM2,DISABLE);			//�رն�ʱ��2

}


void Serial_SendByte1(uint8_t Byte)
{
	USART_SendData(USART2, Byte);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

int fputc(int ch, FILE *f)	//�����ַ�����
{
	Serial_SendByte1(ch);
	return ch;
}


void USART2_IRQHandler(void)
{


	u8 res;	//����һ�ֽ�����      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
	{	
		
		res =USART_ReceiveData(USART2);		 
		if((USART3_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//�����Խ�������
			{
				TIM_SetCounter(TIM2,0);//ÿ����һ�ֽ����ݣ���������� Ϊ�˲���ʱ �����붨ʱ��10ms�ж�      //���������
				if(USART3_RX_STA==0) 	//������⣺ÿ������һ������ʱ��ʹ�ܶ�ʱ��7���ж� 
				{
					TIM_Cmd(TIM2,ENABLE);//ʹ�ܶ�ʱ��7
				}
				USART3_RX_BUF[USART3_RX_STA++]=res;	//��¼���յ���ֵ	 
			}
			else 
			{
				USART3_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
			} 
		}
		// �жϽ��յ��������Ƿ�Ϊ"ffabcdfe"
		if (USART3_RX_STA >= 8)
		{
				if (USART3_RX_BUF[0] == 0xFF && USART3_RX_BUF[1] == 0xAB && USART3_RX_BUF[2] == 0xCD && USART3_RX_BUF[3] == 0xFE)
				{
						// ���ö�����ƺ���
						Servo_SetAngle(180);
						Delay_ms(5000);
						Servo_SetAngle(0);
				}
				
				// ��ս��ջ�����
				USART3_RX_STA = 0;
		}
	}  				 											 
}   

















