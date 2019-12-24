/*
	������Ĺ��ܿ������£�
	ͨ��һ��7S-EG-MPX8-CA-BLUEʵ��ʱ�����ʾ��������¼�µ�ʱ��͵�ǰʱ�䣩��
  ������ѯ   ��ť1��p3.4���͵�ƽ ���Բ鿴�����ʱ�䡢˯������
	�ⲿ�ж� 1 ��ť2��p3.3���½��� ���Կ��Ʊ�������ͣ
	�ⲿ�ж� 0 ��ť3��p3.2���½��� ��������countwaring�������ñ�����״̬����LED������ֹͣ
	��ǰʱ���ʵ����ͨ�������ڷ�ʽһ��T0����ж�ʵ�֣�����������Ϊÿ50000�η���һ���жϣ���20��Ϊһ�룬��60sΪ1min����60minΪ1h
	LED����˸��������Ƶ����T1����
*/
#include <reg51.h>
#include <absacc.h>
#include <intrins.h>
#define uint unsigned int 
#define uchar unsigned char
#define PA8255 0xff21  //�ⲿ82C55 b��
#define PB8255 0xff22 //�ⲿ82C55 C��
#define COM8255 0xff20 //82C55 ���ƿ�
#define LENGTH 500     //50000
sfr IPH =0xB7;
sbit RED = P1^2;
sbit YELLOW = P1^3;
sbit BUZZER = P1^4;
sbit DISPLAY = P3^4;
sbit WHITE = P1^5;
uchar HOUR = 0x00;
uchar MIN = 0x00;
uchar SEC = 0x00;
uchar sleepcount = 0;//˯������
uchar flagwaringstart = 0; //����T1��ʱ
uchar count1s = 0x00;	//��һ�������20�� -- 1 ��
uchar countwaring = 0; //ÿmin��һ������11/60
uchar code table[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};        //����������ܶ����
uchar idata iADDR[36];  // �����¼���˯������
uchar j,y,x;

void delayms(uint xms)
{
   uint i,j;
   for(i=xms;i>0;i--)
         for(j=120;j>0;j--);
}
void display(uchar sec1,uchar min1,uchar hour1)
{
   XBYTE[PA8255] = 0x3E;  //��ʾ���λ 111110
   XBYTE[PB8255] = table[sec1%10];                   //��ʾ���λ
   delayms(3);
	 
   XBYTE[PA8255]=0x3D;//111101
   XBYTE[PB8255]=table[sec1/10];                  //��ʾ��ʮλ
   delayms(3);
/*
   P2=0x03;                                  //��ʾ����
   P0=0xbf;
   delayms(5);
*/
   XBYTE[PA8255]=0x3B; //111011                                         //��ʾ�ָ�λ
   XBYTE[PB8255]=table[min1%10]&0x7f;
   delayms(3);

   XBYTE[PA8255]=0x37; //110111                                         //��ʾ��ʮλ
   XBYTE[PB8255]=table[min1/10];
   delayms(3);
/*
   P2=0x04;                              //��ʾ����
   P0=0xbf;
   delayms(5);
*/
   XBYTE[PA8255]=0x2F;         //101111                                  //��ʾʱ��λ
   XBYTE[PB8255]=table[hour1%10]&0x7f;    //�˶��������ʾС�������Ҫ�ڶ�ѡ��� + &0x7F !!!
   delayms(3);

   XBYTE[PA8255]=0x1f;//011111
   XBYTE[PB8255]=table[hour1/10];
   delayms(3);
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
	YELLOW = 1;//ֱ����P1
		RED = 1;
	BUZZER = 1;
	  XBYTE[COM8255] = 0x43;
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
					for(x = 0;x<100;x++) {XBYTE[PA8255]=0x3D;XBYTE[PB8255]=table[iADDR[0]/10];XBYTE[PA8255]=0x3E;XBYTE[PB8255]=table[iADDR[0]%10];}
					delayms(500);
				for(j = 0;j < iADDR[0];j++){
					for(x = 0;x<100;x++) display(iADDR[3 + j*3],iADDR[2 + j*3],iADDR[1 + j*3]);
					delayms(500);
				}
			}
		}
}
void exint0(void) interrupt 0 {  // ��������ʱ��0
	countwaring = 0;
	YELLOW = 1;
		RED = 1;
	BUZZER = 1;
	TR1 = 0;
}
void exint1(void) interrupt 2 {  // ��/ͣ��������
	ET1 = !ET1;
	countwaring = 0;
	TR1=0;
	YELLOW = 1;//ֱ����P1
		RED = 1;
	BUZZER = 1; 
	flagwaringstart = !flagwaringstart;  // ���ƾ���λ����ֹ��T0�ж�������T1
}
void timer1int(void) interrupt 3{  // ��ʱ��1 ���ƺ�/�Ƶ���˸������������Ƶ��
	if(countwaring<11){TH1 = 10000/256;TL1 = 10000%256;YELLOW = 1;RED = 1;}
	if(countwaring>10 && countwaring<60 ){TH1 = 0;TL1 = 0;YELLOW = !YELLOW;RED = 1;}
	if(countwaring>59){TH1 = 30000/256;TL1 = 30000%256;YELLOW = 1;RED = !RED;}
	BUZZER = !BUZZER;
}
void timer0int(void) interrupt 1{  // ͨ��T0�ж� ��ʾʱ��ӿ��������ڵ�ʱ��
	TH0 = (65536-LENGTH)/256;
	TL0 = (65536-LENGTH)%256;	
	count1s++;
	/*
		����һ�ΰ��°�ť��ȥ11min�����Ƶ���˸����countwaring = 11�������Ӽ�һ��
		ʱ�ж���жϵķ�������if��countwaring = 11ʱֻ��ִ��һ�Σ���SEC��count1s���ж�
		flagwaringstart��ͨ��P3.3���Ʒ�ֹT1��ֹͣ����ʱ����
		countwaring = 60ʱ��ͬ�� 
	*/
	if(countwaring == 10 && !SEC && count1s == 1 && flagwaringstart)  
			{	TR1 = 0;
				TH1 = 0;
				TL1 = 0;
				TR1 = 1;
			}
		if(countwaring == 11 && !SEC && count1s == 1 && flagwaringstart)  
		{	TR1 = 0;
			TH1 = 10000/256;
			TL1 = 10000%256;
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