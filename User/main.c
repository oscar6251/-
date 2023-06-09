#include "stm32f10x.h"                  
#include "Delay.h"
#include "oled_iic.h"
#include "LED.h"
#include "rc522.h"
#include "Serial.h"
#include "timer.h"
#include "usart3.h"
#include "Key1.h"
#include <stdio.h>
#include "keyCode.h"
#include "flash.h"
#include "SERVO.h"
#include "Beep.h"



uint8_t MEAU=0;	//�˵��л���0�����˵���2������/3��ˢ��/4������


unsigned char Status;
unsigned char CardType[2];//������ 
unsigned char Id[4];    //FRIDʶ���4λUID���ţ�����Id������
 
//  ¼�뿨��UID,�����ж��ſ����ر�
unsigned char Card_0[4]= {0x97,0x2D,0xC0,0xA7};//�׿�	 ��ȷ��0x97,0x2D,0xC0,0xA7
unsigned char Card_1[4]= {0x19,0x18,0x91,0x98};//����  ��ȷ��0x19,0x18,0x91,0x98
//void RC522_Test1(void);

//  �������ڽ�������
uint8_t RxData;
unsigned char Blu[2]= {0xab,0xcd};//��������
void BluetoothTest2(void);

//RC522���Ժ���
 void RC522_Test1(void);
	 
//���˵�
void ShowMeau(void);

int main(void)
{
	OLED_Init();	//OLED��ʼ��
	LED_Init();		//LED��ʼ��
	Beep_Init();	//��������ʼ��

	Serial_Init();	//����1��ʼ������
	

	RC522_Init();  //��ʼ����Ƶ��ģ��
  usart3_init(9600);//����2���ճ�ʼ��
	
/********���ֿ��ʼ������***********/
	
	extern const u8 BMP1[];
	HZ= GB16_NUM();//��������
	
/********���˵���ʼ������***********/	
	ShowMeau();
   
/********������̳�ʼ������***********/
	Matrix_Key_Init();
	
	
/********�ϵ��ȡ��������ʼ������***********/
	KeyCode_Init();
	
/********�����ʼ��***********/	
	Servo_Init();
	
	while(1)
  {

		KeyVal=Matrix_Key_Scan();

		if(MEAU==0)
		{	
		  if(KeyVal==1)	//��������
		  {
				OLED_Clear();
				OLED_ShowCH(2,0,(u8*)"app��������");  //ffabcdfe
				MEAU=2;
		  }
		  if(KeyVal==3)	//����RFID
		  {
				OLED_Clear();
				OLED_ShowCH(32,0,(u8*)"��ˢ��");  
				MEAU=3;
		  }
		  if(KeyVal==6)	//����������
		  {
				OLED_Clear();
				OLED_ShowCH(0,0,(u8*)"C ����������");
				OLED_ShowCH(0,2,(u8*)"B ȷ������ ");	
				MEAU=4;
		  }
										

		}		
		
		if(MEAU==2)//��������
	  {			
		   BluetoothTest2();	
		}
		 
		if(MEAU==3)//RFID���� 
	  {		
			RC522_Test1();	
		}
		
	  if(MEAU==4)//����������
	  {							
			KeyVal=KEY_ERR;
			KeyVal=Matrix_Key_Scan();  
			data_key();
			KeyVal=Matrix_Key_Scan();
	  }
		
		//��������ʶ��ɹ����͵�����
		BluetoothTest2();
	}

}
	



