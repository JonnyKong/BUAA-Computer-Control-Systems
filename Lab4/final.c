#include<reg52.h>

//---------------------------------------�Դշ����� PID ����-----------------------------------------
#define KP1 2.455//����ϵ��1	
#define KI1 0.230//����ϵ��	1	
#define KD1 0.115//΢��ϵ��1	  
#define KP2 3.185//����ϵ��2	
#define KI2 0.230//����ϵ��2		
#define KD2 0.100//΢��ϵ��	2   
#define KP3 4.995//����ϵ��3	
#define KI3 0.230//����ϵ��3		
#define KD3 0.043//΢��ϵ��	3   

#define alpha 0.3
#define KeyPort P3 //�������ݶ˿�
#define DataPort P0 // ��������ݶ˿�

//------------------------------------------�˿ڡ���������---------------------------------------------
sbit SEGLOCK=P2^2;//������
sbit BITLOCK=P2^3;//λ����
sbit DCOUT = P1^7;//���� PWM ����˿�
sbit DCIN=P1^1;//T2 ����˿ڣ��������������
unsigned char speedflag=0; //�ٶȼ�����ɱ�־
unsigned char d=0;//����ƽ���ٶȼ����еĲ���ʱ�䣨����������
unsigned char PulsNum=0;//������������
unsigned int OverFlow=0; //T2 �������
unsigned datal=0; //��׽��ǰ����ֵ�ĵ� 8 λ
unsigned datah=0; //��׽��ǰ����ֵ�ĸ� 8 λ
unsigned long LOW; //����͵�ƽ������ʱ��������ֵ��
unsigned long HIGH; //����ߵ�ƽ������ʱ��������ֵ��
unsigned long CountPerMinute=59765110; //ȡ����Ƶ�� 11.0592M 
unsigned int speed=0; //������ת��
void Speedcal(unsigned a,unsigned b); //�����ٶȼ��㺯��
int error[3]={0}; //��������ʽ PID �����������
int error1[3]={0};
unsigned char code Segindex[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};// ����
unsigned char code Bitindex[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};//λ��
unsigned char Temp[8]; //����洢�������ʾֵ��ȫ���������
unsigned char v_temp[4]=0;//����洢�ٶ�����ֵ�� 4 λ�������
unsigned int speed_t[4]=0;//���� 4 ʱ���ٶȲ����洢�������
unsigned int speed_aver=0;//��ʼ�������ٶ�ƽ��ֵ
int v_setpoint=-1;//��ʼ���ٶ��趨ֵ
void DelayUs2x(unsigned char t);//us ����ʱ��������
void DelayMs(unsigned char t); //ms ����ʱ��������
void Display(unsigned char FirstBit,unsigned char Num);//�������ʾ����
unsigned char KeyScan(void);//����ɨ��
unsigned char KeyPro(void);//��ֵ���غ���
unsigned char setflag;//�������������ɱ�־
void Init_Timer0(void);//��ʱ����ʼ��

