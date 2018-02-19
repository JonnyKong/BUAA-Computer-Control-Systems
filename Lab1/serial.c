#define KeyPort P1

void uart_init(void) {
    TMOD = 0x20;
    PCON = 0x80;
    TH1 = 0xfd;
    TL1 = 0xfd;
    TR1 = 1;
    REN = 1;
    SM0 = 0;
    SM1 = 1;
    SCON = 0x50;
    EA = 1;
    ES = 0;
}

void SendByte(unsigned char dat) {
    SBUF = dat;
    while(!TI);
    TI = 0;
}

void UART_SER(void) interrupt 4 {
    EA = 0;
    if(RI) {
        RI = 0;
        rdata = SBUF;
        UartFlag = 1;
    }
    if(TI) TI = 0;
}

void delay(unsigned int time) {
    unsigned int x, y;
    for(x = time; x > 0; --x) {
        for(y = 128; y > 0; --y);
    }
}

unsigned int dis_num[10] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6};
unsigned int dis_pos[8] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};

void display(unsigned int i, unsigned int j) {
    LATCH1 = 0;
    LATCH2 = 1;
    DataPort = dis_pos[i];
    LATCH1 = 1;
    LATCH2 = 0;
    DataPort = dis_num[j];
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
    uart_init();
    Init_INT0();
    ES = 1;
    while(1) {
        KeyPort = 0xf0;
        if(KeyPressFlag == 1) {
            KeyPressFlag = 0;
            num = KeyPro();
            if(num != 0xff) {
                display(0, num);
                tdata = dis_num[num];
                SendByte(tdata);
            }
        }
        if(UartFlag == 1) {
            UartFlag = 0;
            if((rdata >= '0') && (rdata <= '9')) num = rdata - 0x30;
            else if((rdata >= 'a') && (rdata <= 'f')) num = rdata - 0x57;
            else if((rdata >= 'A') && (rdata <= 'F')) num = rdata - 0x37;
            else num = sdata;
            sdata = num;
            display(2, num);
            EA = 1; 
        }
    }
}