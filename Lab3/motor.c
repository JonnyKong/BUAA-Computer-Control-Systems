#include <reg52.h>
#define DataPort P0
#define KeyPort P1

sbit LATCH1 = P2 ^ 2;
sbit LATCH2 = P2 ^ 3;
sbit DCOUT = P3 ^ 4;

#define CYCLE 10
unsigned char PWM_ON;   //Speed level
unsigned char PwmL = 1500;
unsigned char PwmH = 1500;

void delay(unsigned int time) {
    unsigned int x, y;
    for(x = time; x > 0; --x) {
        for(y = 128; y > 0; --y);
    }
}

unsigned char dis_num[16] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6,0xee,0x3e,0x9c,0x7a,0x9e,0x8e};
unsigned char dis_pos[8] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};

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
        // EA = 0;
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
                // EA = 1;
                // TH0 = (65536 - PwmH) >> 8;
                // TL0 = (65536 - PwmH) % 256;
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
                // EA = 1;
                // TH0 = (65536 - PwmH) >> 8;
                // TL0 = (65536 - PwmH) % 256;
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
                // EA = 1;
                // TH0 = (65536 - PwmH) >> 8;
                // TL0 = (65536 - PwmH) % 256;
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
                // EA = 1;
                // TH0 = (65536 - PwmH) >> 8;
                // TL0 = (65536 - PwmH) % 256;
                return Val;
            }
        }
    }
    // EA = 1;
    // TH0 = (65536 - PwmH) >> 8;
    // TL0 = (65536 - PwmH) % 256;
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

void Init_Timer0() {
    EA = 1;         //Enable interrupts
    TMOD = 0x11;    //b00010001. Mode 1,
    TH0 = 65536 / 256;
    TL0= 65536 % 256;
    ET0 = 1;        //Allow timer0 interrupt
    //ET1 = 1;        //Allow timer1 interrupt
    //T2CON = 0x09;   //b01010000
    //TH2 = 0x00;     //b00000000
    //TL2 = 0x00;     //b00000000
    //ET2 = 1;        //Allow timer2 interrupt
    //PT2 = 1;        //Timer2 priority
    TR0 = 1;        //Timer0 to be controlled by software
    //TR2 = 1;        //Timer2 to be controlled by software
    //TR1 = 1;        //Timer1 to be controlled by software
    //ET1 = 1;        //Allow timer1 interrupt
}

void Timer0_isr() interrupt 1 {
    if(DCOUT == 1) {
        TH0 = (65536 - PwmH) >> 8;
        TL0 = (65536 - PwmH) % 256;
        DCOUT = 0;
        display(0, 8);
    }
    else if(DCOUT == 0) {
        TH0 = (65536 - PwmL) >> 8;
        TL0 = (65536 - PwmL) % 256;
        DCOUT = 1;
        display(0, 8);
    }
    /*
    TH0 = (65536 - 1000) / 256;
    TL0 = (65536 - 1000) % 256;
    if(count == PWM_ON) {
        DCOUT = 0;
    }
    count++;
    if(count == CYCLE) {
        count = 0;
        if(PWM_ON != 0)
            DCOUT = 1;
    }
    */
}

void main() {
    PWM_ON = 0;
    Init_Timer0();
    while(1) {
        num = KeyPro();
        if(num == 1) {
            PWM_ON = 1;
            PwmH = 600;
            PwmL = 2400;
        }
        else if(num == 2) {
            PWM_ON = 3;
            PwmH = 1200;
            PwmL = 1800;
        }
        else if(num == 3) {
            PWM_ON = 5;
            PwmH = 1800;
            PwmL = 1200;
        }
        else if(num == 4) {
            PWM_ON = 7;
            PwmH = 2400;
            PwmL = 600;
        }
    }
}