//------------------------------------------------������--------------------------------------------------
void main (void)
{
	unsigned char num;//����������ʱ����
	unsigned char j=4; //���� 4 λ����
	int P=0;//���岢��ʼ�� P��I��D �����������ֵ
	int I=0;
	int D=0;
	int delt=0; //P��I��D �����ܺ�
	Init_Timer0();//�жϳ�ʼ��
	while (1) //��ѭ��
	{
		Display(0,8);//�������ʾ��ǰ 4 λΪ��ǰ�ٶȲ���ֵ���� 4 λΪ�ٶ��趨ֵ
		num=KeyPro(); //ѭ�����ð���ɨ�裬��ȡ����ļ�ֵ
		if(num>=0&num<=9)
		{
			Temp[j]=Segindex[num] ;//��������ֵȡ����
			v_temp[j-4]=num;//�洢�ٶ����� 4 λ����
			j++;
			if(j<8)
			{
				setflag=0;
			}
			if(j==8)
			{
				setflag=1;//������ɱ�־
				j=4;
			}
		}
		Speedcal(datah,datal);
		if(setflag==1)//�ٶ�������ɺ����������ٶ��趨ֵ
		v_setpoint=v_temp[0]*1000+v_temp[1]*100+v_temp[2]*10+v_temp[3];
		if(v_setpoint==-1)//�����ٶ����룬�ٶ��趨ֵ���ֳ�ʼ��ԭֵ-1
		{
			HIGH=10000;// //�����ٶ�����ʱ��ʼ�� PWM ���ռ�ձȣ�1200 ת���ң�
			LOW=20000-HIGH;
		}
		if(v_setpoint<3000)
		{
		    error[2] = error[1];
			error[1] = error[0];
			error[0] = v_setpoint-speed_aver; //�洢���ֵ
			P = KP3*(error[0]-error[1]);//������� P��I��D �������õõ��Ŀ���������
			I = KI3* error[0];
			D = KD3*(error[0]-2*error[1]+error[2]);
			if(error[0]>=-800&error[0]<=800) //���ֱ��Ϳ���  800
				delt=P+I+D; //������� P��I��D �������õõ��Ŀ����������ܺ�
			else
				delt=P+D; //������� P��D �������õõ��Ŀ����������ܺ�
			if((HIGH+delt)>v_setpoint+800) //���÷����� PID �������ƻ���
				HIGH=v_setpoint;  
			else 
				HIGH+=delt;  //���޷������µ����� PID ��������
			LOW=20000-HIGH;	//3000
		}
		else if(v_setpoint>3000&&v_setpoint<5000)	   // fenduan
		{
		    error[2] = error[1];
			error[1] = error[0];
			error[0] = v_setpoint-speed_aver; //�洢���ֵ
			P = KP2*(error[0]-error[1]);//������� P��I��D �������õõ��Ŀ���������
			I = KI2* error[0];
			D = KD2*(error[0]-2*error[1]+error[2]);
			if(error[0]>=-800&error[0]<=800) //���ֱ��Ϳ���  800
				delt=P+I+D; //������� P��I��D �������õõ��Ŀ����������ܺ�
			else
				delt=P+D; //������� P��D �������õõ��Ŀ����������ܺ�
			if((HIGH+delt)>v_setpoint+800) //���÷����� PID �������ƻ���
				HIGH=v_setpoint;  
			else 
				HIGH+=delt;  //���޷������µ����� PID ��������
			LOW=20000-HIGH;	//3000
		}
		else //�����ٶ�����ֵ���� PID ����ƽ������
		{
		  {
		  
			{
			error[2] = error[1];
			error[1] = error[0];
			error[0] = v_setpoint-speed_aver; //�洢���ֵ
			P = KP1*(error[0]-error[1]);//������� P��I��D �������õõ��Ŀ���������
			I = KI1* error[0];
			D = KD1*(error[0]-2*error[1]+error[2]);
			if(error[0]>=-8000&error[0]<=8000) //���ֱ��Ϳ���  800
				delt=P+I+D; //������� P��I��D �������õõ��Ŀ����������ܺ�
			else
				delt=P+D; //������� P��D �������õõ��Ŀ����������ܺ�
			if((HIGH+delt)>v_setpoint+8000) //���÷����� PID �������ƻ���
				HIGH=v_setpoint;  
			else 
				HIGH+=delt;  //���޷������µ����� PID ��������
			LOW=20000-HIGH;	//3000
		}
	}
	}
		if(speedflag==1)//�ж�ת�ټ����Ƿ���ɣ��������Ϊ�������ʾ������ֵ
		{
			unsigned k,b,s,g; //k,b,s,g �ֱ����ǧλ����λ��ʮλ����λ
			unsigned int middle=0;//�����м������������������
			speed_aver=speed_t[0];
			/*speed_aver=(speed_t[0]+speed_t[1]+speed_t[2]+speed_t[3])/4;*/
			if((speed_aver>=0)&&(speed_aver<=9999))
			{
				k=speed_aver/1000; //���ǧλ��ֵ
				Temp[0]= Segindex[k] ;
				middle=speed_aver%1000; //ȡ��ʣ�� 3 λ��
				b=middle/100; //��ð�λ��ֵ
				Temp[1]= Segindex[b] ;
				middle=middle%100; //ȡ��ʣ�� 2 λ��
				s=middle/10; //���ʮλ��ֵ
				Temp[2]= Segindex[s] ;
				g=middle%10; //��ø�λ��ֵ��ʣ�� 1 λ����
				Temp[3]= Segindex[g] ;
			}
			else //����ٶȳ��� 10000,С�ڱ������� 65536
			{
				Temp[0]=0x80; //�������λ����ʾһ����'.',dp=1
			}
		}
	}
}

//--------------------------�жϳ�ʼ�������� T0��T1��T2 �жϳ���----------------------------
void Init_Timer0(void)//�жϳ�ʼ������
{
	EA=1; //ȫ���жϿ�
	TMOD=0x11; //��ʱ�� 0 �����ڷ�ʽ 1
	ET0=1; //��ʱ�� 0 �ж�����
	ET1 = 1;  //��ʱ�� 1 �ж�����
	TR1 = 1;  //������ʱ�� 1
	TH1 = 0xdc; //��ʱ�� 1 ������ֵ����
	TL1 = 0x00;
	T2CON=0x09; //��ʱ�� 2 �����ڲ�׽��ʽ
	TH2=0x00; //��ʱ�� 2 ������ֵ����
	TL2=0x00;
	ET2=1; //��ʱ�� 2 �ж�����
	PT2=1; //��ʱ�� 2 �ж����ȼ����
	TR0=1; //������ʱ�� 0
	TR2=1; //������ʱ�� 2
}

