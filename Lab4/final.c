#include<reg52.h>

//---------------------------------------试凑法整定 PID 参数-----------------------------------------
#define KP1 2.455//比例系数1	
#define KI1 0.230//积分系数	1	
#define KD1 0.115//微分系数1	  
#define KP2 3.185//比例系数2	
#define KI2 0.230//积分系数2		
#define KD2 0.100//微分系数	2   
#define KP3 4.995//比例系数3	
#define KI3 0.230//积分系数3		
#define KD3 0.043//微分系数	3   

#define alpha 0.3
#define KeyPort P3 //键盘数据端口
#define DataPort P0 // 数码管数据端口

//------------------------------------------端口、变量定义---------------------------------------------
sbit SEGLOCK=P2^2;//段锁存
sbit BITLOCK=P2^3;//位锁存
sbit DCOUT = P1^7;//定义 PWM 输出端口
sbit DCIN=P1^1;//T2 捕获端口，即输入霍尔脉冲
unsigned char speedflag=0; //速度计算完成标志
unsigned char d=0;//定义平均速度计算中的采样时间（计数）变量
unsigned char PulsNum=0;//脉冲数计数器
unsigned int OverFlow=0; //T2 溢出计数
unsigned datal=0; //捕捉当前计数值的低 8 位
unsigned datah=0; //捕捉当前计数值的高 8 位
unsigned long LOW; //定义低电平输出相对时长（计数值）
unsigned long HIGH; //定义高电平输出相对时长（计数值）
unsigned long CountPerMinute=59765110; //取晶振频率 11.0592M 
unsigned int speed=0; //定义电机转速
void Speedcal(unsigned a,unsigned b); //声明速度计算函数
int error[3]={0}; //定义增量式 PID 调节误差数组
int error1[3]={0};
unsigned char code Segindex[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};// 段码
unsigned char code Bitindex[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};//位码
unsigned char Temp[8]; //定义存储数码管显示值的全局数组变量
unsigned char v_temp[4]=0;//定义存储速度输入值的 4 位数组变量
unsigned int speed_t[4]=0;//定义 4 时刻速度采样存储数组变量
unsigned int speed_aver=0;//初始化采样速度平均值
int v_setpoint=-1;//初始化速度设定值
void DelayUs2x(unsigned char t);//us 级延时函数声明
void DelayMs(unsigned char t); //ms 级延时函数声明
void Display(unsigned char FirstBit,unsigned char Num);//数码管显示函数
unsigned char KeyScan(void);//键盘扫描
unsigned char KeyPro(void);//键值返回函数
unsigned char setflag;//定义键盘输入完成标志
void Init_Timer0(void);//定时器初始化

