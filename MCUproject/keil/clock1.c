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
uchar sleepcount = 0;//睡觉次数
uchar flagwaringstart = 0; //启动T1计时
uchar count1s = 0x00;//达一秒计数
uchar countwaring = 0; //每min计一数，达11/60
uchar code table[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};        //共阳极数码管断码表
uchar idata iADDR[36];  // 保存事件及睡觉次数
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
   P0=table[sec1%10];                   //显示秒个位
   delayms(50);

   P2=0x40;
   P0=table[sec1/10];                  //显示秒十位
   delayms(50);

   P2=0x20;                                  //显示横线
   P0=0xbf;
   delayms(50);

   P2=0x10;                                          //显示分个位
   P0=table[min1%10];
   delayms(50);

   P2=0x08;                                          //显示分十位
   P0=table[min1/10];
   delayms(50);

   P2=0x04;                              //显示横线
   P0=0xbf;
   delayms(50);

   P2=0x02;                                           //显示时个位
   P0=table[hour1%10];
   delayms(50);

   P2=0x01;  //显示时十位
   P0=table[hour1/10];
   delayms(50);
}

/*
	T0用来显示时间
	T1用来控制灯的闪烁、喇叭的频率
	用countwaring来计算离上一次按下按钮的时间（分钟）
	INT0是重置countwaring
	INT1是控制警报是否启动
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
void exint0(void) interrupt 0 {  // 将警报计时清0
	countwaring = 0;
	P1 = 0;
	TR1 = 0;
}
void exint1(void) interrupt 2 {
	ET1 = !ET1;  //控制T1的溢出中断
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
	if(countwaring == 11 && !SEC && count1s == 1 && flagwaringstart)  // 离上一次按下按钮过去11min启动黄灯闪烁
			{	TR1 = 0;
				TH1 = 0;
				TL1 = 0;
				TR1 = 1;
			}
	if(countwaring == 60 && !SEC && count1s == 1 && flagwaringstart){  // 离上一次按下按钮过去60min 红灯闪烁 喇叭频率加快 保存数据至iADDR数组中
		TR1 = 0;
		TH1 = 30000/256;
		TL1 = 30000%256;
		TR1 = 1;
		
		iADDR[sleepcount*3+1] = HOUR;
		iADDR[sleepcount*3+2] = MIN;
		iADDR[sleepcount*3+3] = SEC;
		iADDR[0] = ++sleepcount;
	}
	if(count1s>19)//计数时分秒
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