void Timer0_isr(void) interrupt 1 //T0 �жϳ���
{
	if(DCOUT==1) //��ǰΪ�ߵ�ƽ
	{
		TH0=(65536-LOW)>>8;//����ֵ��Ϊ�͵�ƽʱ��ֵ
		TL0=(65536-LOW); 
		DCOUT=0; //����͵�ƽ
	}
	else if(DCOUT==0) //��ǰΪ�͵�ƽ
	{
		TH0=(65536-HIGH)>>8;//����ֵ��Ϊ�ߵ�ƽʱ��ֵ
		TL0=(65536-HIGH); //����ֵ��Ϊ�͵�ƽʱ��ֵ
		DCOUT=1; //����ߵ�ƽ
	}
}

void time1() interrupt 3 //��ʱ�� 1 �жϳ������ٶȲ���
{
	unsigned char j=0;
	TH1 = 0x00;//����ֵ���ã��ɸ���ʵ���������
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
			speed_t[0] = speed;//����ʱ�����ڴ洢 4 ���ٶȲ���ֵ����
		}
	}
}

void time2() interrupt 5 //T2 �жϳ���
{
	if(EXF2==1)  //��׽������ж�
	{
		PulsNum++; //��������� 1
		if(PulsNum==1) //��ʱ������
		{
			OverFlow=0; //��ʱ�� 2 �����������������
			TH2=0;
			TL2=0;
		}
		else if(PulsNum==5) //���תһȦ�����ת��
		{
			TR2=0;  //ֹͣ����
			datal=RCAP2L; //��ȡ��׽ֵ
			datah=RCAP2H;
			TR2=1;  //��������
			PulsNum=0; //�����������
		}
		EXF2=0; //���жϱ�־
	}
	else if(TF2) //�������������ж�
	{
		OverFlow++; //��������� 1
		TF2=0; //���жϱ�־
	}
}

//------------��ʱ�����������ɨ����ʾ�������ٶȼ��㺯��������ɨ�躯��------------
void DelayUs2x(unsigned char t)//us ������ʱ����
{
	while(--t);
}

void DelayMs(unsigned char t)//ms ������ʱ����
{
	while(t--)
	{
		DelayUs2x(245);
		DelayUs2x(245);
	}
}

void Display(unsigned char FirstBit,unsigned char Num)//�����ɨ����ʾ����
{
	static unsigned char i=0;
	DataPort=0; //������ݣ���ֹ�н�����Ӱ
	SEGLOCK=1; //������
	SEGLOCK=0;
	DataPort= Bitindex[i+FirstBit]; //ȡλ��
	BITLOCK=1; //λ����
	BITLOCK=0;
	DataPort=Temp[i]; //ȡ��ʾ���ݣ�����
	SEGLOCK=1; //������
	SEGLOCK=0;
	i++;
	if(i==Num)
		i=0;
}

void Speedcal(unsigned a,unsigned b) //�ٶȼ��㺯��
{
	//�ٶ�=TIMER2 ÿ���Ӽ���ֵ/���������ֵ+���תһȦ�ļ���ֵ��
	speed=CountPerMinute/(65536*OverFlow+256*a+b);
	speedflag=1;//���ת�ټ�����ɱ�־�� 1
}

unsigned char KeyScan(void) //����ɨ�躯����ʹ��������ɨ�跨
{
	unsigned char Val;
	KeyPort=0xf0;//����λ�øߣ�����λ����
	if(KeyPort!=0xf0)//��ʾ�а�������
	{
		DelayMs(10); //ȥ��
		if(KeyPort!=0xf0)
		{ //��ʾ�а�������
			KeyPort=0xfe; //����һ��
			if(KeyPort!=0xfe)
			{
				Val=KeyPort&0xf0;
				Val+=0x0e;
				while(KeyPort!=0xfe);
				DelayMs(10); //ȥ��
				while(KeyPort!=0xfe);
				return Val;
			}
			KeyPort=0xfd; //���ڶ���
			if(KeyPort!=0xfd)
			{
				Val=KeyPort&0xf0;
				Val+=0x0d;
				while(KeyPort!=0xfd);
				DelayMs(10); //ȥ��
				while(KeyPort!=0xfd);
				return Val;
			}
			KeyPort=0xfb; //��������
			if(KeyPort!=0xfb)
			{
				Val=KeyPort&0xf0;
				Val+=0x0b;
				while(KeyPort!=0xfb);
				DelayMs(10); //ȥ��
				while(KeyPort!=0xfb);
				return Val;
			}
			KeyPort=0xf7; //��������
			if(KeyPort!=0xf7)
			{
				Val=KeyPort&0xf0;
				Val+=0x07;
				while(KeyPort!=0xf7);
				DelayMs(10); //ȥ��
				while(KeyPort!=0xf7);
				return Val;
			}
		}
	}
	return 0xff;
}

unsigned char KeyPro(void) //����ֵ������������ɨ��ֵ
{
	switch(KeyScan())
	{
		case 0x7e:return 0;break;//0 ������Ӧ�ļ���ʾ���Ӧ����ֵ
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