//���ܣ��Ž�ģ��

 void RC522_Test1()
{

    Status = PcdRequest(PICC_REQALL,CardType);//Ѱ��
	
    if(Status==MI_OK)// Ѱ���ɹ�
    {
        Status=MI_ERR;
				Status = PcdAnticoll(Id);// ����ײ ���UID ����Id
    }

    int isExecuted = 0; // ��ʼ����־������0 ��ʾ false

		if (Status == MI_OK) // ����ײ�ɹ�
		{
				Status = MI_ERR;
				ShowID(Id); // ���ڴ�ӡİ������ID�� UID
				//¼��Ŀ��˶Գɹ��������ʹ�����ת
				if ((Id[0] == Card_0[0]) && (Id[1] == Card_0[1]) && (Id[2] == Card_0[2]) && (Id[3] == Card_0[3]))
				{
						OLED_Clear();
						OLED_ShowCH(32, 0, (u8 *)"ˢ���ɹ�");
						Servo_SetAngle(180);
						LED2_ON();
						Delay_ms(5000);
						Servo_SetAngle(0);
						LED2_OFF();
						isExecuted = 1; // ִ���˵�һ������(��һ��¼��Ŀ�ƥ��ɹ�)
				}

				if (!isExecuted && (Id[0] == Card_1[0]) && (Id[1] == Card_1[1]) && (Id[2] == Card_1[2]) && (Id[3] == Card_1[3]))
				{
						OLED_Clear();
						OLED_ShowCH(32, 0, (u8 *)"ˢ���ɹ�");
						Servo_SetAngle(180);
						LED2_ON();
						Delay_ms(5000);
						Servo_SetAngle(0);
						LED2_OFF();
						isExecuted = 1; // ִ���˵ڶ�������(�ڶ���¼��Ŀ�ƥ��ɹ�)
				}

				if (!isExecuted)
				{
						OLED_Clear();
						OLED_ShowCH(32, 0, (u8 *)"ˢ��ʧ��");
						LED1_ON();
						Beep_ON();
						Delay_ms(1500);
						LED1_OFF();
						Beep_OFF();
				}

				MEAU = 0;
				ShowMeau();
		}


		if(KeyVal=='*')	//���ذ���
	  {
			OLED_Clear();
			OLED_ShowCH(0,0,(u8*)"K1 ����  ");
			OLED_ShowCH(0,2,(u8*)"K3 ˢ��  ");	
			OLED_ShowCH(0,4,(u8*)"K6 ����  ");
			//OLED_ShowCH(0,6,(u8*)"* ����	 ");
			MEAU=0;
	  }

}


//���ܣ�����ģ����������ʾ��OLED

void BluetoothTest2()
{
	
	uint8_t i;
	if (Serial_GetRxFlag() == 1)//�յ����ݣ�����ȫ������ȫ�ֱ���Serial_RxPacket[4]��������ʾ������ȡ�����ݲ���
	 {	
			//LED2_Turn();
			if((Serial_RxPacket[0]==Blu[0])&&(Serial_RxPacket[1]==Blu[1]))	//app�����������ȷ
				{
					OLED_Clear();
					OLED_ShowCH(0,0,(u8*)"�ɹ�"); 

					LED2_ON();
					Servo_SetAngle(180);
					Delay_ms(5000);
					Servo_SetAngle(0);
					MEAU=0;
					ShowMeau();
				}
			else
				{
					OLED_Clear();
					OLED_ShowCH(0,0,(u8*)"����");
					LED1_ON();
					Beep_ON();
					Delay_ms(1500);
					LED1_OFF();
					Beep_OFF();
					MEAU=0;
					ShowMeau();
				}
				
				
		}
		if(KeyVal=='*')	//���ذ�����һ��һ��
		{
				OLED_Clear();
				OLED_ShowCH(0,0,(u8*)"K1 ����  ");
				OLED_ShowCH(0,2,(u8*)"K3 ˢ��  ");	
				OLED_ShowCH(0,4,(u8*)"K6 ����  ");
				//OLED_ShowCH(0,6,(u8*)"* ����	 ");
				MEAU=0;
		}
		else	
		{
				LED2_OFF();
		}	
				
		
}




/********���˵�*********/
void ShowMeau()
{
		OLED_Clear();
		OLED_ShowCH(0,0,(u8*)"K1 ����  ");
		OLED_ShowCH(0,2,(u8*)"K3 ˢ��  ");	
		OLED_ShowCH(0,4,(u8*)"K6 ����  ");
		//OLED_ShowCH(0,6,(u8*)"* ����	 ");
}
	