//------------------------------------------------主函数--------------------------------------------------
void main (void)
{
	unsigned char num;//键盘输入临时变量
	unsigned char j=4; //输入 4 位计数
	int P=0;//定义并初始化 P、I、D 调节输出增量值
	int I=0;
	int D=0;
	int delt=0; //P、I、D 增量总和
	Init_Timer0();//中断初始化
	while (1) //主循环
	{
		Display(0,8);//数码管显示，前 4 位为当前速度采样值，后 4 位为速度设定值
		num=KeyPro(); //循环调用按键扫描，提取允许的键值
		if(num>=0&num<=9)
		{
			Temp[j]=Segindex[num] ;//根据输入值取段码
			v_temp[j-4]=num;//存储速度设置 4 位数字
			j++;
			if(j<8)
			{
				setflag=0;
			}
			if(j==8)
			{
				setflag=1;//输入完成标志
				j=4;
			}
		}
		Speedcal(datah,datal);
		if(setflag==1)//速度输入完成后计算输入的速度设定值
		v_setpoint=v_temp[0]*1000+v_temp[1]*100+v_temp[2]*10+v_temp[3];
		if(v_setpoint==-1)//若无速度输入，速度设定值保持初始化原值-1
		{
			HIGH=10000;// //若无速度输入时初始化 PWM 输出占空比（1200 转左右）
			LOW=20000-HIGH;
		}
		if(v_setpoint<3000)
		{
		    error[2] = error[1];
			error[1] = error[0];
			error[0] = v_setpoint-speed_aver; //存储误差值
			P = KP3*(error[0]-error[1]);//计算根据 P、I、D 参数设置得到的控制量增量
			I = KI3* error[0];
			D = KD3*(error[0]-2*error[1]+error[2]);
			if(error[0]>=-800&error[0]<=800) //积分饱和控制  800
				delt=P+I+D; //计算根据 P、I、D 参数设置得到的控制量增量总和
			else
				delt=P+D; //计算根据 P、D 参数设置得到的控制量增量总和
			if((HIGH+delt)>v_setpoint+800) //设置非线性 PID 控制限制环节
				HIGH=v_setpoint;  
			else 
				HIGH+=delt;  //非限幅条件下的正常 PID 增量控制
			LOW=20000-HIGH;	//3000
		}
		else if(v_setpoint>3000&&v_setpoint<5000)	   // fenduan
		{
		    error[2] = error[1];
			error[1] = error[0];
			error[0] = v_setpoint-speed_aver; //存储误差值
			P = KP2*(error[0]-error[1]);//计算根据 P、I、D 参数设置得到的控制量增量
			I = KI2* error[0];
			D = KD2*(error[0]-2*error[1]+error[2]);
			if(error[0]>=-800&error[0]<=800) //积分饱和控制  800
				delt=P+I+D; //计算根据 P、I、D 参数设置得到的控制量增量总和
			else
				delt=P+D; //计算根据 P、D 参数设置得到的控制量增量总和
			if((HIGH+delt)>v_setpoint+800) //设置非线性 PID 控制限制环节
				HIGH=v_setpoint;  
			else 
				HIGH+=delt;  //非限幅条件下的正常 PID 增量控制
			LOW=20000-HIGH;	//3000
		}
		else //按照速度输入值进行 PID 控制平滑调速
		{
		  {
		  
			{
			error[2] = error[1];
			error[1] = error[0];
			error[0] = v_setpoint-speed_aver; //存储误差值
			P = KP1*(error[0]-error[1]);//计算根据 P、I、D 参数设置得到的控制量增量
			I = KI1* error[0];
			D = KD1*(error[0]-2*error[1]+error[2]);
			if(error[0]>=-8000&error[0]<=8000) //积分饱和控制  800
				delt=P+I+D; //计算根据 P、I、D 参数设置得到的控制量增量总和
			else
				delt=P+D; //计算根据 P、D 参数设置得到的控制量增量总和
			if((HIGH+delt)>v_setpoint+8000) //设置非线性 PID 控制限制环节
				HIGH=v_setpoint;  
			else 
				HIGH+=delt;  //非限幅条件下的正常 PID 增量控制
			LOW=20000-HIGH;	//3000
		}
	}
	}
		if(speedflag==1)//判断转速计算是否完成，若完成则为数码管显示变量赋值
		{
			unsigned k,b,s,g; //k,b,s,g 分别代表千位、百位、十位、各位
			unsigned int middle=0;//定义中间变量，代表计算的余数
			speed_aver=speed_t[0];
			/*speed_aver=(speed_t[0]+speed_t[1]+speed_t[2]+speed_t[3])/4;*/
			if((speed_aver>=0)&&(speed_aver<=9999))
			{
				k=speed_aver/1000; //获得千位数值
				Temp[0]= Segindex[k] ;
				middle=speed_aver%1000; //取得剩余 3 位数
				b=middle/100; //获得百位数值
				Temp[1]= Segindex[b] ;
				middle=middle%100; //取得剩余 2 位数
				s=middle/10; //获得十位数值
				Temp[2]= Segindex[s] ;
				g=middle%10; //获得个位数值（剩余 1 位数）
				Temp[3]= Segindex[g] ;
			}
			else //如果速度超过 10000,小于变量上限 65536
			{
				Temp[0]=0x80; //数码管首位后显示一个点'.',dp=1
			}
		}
	}
}

//--------------------------中断初始化函数与 T0，T1、T2 中断程序----------------------------
void Init_Timer0(void)//中断初始化函数
{
	EA=1; //全局中断开
	TMOD=0x11; //定时器 0 工作于方式 1
	ET0=1; //定时器 0 中断允许
	ET1 = 1;  //定时器 1 中断允许
	TR1 = 1;  //启动定时器 1
	TH1 = 0xdc; //定时器 1 计数初值设置
	TL1 = 0x00;
	T2CON=0x09; //定时器 2 工作在捕捉方式
	TH2=0x00; //定时器 2 计数初值设置
	TL2=0x00;
	ET2=1; //定时器 2 中断允许
	PT2=1; //定时器 2 中断优先级最高
	TR0=1; //启动定时器 0
	TR2=1; //启动定时器 2
}

void Timer0_isr(void) interrupt 1 //T0 中断程序
{
	if(DCOUT==1) //当前为高电平
	{
		TH0=(65536-LOW)>>8;//计数值赋为低电平时间值
		TL0=(65536-LOW); 
		DCOUT=0; //输出低电平
	}
	else if(DCOUT==0) //当前为低电平
	{
		TH0=(65536-HIGH)>>8;//计数值赋为高电平时间值
		TL0=(65536-HIGH); //计数值赋为低电平时间值
		DCOUT=1; //输出高电平
	}
}

