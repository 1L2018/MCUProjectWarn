#include<REG52.h>
#define uint unsigned int 
#define uchar unsigned char 
sfr T2MOD = 0xC9;
sfr IPH = 0xB7;
uint HOUR = 0x00;
uint MIN = 0x00;
uint SEC = 0x00;
uint count1s = 0x00;//达一秒计数
uint countwaring = 0; //每min计一数，达11/60
uchar code table[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};        //共阳极数码管断码表
void delayms(uint xms)
{
   uint i,j;
   for(i=xms;i>0;i--)
         for(j=120;j>0;j--);
}
void display(void)
{
   P2=0x80;
   P0=table[SEC%10];                   //显示秒个位
   delayms(50);

   P2=0x40;
   P0=table[SEC/10];                  //显示秒十位
   delayms(50);

   P2=0x20;                                  //显示横线
   P0=0xbf;
   delayms(50);

   P2=0x10;                                          //显示分个位
   P0=table[MIN%10];
   delayms(50);

   P2=0x08;                                          //显示分十位
   P0=table[MIN/10];
   delayms(50);

   P2=0x04;                              //显示横线
   P0=0xbf;
   delayms(50);

   P2=0x02;                                           //显示时个位
   P0=table[HOUR%10];
   delayms(50);

   P2=0x01;                                          //显示时十位
   P0=table[HOUR/10];
   delayms(50);
}
void main(void){
		SP = 0x60;
		P1=0;
		T2MOD = 0x00;
		T2CON = 0x00;
		TH2 = (65536-50000)/256;
		TL2 = (65536-50000)%256;
		TCON = 0x01;
		RCAP2L = TL2;
		RCAP2H = TH2;
		IP = 0x01;
		IPH = 0x21;
		IE = 0xA1;
		TR2 = 1;
		while(1){
			display();
			if(countwaring>10 && countwaring<60)
				P1 = 0x08;
			if(countwaring>59)
				P1 = 0x04;
		}
}
void exint0(void) interrupt 0 {
	countwaring = 0;
	P1 = 0;
}
void timer2int(void) interrupt 5{
	count1s++;
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
/*sbit P16 = P1^3;
void main(){
	SP = 0x60;
	T2MOD = 0x00;
	T2CON = 0x04;
	TL2 = 0x18;TH2 = 0xFC;
	RCAP2H = 0xFC;RCAP2L = 0x18;
	IE=0xa0;
	IP = 0x20; IPH = 0x20;
	while(1){
		if(TF2){TF2 = 0;P16 = !P16;}}
	}*/