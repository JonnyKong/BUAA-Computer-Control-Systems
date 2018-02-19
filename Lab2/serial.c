#include <reg52.h>
#define DataPort P0
#define KeyPort P1

void display(unsigned char, unsigned char);
void delay(unsigned int);
bit UartFlag;
sbit LATCH1 = P2 ^ 2;
sbit LATCH2 = P2 ^ 3;

char rdata = 0xff;
char tdata;
char sdata;

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

void UART_SER(void) interrupt 4 {
    EA = 0;
    if(RI) {	//RI=1 after all 8-bits received, also serves at interrupt_flag.
        RI = 0;
        rdata = SBUF;
        UartFlag = 1;
    }
    if(TI) TI = 0;	//Interrupt caused by completion of transfer. Clear TI and pass. 
	EA = 1; //In main function
}

void delay(unsigned int time) {
    unsigned int x, y;
    for(x = time; x > 0; --x) {
        for(y = 128; y > 0; --y);
    }
}

//unsigned int dis_num[10] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6};
//unsigned int dis_pos[8] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};
unsigned char dis_num[16] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6,0xee,0x3e,0x9c,0x7a,0x9e,0x8e};
unsigned char dis_pos[8] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};

//void display(unsigned int i, unsigned int j) {
void display(unsigned char i, unsigned char j) {
    LATCH1 = 0;
    LATCH2 = 1;
    DataPort = dis_pos[i];
    LATCH1 = 1;
    LATCH2 = 0;
    DataPort = dis_num[j];
	delay(1);
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

void main() {
    unsigned char num;
	bit displayKeyboard;
    uart_init();
    //Init_INT0();
    ES = 1;
	displayKeyboard= 0;
	UartFlag = 0;
    while(1) {
        KeyPort = 0xf0;
        //if(KeyPressFlag == 1) {
		if(KeyPort != 0xf0) {
            num = KeyPro();
            if(num != 0xff) {
                //display(0, num);
				displayKeyboard = 1;
                //tdata = dis_num[num];
                if(num <= 9) SendByte(num + 0x30);
				else SendByte(num + 0x37);
            }
        }
        if(UartFlag == 1) {
            UartFlag = 0;
			displayKeyboard = 0;
            if((rdata >= '0') && (rdata <= '9')) num = rdata - 0x30;
            else if((rdata >= 'a') && (rdata <= 'f')) num = rdata - 0x57;
            else if((rdata >= 'A') && (rdata <= 'F')) num = rdata - 0x37;
            else num = sdata;
            sdata = num;
            //display(2, num); 
        }
		if(displayKeyboard) display(0, num);
		else display(2, num);
    }
}