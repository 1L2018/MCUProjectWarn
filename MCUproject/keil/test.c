#include<REG52.h>
#define uint unsigned int 
#define uchar unsigned char 
sfr T2MOD = 0xC9;
uint count = 0;
uint min = 0;
void main(){
	SP = 0x60;
	P1=0;
	T2MOD = 0x00;
	T2CON = 0x00;
	TH2 = (65536-50000)/256;
	TL2 = (65536-50000)%256;
	TCON = 0x01;
	RCAP2L = TL2;
	RCAP2H = TH2;
	IE = 0xA1;
	TR2 = 1;
	while(1);
}
void timer2int(void) interrupt 5{
	count++;
	if(count>1199)//计数时分秒
	{	
		P2 = min;
		min++;
		count = 0;
	}
}