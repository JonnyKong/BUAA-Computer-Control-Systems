#include<reg52.h>
#define KeyPort P3
#define dataport P0
sbit wela = P2^7;		//位选
sbit dula = P2^6;		//段选
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
int pwm_High;						//PWM脉冲占空比
unsigned int timer0_counter=0;	  	//timer0计数
unsigned int timer1_counter=0;	   	//timer1计数
unsigned int pulsnum =0;			//定时器2脉冲计数器
unsigned int timer2_flow =0; 		//定时器2溢出信号
int speed;							//电机转速
unsigned long min = 60000000;		//一分钟折合的微秒
unsigned char flag_speed,flag_show;	//速度更新、显示置位
unsigned char datal,datah;			//临时变量
unsigned char speed_leddata[8]={0x00};	//显示数组
unsigned char num;					//临时变量

void delay(unsigned int i)	//ms延时
{
	unsigned int m,n;
	for(m=i;m>0;m--)
		for(n=114;n>0;n--);
}
		   
void display()   //数码管显示函数
{	char i;
	P0 = weixuan[i];
	wela = 1;
	wela = 0;
	P0 = speed_leddata[0];
	dula = 1;
	dula = 0;
	delay(1);
	for(i=2;i<6;i++)
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

//键盘扫描程序
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

//将键盘扫描函数的输出转换为0-15的ASCII码
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

//由计数值得到速度值
void caculate_speed(unsigned char a,unsigned char b)
{
	speed = min/(65535*timer2_flow+256*a+b);
	flag_speed = 1;
}

//定时器初始化
void Init_timer()		
{
	TMOD |= 0x12;		//TO 工作方式2循环计数,T1工作在方式1，16位计数
	TH0 = 156;
	TL0 = 156;	//100us定时
	TH1 = (65535-1000)/256;
	TL1 = (65535-1000)%256;	
	EA = 1;
	ET0 = 1;
	ET1 = 1;	 		//定时器0,1中断允许
	TR0 = 1;
	TR1 = 1;	   		//启动定时器0,1

	T2CON = 0x09;		//定时器2捕捉模式，
	TH2 = 0; 			//60ms循环计数
	TL2 = 0;
	ET2 = 1;
	PT2 = 1;		 	//定时器2最高优先级
	TR2 = 1;
}

//定时器0中断服务程序
void timer0_PWMcontrol() interrupt 1 using 1
{
	timer0_counter++;
	if(timer0_counter == 500)
	{
		timer0_counter =0;
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
	flag_show = 1;
}
//定时器2中断服务程序
void timer2_freport() interrupt 5 
{
	if(EXF2==1)			//来自编码器的外部中断
	{
		pulsnum++;
		if(pulsnum==1)
		{
			 timer2_flow = 0;
			 TH2 = 0;		//清零
			 TL2 = 0;
		}
		else if(pulsnum==5)
		{
			TR2 = 0;
			datal = RCAP2L;	//读入数据
			datah = RCAP2H;
			caculate_speed(datah,datal);
			TR2 = 1;
			pulsnum = 0;
		}
		EXF2 = 0;			//清中断标志
	}
	else if(TF2)
	{
		timer2_flow++;		//计数器2溢出
		TF2 = 0;	
	}
}

//将x写入显示数组
void data_calculate(unsigned int x)
{
	unsigned char k,b,s,g;
	unsigned int tem;
	if((x>0) &&(x<=9999) && input_mode == 0)
	{
		k = x/1000;
		speed_leddata[2] =leddata[k];
		tem = x%1000;
		b = tem/100;
		speed_leddata[3] =leddata[b];
		tem = tem%100;
		s = tem/10;
		speed_leddata[4] =leddata[s];
		g = tem%10;
		speed_leddata[5] =leddata[g];
	}
}

void main()
{	
	Init_timer();
	PWM_OUT = 1;
	while(1)
	{	
		if((num = KeyPro()) != 0xff)
		{
			//根据输入调整PWM占空比
			switch(num)
			{
				speed_leddata[0] = num;
				case 0:	pwm_High = 5;break;	   
				case 1:	pwm_High = 25;break;
				case 2:	pwm_High = 45;break;
				case 3:	pwm_High = 65;break;				
			}
		}
		//速度更新后，将新的速度写入显示数组
		if(flag_speed == 1)
		{
			data_calculate(speed);
			flag_speed = 0;
		}
		//根据定时器1刷新LED显示
		if(flag_show == 1)
		{
			flag_show = 0;
			display();
		}
	}
}