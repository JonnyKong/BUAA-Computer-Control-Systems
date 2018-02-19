#include<reg52.h>
#define NO1 1000
#define NO2 2000
#define NO3 3000
#define NO4 4000
#define KeyPort P3
#define dataport P0
sbit wela = P2^7;
sbit dula = P2^6;
sbit PWM_OUT = P1^4;	//PWM波输出端
unsigned char code weixuan[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};	//数码管各位的码表
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
                0x00,  //熄灭
                0x00  //自定义
                         };	
int pwm_High;	//	PWM脉冲占空比
unsigned int timer0_counter=0;	  //timer0计数
unsigned int timer1_counter=0;	   // timer1计数
unsigned int pulsnum =0;	//定时器2脉冲计数器
unsigned int timer2_flow =0; //定时器2溢出信号
int speed;	//电机转速
unsigned long min = 60000000;	//一分钟折合微秒
unsigned char flag1,flag2,flag_show;
//unsigned int datal,datah;
unsigned char datal,datah;
// unsigned char speed_leddata[7]={0x40};
unsigned char speed_leddata[8]={0x00};
unsigned char num;

unsigned int input_mode = 0;
unsigned long target_rpm = 5000;
float kp = 0.01;
float ki = 0.5;
float kd = 1.2;
int en = 0, en1 = 0, en2 = 0;
int pn = 0;
float delta;
unsigned long new_target_rpm; 
bit UartFlag;

void delay(unsigned int i)	//ms延时
{
	unsigned int m,n;
	for(m=i;m>0;m--)
		for(n=114;n>0;n--);
}
		   
void display()   //数码管显示函数
{	char i;
	for(i=0;i<8;i++)
	{
		P0=weixuan[i];	   
		wela=1;
		wela=0;
		P0=speed_leddata[i];	  
		dula=1;
		dula=0;
		delay(1);
	}

}

// unsigned char KeyScan()	//键盘扫描，带返回值的子函数
// {
// 	unsigned char cord_l,cord_h;//声明列线和行线的值的储存变量
// 	P3 = 0xf0;//1111 0000
// 	if( (P3 & 0xf0) != 0xf0)//判断是否有按键按下
// 	{
// 		delay(5);//软件消抖
// 		if( (P3 & 0xf0) != 0xf0)//判断是否有按键按下
// 		{	  
// 			  cord_l = P3 & 0xf0;// 储存列线值
// 			  P3= cord_l | 0x0f;
// 			  cord_h = P3 & 0x0f;// 储存行线值
// 			  while( (P3 & 0x0f) != 0x0f );//松手检测
// 			  return (cord_l + cord_h);//返回键值码
// 		}
// 		else return(0);	
// 	}
		
// }

// unsigned char KeyPro()
// {
	
// 	unsigned char key_value;
// 	switch( KeyScan() )
// 		{  
// 			//第一行键值码
// 		case 0xee: key_value = 0x00;
// 					break;
// 		case 0xde: key_value = 0x01;		
// 					break;
// 		case 0xbe: key_value = 0x02;		
// 					break;
// 		case 0x7e: key_value = 0x03;	
// 					break;
// 		case 0x00: key_value = 0xf0;	 //若无按键按下，返回判断值0xf0
// 		}
// 		if(key_value != 0xf0)
// 		{
// 			speed_leddata[0]=leddata[key_value];					
// 		}
// 		return (key_value);
// } 

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

void Init_timer()		//定时器初始化
{
	
	TMOD |= 0x12;	//TO 工作方式2循环计数,T1工作在方式1，16位计数
	TH0 = 156;
	TL0 = 156;	//100us定时
	TH1 = (65535-1000)/256;
	TL1 = (65535-1000)%256;	
	EA = 1;
	ET0 = 1;
	ET1 = 1;	  //定时器0,1中断允许
	TR0 = 1;
	TR1 = 1;	   //启动定时器0,1

	T2CON = 0x09;	//定时器2捕捉模式，
	TH2 = 0; //60ms循环计数
	TL2 = 0;
	ET2 = 1;
	PT2 = 1;		 //定时器2最高优先级
	TR2 = 1;
}

//定时器0中断
void timer0_PWMcontrol() interrupt 1 using 1
{
	timer0_counter++;
	if(timer0_counter == 500)			//100HzPWM波
	{
		timer0_counter =0;
		//PWM_OUT = ~PWM_OUT;
		PWM_OUT = 1;
	}
	if(pwm_High == timer0_counter)	//占空比可调
		PWM_OUT = 0;					 
}
//定时器1中断服务程序
void timer1_ledscan() interrupt 3 using 1
{
	TH1 = (65535-200)/256;
	TL1 = (65535-200)%256;
	// timer1_counter++;
	flag_show = 1;
	// if(timer1_counter == 100)			//1ms频率扫描显示
	// {
	// 	timer1_counter = 0;
	// }
}
//定时器2中断服务程序
void timer2_freport() interrupt 5 
{
	if(EXF2==1)			//当EXEN2=1时，且T2EX引脚（P1.1）出现负跳变而造成T2的捕获或重装的时候，EXF2置位并申请中断。
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
		EXF2 = 0;			//清中断标志
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
		//SCON
    REN = 1; 	//Receive allowed
    SM0 = 0;
    SM1 = 1;
    SCON = 0x50;	//01010000. mode1 10bit uart. Receive allowed
	
    EA = 1;	//Enable all interrupts
    ES = 0;	//Disable serial port interrupts
}

void SendByte(unsigned char dat) {
    SBUF = dat;
    //while(!TI);
	delay(2);
    TI = 0;
}

void SendNumber(int a) {
	if(a < 0) {
		a *= -1;
		SendByte('-');
	}
	SendByte(a / 1000 + 0x30);
	SendByte((a / 100) % 10 + 0x30);
	SendByte((a / 10) % 10 + 0x30);
	SendByte(a % 10 + 0x30);
	SendByte('\n');
}

void main()
{	
	Init_timer();
	//uart_init();
	PWM_OUT=1;
	new_target_rpm = 0;
	while(1)
	{	
		// if((num = KeyPro()) != 0xff)
		// {
		// 	switch(num)
		// 	{
		// 		case 0:	pwm_High = 5;break;	   //调整PWM占空比
		// 		case 1:	pwm_High = 25;break;
		// 		case 2:	pwm_High = 45;break;
		// 		case 3:	pwm_High = 65;break;				
		// 	}
		// }
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
				//target_rpm = 1000 * leddata[2] + 100 * leddata[3] + 10 * leddata[4] + leddata[5];
				new_target_rpm /= 10;
				target_rpm = new_target_rpm;
				new_target_rpm = 0;
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
			en = target_rpm - speed;
			//SendNumber(en);
			en += 50;
			//delta = kp * (2.40 * en - 3.5 * en1 + 1.25 * en2);
			delta = kp*(en-en1)+kp*ki*en+kp*kd*(en-2*en1+en2);
			pn += delta;
			//if(pwm_High > 500) pwm_High = 500;
			//else if(pwm_High < 0) pwm_High = 0;
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