void time1() interrupt 3 //定时器 1 中断程序，做速度采样
{
	unsigned char j=0;
	TH1 = 0x00;//计数值设置，可根据实验情况调节
	TL1 = 0xf0;
	++d;
	if(d == 1)
	{
		d=0;
		if(speedflag==1)
		{
			speed_t[3] = speed_t[2];
			speed_t[2] = speed_t[1];
			speed_t[1] = speed_t[0];
			speed_t[0] = speed;//连续时间间隔内存储 4 个速度采样值备用
		}
	}
}

void time2() interrupt 5 //T2 中断程序
{
	if(EXF2==1)  //捕捉引起的中断
	{
		PulsNum++; //脉冲个数加 1
		if(PulsNum==1) //定时器清零
		{
			OverFlow=0; //定时器 2 溢出次数计数器清零
			TH2=0;
			TL2=0;
		}
		else if(PulsNum==5) //电机转一圈后计算转速
		{
			TR2=0;  //停止计数
			datal=RCAP2L; //读取捕捉值
			datah=RCAP2H;
			TR2=1;  //启动计数
			PulsNum=0; //脉冲个数清零
		}
		EXF2=0; //清中断标志
	}
	else if(TF2) //计数溢出引起的中断
	{
		OverFlow++; //溢出次数加 1
		TF2=0; //清中断标志
	}
}

//------------延时函数、数码管扫描显示函数、速度计算函数、键盘扫描函数------------
void DelayUs2x(unsigned char t)//us 级别延时函数
{
	while(--t);
}

void DelayMs(unsigned char t)//ms 级别延时函数
{
	while(t--)
	{
		DelayUs2x(245);
		DelayUs2x(245);
	}
}

void Display(unsigned char FirstBit,unsigned char Num)//数码管扫描显示函数
{
	static unsigned char i=0;
	DataPort=0; //清空数据，防止有交替重影
	SEGLOCK=1; //段锁存
	SEGLOCK=0;
	DataPort= Bitindex[i+FirstBit]; //取位码
	BITLOCK=1; //位锁存
	BITLOCK=0;
	DataPort=Temp[i]; //取显示数据，段码
	SEGLOCK=1; //段锁存
	SEGLOCK=0;
	i++;
	if(i==Num)
		i=0;
}

void Speedcal(unsigned a,unsigned b) //速度计算函数
{
	//速度=TIMER2 每分钟计数值/（溢出计数值+电机转一圈的计数值）
	speed=CountPerMinute/(65536*OverFlow+256*a+b);
	speedflag=1;//电机转速计算完成标志置 1
}

unsigned char KeyScan(void) //键盘扫描函数，使用行列逐级扫描法
{
	unsigned char Val;
	KeyPort=0xf0;//高四位置高，低四位拉低
	if(KeyPort!=0xf0)//表示有按键按下
	{
		DelayMs(10); //去抖
		if(KeyPort!=0xf0)
		{ //表示有按键按下
			KeyPort=0xfe; //检测第一行
			if(KeyPort!=0xfe)
			{
				Val=KeyPort&0xf0;
				Val+=0x0e;
				while(KeyPort!=0xfe);
				DelayMs(10); //去抖
				while(KeyPort!=0xfe);
				return Val;
			}
			KeyPort=0xfd; //检测第二行
			if(KeyPort!=0xfd)
			{
				Val=KeyPort&0xf0;
				Val+=0x0d;
				while(KeyPort!=0xfd);
				DelayMs(10); //去抖
				while(KeyPort!=0xfd);
				return Val;
			}
			KeyPort=0xfb; //检测第三行
			if(KeyPort!=0xfb)
			{
				Val=KeyPort&0xf0;
				Val+=0x0b;
				while(KeyPort!=0xfb);
				DelayMs(10); //去抖
				while(KeyPort!=0xfb);
				return Val;
			}
			KeyPort=0xf7; //检测第四行
			if(KeyPort!=0xf7)
			{
				Val=KeyPort&0xf0;
				Val+=0x07;
				while(KeyPort!=0xf7);
				DelayMs(10); //去抖
				while(KeyPort!=0xf7);
				return Val;
			}
		}
	}
	return 0xff;
}

unsigned char KeyPro(void) //按键值处理函数，返回扫键值
{
	switch(KeyScan())
	{
		case 0x7e:return 0;break;//0 按下相应的键显示相对应的码值
		case 0x7d:return 1;break;//1
		case 0x7b:return 2;break;//2
		case 0x77:return 3;break;//3
		case 0xbe:return 4;break;//4
		case 0xbd:return 5;break;//5
		case 0xbb:return 6;break;//6
		case 0xb7:return 7;break;//7
		case 0xde:return 8;break;//8
		case 0xdd:return 9;break;//9
		default:return 0xff;break;
	}
}
