#include <REG52.h>
#include <absacc.h>
#define uint unsigned int 
#define uchar unsigned char
#define LENGTH 700
sfr IPH =0xB7;
sbit RED = P1^2;
sbit YELLOW = P1^3;
sbit BUZZER = P1^4;
sbit DISPLAY = P3^4;
uchar HOUR = 0x00;
uchar MIN = 0x00;
uchar SEC = 0x00;
uchar sleepcount = 0;//˯������
uchar flagwaringstart = 0; //����T1��ʱ
uchar count1s = 0x00;//��һ�����
uchar countwaring = 0; //ÿmin��һ������11/60
uchar code table[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};        //����������ܶ����
uchar idata iADDR[36];  // �����¼���˯������
uchar j;
void delayms(uint xms)
{
   uint i,j;
   for(i=xms;i>0;i--)
         for(j=120;j>0;j--);
}
void display(uchar sec1,uchar min1,uchar hour1)
{
   P2=0x80;
   P0=table[sec1%10];                   //��ʾ���λ
   delayms(50);

   P2=0x40;
   P0=table[sec1/10];                  //��ʾ��ʮλ
   delayms(50);

   P2=0x20;                                  //��ʾ����
   P0=0xbf;
   delayms(50);

   P2=0x10;                                          //��ʾ�ָ�λ
   P0=table[min1%10];
   delayms(50);

   P2=0x08;                                          //��ʾ��ʮλ
   P0=table[min1/10];
   delayms(50);

   P2=0x04;                              //��ʾ����
   P0=0xbf;
   delayms(50);

   P2=0x02;                                           //��ʾʱ��λ
   P0=table[hour1%10];
   delayms(50);

   P2=0x01;  //��ʾʱʮλ
   P0=table[hour1/10];
   delayms(50);
}

/*
	T0������ʾʱ��
	T1�������ƵƵ���˸�����ȵ�Ƶ��
	��countwaring����������һ�ΰ��°�ť��ʱ�䣨���ӣ�
	INT0������countwaring
	INT1�ǿ��ƾ����Ƿ�����
*/
void main(void){
		SP = 0x60;
		P1=0;

		TH0 = (65536-LENGTH)/256;
		TL0 = (65536-LENGTH)%256;
		TMOD = 0x11;
		TCON = 0x05;
		IP = 0x01;
		IPH = 0x03;
		IE = 0x87;//10000111
		TR0 = 1;
		while(1){
			display(SEC,MIN,HOUR);
			while(!DISPLAY){
			  uint a = iADDR[0];
				delayms(1000);
				for(j = 0;j < iADDR[0];j++){
					display(iADDR[3 + j*3],iADDR[2 + j*3],iADDR[1 + j*3]);
					delayms(1000);
				}
			}
		}
}
void exint0(void) interrupt 0 {  // ��������ʱ��0
	countwaring = 0;
	P1 = 0;
	TR1 = 0;
}
void exint1(void) interrupt 2 {
	ET1 = !ET1;  //����T1������ж�
	countwaring = 0;
	TR1=0;
	P1 = 0;
	TH1 = 0;
	TL1 = 0;
	flagwaringstart = !flagwaringstart;
}
void timer1int(void) interrupt 3{
	
	if(countwaring<60){TH1 = 0;TL1 = 0;YELLOW = !YELLOW;RED = 0;}
	if(countwaring>59){TH1 = 30000/256;TL1 = 30000%256;YELLOW = 0;RED = !RED;}
	
	BUZZER = !BUZZER;
}
void timer0int(void) interrupt 1{
	TH0 = (65536-LENGTH)/256;
	TL0 = (65536-LENGTH)%256;
	count1s++;
	if(countwaring == 11 && !SEC && count1s == 1 && flagwaringstart)  // ����һ�ΰ��°�ť��ȥ11min�����Ƶ���˸
			{	TR1 = 0;
				TH1 = 0;
				TL1 = 0;
				TR1 = 1;
			}
	if(countwaring == 60 && !SEC && count1s == 1 && flagwaringstart){  // ����һ�ΰ��°�ť��ȥ60min �����˸ ����Ƶ�ʼӿ� ����������iADDR������
		TR1 = 0;
		TH1 = 30000/256;
		TL1 = 30000%256;
		TR1 = 1;
		
		iADDR[sleepcount*3+1] = HOUR;
		iADDR[sleepcount*3+2] = MIN;
		iADDR[sleepcount*3+3] = SEC;
		iADDR[0] = ++sleepcount;
	}
	if(count1s>19)//����ʱ����
	{
		count1s = 0;
		SEC++;
		if(SEC>59){
			SEC = 0;
			MIN++;
			countwaring++;
			if(MIN>59){
				MIN = 0;
				HOUR++;
				if(HOUR>23)HOUR = 0;
			}
		}
	}
	

}