#include <reg52.h>

#define DataPort P0
sbit LATCH1 = P2 ^ 2;
sbit LATCH2 = P2 ^ 3;

void delay(unsigned int time) {
    unsigned int x, y;
    for(x = time; x > 0; --x) {
        for(y = 128; y > 0; --y);
    }
}

unsigned int dis_num[10] = {0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,0xfe,0xf6};
unsigned int dis_pos[8] = {0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};
unsigned int number1[8] = {1, 4, 0, 3, 1, 2, 5, 9};
unsigned int number2[8] = {1, 4, 0, 3, 1, 2, 3, 5};

void display(unsigned int i, unsigned int j) {
    LATCH1 = 0;
    LATCH2 = 1;
    DataPort = dis_pos[i];
    LATCH1 = 1;
    LATCH2 = 0;
    DataPort = dis_num[j];
}

void main() {
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int flag = 0;
    while(1) {	
		if(flag == 0) {
	        for(i = 0; i < 8; ++i) {
	            display(i, number1[i]);
	            delay(1);
	        }
		}
		else if(flag == 1) {
			 for(i = 0; i < 8; ++i) {
	            display(i, number2[i]);
	            delay(1);
	        }
		}
		if(++j == 128) {
			 j = 0;
			 flag ^= 1;
		}
    }
}