#include <reg52.h>
#include <stdlib.h>
#define KeyPort P3
#define dataport P0
sbit wela = P2^7;
sbit dula = P2^6;
sbit PWM_OUT = P1^4;	//PWM�������
unsigned char code weixuan[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};	//����ܸ�λ�����
unsigned char code leddata[]={ 
                0x3F,  //"0"
                0x06,  //"1"
                0x5B,  //"2"
                0x4F,  //"3"
                0x66,  //"4"
                0x6D,  //"5"
                0x7D,  //"6"
                0x07,  //"7"
                0x7F,  //"8"
                0x6F,  //"9"
                0x77,  //"A"
                0x7C,  //"B"
                0x39,  //"C"
                0x5E,  //"D"
                0x79,  //"E"
                0x71,  //"F"
                0x76,  //"H"
                0x38,  //"L"
                0x37,  //"n"
                0x3E,  //"u"
                0x73,  //"P"
                0x5C,  //"o"
                0x40,  //"-"
                0x00,  //Ϩ��
                0x00  //�Զ���
                         };	
int pwm_High;						//PWM����ռ�ձ�
unsigned int timer0_counter=0;		//timer0����
unsigned int timer1_counter=0;		// timer1����
unsigned int pulsnum =0;			//��ʱ��2���������
unsigned int timer2_flow =0; 		//��ʱ��2����ź�
int speed;							//���ת��
unsigned long min = 60000000;		//һ�����ۺ�΢��
unsigned char flag1,flag2,flag_show;
unsigned char datal,datah;
unsigned char speed_leddata[8]={0x00};
unsigned char num;
int stabled = 0;
int numout = 0;

unsigned int input_mode = 0;
unsigned long target_rpm = 4000;
float kp0 = 0.01, kp1 = 0.015, kp2 = 0.02, kp3 = 0.025;
float ki0 = 0.50, ki1 = 0.50, ki2 = 0.50, ki3 = 0.50;
float kd0 = 1.20, kd1 = 1.20, kd2 = 1.20, kd3 = 1.20;
int en = 0, en1 = 0, en2 = 0;
int pn = 256;
float delta;
unsigned long new_target_rpm; 
bit UartFlag;

void delay(unsigned int i)	//��ʱ
{
	unsigned int m,n;
	for(m=i;m>0;m--)
		for(n=114;n>0;n--);
}
		   
void display()   //�������ʾ����
{	char i;
	for(i=0;i<8;i++)
	{
		wela=0 ;
		P0=weixuan[i];	   
		wela=1;
		wela=0;
		P0=speed_leddata[i];	  
		dula=1;
		dula=0;
		delay(1);
	}

}

unsigned char KeyScan() {
    unsigned char Val;
    KeyPort = 0xf0;
    if(KeyPort != 0xf0) {
        delay(10);
        if(KeyPort != 0xf0) {
            KeyPort = 0xfe;
            if(KeyPort != 0xfe) {
                Val = KeyPort & 0xf0;
                Val += 0x0e;
                while(KeyPort != 0xfe);
                delay(10);
                while(KeyPort != 0xfe);
                delay(10);
                return Val;
            }
            KeyPort = 0xfd;
            if(KeyPort != 0xfd) {
                Val = KeyPort & 0xf0;
                Val += 0x0d;
                while(KeyPort != 0xfd);
                delay(10);
                while(KeyPort != 0xfd);
                delay(10);
                return Val;
            }
            KeyPort = 0xfb;
            if(KeyPort != 0xfb) {
                Val = KeyPort & 0xf0;
                Val += 0x0b;
                while(KeyPort != 0xfb);
                delay(10);
                while(KeyPort != 0xfb);
                delay(10);
                return Val;
            }
            KeyPort = 0xf7;
            if(KeyPort != 0xf7) {
                Val = KeyPort & 0xf0;
                Val += 0x07;
                while(KeyPort != 0xf7);
                delay(10);
                while(KeyPort != 0xf7);
                delay(10);
                return Val;
            }
        }
    }
    return 0xff;
}

int KeyPro() {
    switch(KeyScan()) {
        case 0x7e: return 1; break;
        case 0x7d: return 4; break;
        case 0x7b: return 7; break;
        case 0x77: return 0; break;
        case 0xbe: return 2; break;
        case 0xbd: return 5; break;
        case 0xbb: return 8; break;
        case 0xb7: return 10; break;
        case 0xde: return 3; break;
        case 0xdd: return 6; break;
        case 0xdb: return 9; break;
        case 0xd7: return 11; break;
        case 0xee: return 12; break;
        case 0xed: return 13; break;
        case 0xeb: return 14; break;
        case 0xe7: return 15; break;
        default: return 0xff; break;
    }
}

void caculate_speed(unsigned char a,unsigned char b)
{
	speed = min/(65535*timer2_flow+256*a+b);
	flag2 = 1;
}

void Init_timer()		//��ʱ����ʼ��
{
	TMOD |= 0x12;	//TO ������ʽ2ѭ������,T1�����ڷ�ʽ1��16λ����
	TH0 = 156;
	TL0 = 156;		//100us��ʱ
	TH1 = (65535-1000)/256;
	TL1 = (65535-1000)%256;	
	EA = 1;
	ET0 = 1;
	ET1 = 1;	  	//��ʱ��0,1�ж�����
	TR0 = 1;
	TR1 = 1;	   	//������ʱ��0,1
	T2CON = 0x09;	//��ʱ��2��׽ģʽ��
	TH2 = 0; //60msѭ������
	TL2 = 0;
	ET2 = 1;
	PT2 = 1;		 //��ʱ��2������ȼ�
	TR2 = 1;
}

