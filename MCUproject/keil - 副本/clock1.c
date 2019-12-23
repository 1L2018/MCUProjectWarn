/*
	本程序的功能控制如下：
	通过一块7S-EG-MPX8-CA-BLUE实现时间的显示（包括记录下的时间和当前时间），
  反复查询   按钮1（p3.4）低电平 可以查看保存的时间、睡觉次数
	外部中断 1 按钮2（p3.3）下降沿 可以控制报警的启停
	外部中断 0 按钮3（p3.2）下降沿 可以重置countwaring，即重置报警的状态，让LED和喇叭停止
	当前时间的实现是通过工作在方式一的T0溢出中断实现，计数次数设为每50000次发生一次中断，满20次为一秒，满60s为1min，满60min为1h
	LED灯闪烁、喇叭响频率由T1控制
*/
#include <REG52.h>
#include <absacc.h>
#define uint unsigned int 
#define uchar unsigned char
#define PA8255 0xff7c  //外部82C55 A口
#define PC8255 0xff7e //外部82C55 C口
#define COM8255 0xff7f //82C55 控制口
#define LENGTH 500     //50000
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
uchar count1s = 0x00;	//达一秒计数，20次 -- 1 秒
uchar countwaring = 0; //每min计一数，达11/60
uchar code table[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};        //共阳极数码管段码表
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
   XBYTE[PC8255] = 0x01;
   XBYTE[PA8255] = table[sec1%10];                   //显示秒个位
   delayms(5);

    XBYTE[PC8255]=0x02;
   XBYTE[PA8255]=table[sec1/10];                  //显示秒十位
   delayms(5);
/*
   P2=0x03;                                  //显示横线
   P0=0xbf;
   delayms(5);
*/
    XBYTE[PC8255]=0x04;                                          //显示分个位
   XBYTE[PA8255]=table[min1%10]&0x7f;
   delayms(5);

    XBYTE[PC8255]=0x08;                                          //显示分十位
   XBYTE[PA8255]=table[min1/10];
   delayms(5);
/*
   P2=0x04;                              //显示横线
   P0=0xbf;
   delayms(5);
*/
    XBYTE[PC8255]=0x10;                                           //显示时个位
   XBYTE[PA8255]=table[hour1%10]&0x7f;    //八段数码管显示小数点仅需要在段选码后 + &0x7F !!!
   delayms(5);

    XBYTE[PC8255]=0x20;  //显示时十位
   XBYTE[PA8255]=table[hour1/10];
   delayms(5);
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
	  XBYTE[COM8255] = 0x80;
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
void exint1(void) interrupt 2 {  // 启/停报警功能
	ET1 = !ET1;  //控制T1的溢出中断
	countwaring = 0;
	TR1=0;
	P1 = 0;
	TH1 = 0;
	TL1 = 0;
	flagwaringstart = !flagwaringstart;  // 控制警告位，防止在T0中断中启动T1
}
void timer1int(void) interrupt 3{  // 定时器1 控制红/黄灯闪烁、喇叭响声的频率
	
	if(countwaring<60){TH1 = 0;TL1 = 0;YELLOW = !YELLOW;RED = 0;}
	if(countwaring>59){TH1 = 30000/256;TL1 = 30000%256;YELLOW = 0;RED = !RED;}
	BUZZER = !BUZZER;
}
void timer0int(void) interrupt 1{  // 通过T0中断 显示时间从开机到现在的时间
	TH0 = (65536-LENGTH)/256;
	TL0 = (65536-LENGTH)%256;
	count1s++;
	/*
		离上一次按下按钮过去11min启动黄灯闪烁，当countwaring = 11（按分钟加一）
		时有多次中断的发生，而if在countwaring = 11时只能执行一次，有SEC和count1s的判断
		flagwaringstart，通过P3.3控制防止T1在停止报警时启动
		countwaring = 60时，同理 
	*/
	if(countwaring == 11 && !SEC && count1s == 1 && flagwaringstart)  
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