//��ʱ��0�ж�
void timer0_PWMcontrol() interrupt 1 using 1
{
	timer0_counter++;
	if(timer0_counter == 500)			//100HzPWM��
	{
		timer0_counter =0;
		PWM_OUT = 1;
	}
	if(pwm_High == timer0_counter)	//ռ�ձȿɵ�
		PWM_OUT = 0;					 
}
//��ʱ��1�жϷ������
void timer1_ledscan() interrupt 3 using 1
{
	TH1 = (65535-200)/256;
	TL1 = (65535-200)%256;
	flag_show = 1;
}
//��ʱ��2�жϷ������
void timer2_freport() interrupt 5 
{
	if(EXF2==1)			//��EXEN2=1ʱ����T2EX���ţ�P1.1�����ָ���������T2�Ĳ������װ��ʱ��EXF2��λ�������жϡ�
	{
		pulsnum++;
		if(pulsnum==1)
		{
			 timer2_flow = 0;
			 TH2 = 0;
			 TL2 = 0;
		}
		else if(pulsnum==5)
		{
			TR2 = 0;
			datal = RCAP2L;
			datah = RCAP2H;
			caculate_speed(datah,datal);
			TR2 = 1;
			pulsnum = 0;
		}
		EXF2 = 0;			//���жϱ�־
	}
	else if(TF2)
	{
		timer2_flow++;
		TF2 = 0;	
	}
}
void data_caculate(unsigned int x)
{
	unsigned char k,b,s,g;
	unsigned int tem;
	if((x>0) &&(x<=9999) && input_mode == 0)
	{	
		// if(x < target_rpm - 1500 || x > target_rpm + 500){
		// 	++numout;
		// }
		// if(numout >= target_rpm / 30) {
		// 	numout = 0;
		// 	stabled = 0;
		// }
		// if(x >= target_rpm - 200 && x <= target_rpm + 50) stabled = 1;			
		// if(stabled == 1) x = target_rpm + (rand() % 20) - 10;			   
		k = x/1000;
		speed_leddata[4] =leddata[k];
		tem = x%1000;
		b = tem/100;
		speed_leddata[5] =leddata[b];
		tem = tem%100;
		s = tem/10;
		speed_leddata[6] =leddata[s];
		g = tem%10;
		speed_leddata[7] =leddata[g];
		k = pwm_High/1000;
		speed_leddata[0] =leddata[k];
		tem = pwm_High%1000;
		b = tem/100;
		speed_leddata[1] =leddata[b];
		tem = tem%100;
		s = tem/10;
		speed_leddata[2] =leddata[s];
		g = tem%10;
		speed_leddata[3] =leddata[g];
	}
}

void uart_init(void) {
    TMOD = 0x20;	//00100000. Timer1. Mode 2.
    PCON = 0x80;	//SMOD = 1
    TH1 = 0xfd;	//19200 baud rate.
    TL1 = 0xfd;	//Refill after timer1 ended.
    TR1 = 1;	//Timer1 to be controlled by software
    REN = 1; 	//Receive allowed
    SM0 = 0;
    SM1 = 1;
    SCON = 0x50;	//01010000. mode1 10bit uart. Receive allowed
    EA = 1;	//Enable all interrupts
    ES = 0;	//Disable serial port interrupts
}

void main()
{	
	Init_timer();
	//uart_init();
	PWM_OUT=1;
	new_target_rpm = 0;
	while(1)
	{	
		if((num = KeyPro()) != 0xff) {
			if(input_mode == 0) {
				// Clear all display
				int i;
				for(i = 0; i < 4; ++i) {
					speed_leddata[i] = 0x00;
				}				
			}
			speed_leddata[input_mode++] = leddata[num];
			new_target_rpm += num;
			new_target_rpm *= 10;
			if(input_mode == 4) {
				input_mode = 0;
				new_target_rpm /= 10;
				target_rpm = new_target_rpm;
				new_target_rpm = 0;
				stabled = 0;
			}
		}

		if(flag2 == 1)
		{
			// Not in input mode, display rmp
			//data_caculate(speed);
			data_caculate(speed);
			flag2 = 0;
			// PID after each sampling
			en2 = en1;
			en1 = en;
			en = target_rpm - speed + 50;
			if(target_rpm < 3000) {
				delta = kp0*(en-en1)+kp0*ki0*en+kp0*kd0*(en-2*en1+en2);
			}
			else if(target_rpm < 4500) {
				delta = kp1*(en-en1)+kp1*ki1*en+kp1*kd1*(en-2*en1+en2);
			}
			else if(target_rpm < 6000) {
				delta = kp2*(en-en1)+kp2*ki2*en+kp2*kd2*(en-2*en1+en2);
			}
			else {
				delta = kp3*(en-en1)+kp3*ki3*en+kp3*kd3*(en-2*en1+en2);
			}
			pn += delta;
			if(pn >= 9000) pn = 9000;
			else if(pn < 0) pn = 0;
			pwm_High = pn >> 4;
		}
		if(flag_show == 1)
		{
			flag_show = 0;
			display();
		}
	}
}