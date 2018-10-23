#ifndef __snes_h__
#define __snes_h__

//Define SNES controller inputs
#define SNES_B  0x8000
#define SNES_Y  0x4000
#define SNES_SL  0x2000
#define SNES_ST  0x1000
#define SNES_up  0x0800
#define SNES_down  0x0400
#define SNES_left  0x0200
#define SNES_right  0x0100
#define SNES_A  0x0080
#define SNES_X  0x0040
#define SNES_L  0x0020
#define SNES_R  0x0010

unsigned short SNES_input = 0x00; 

//SNES pinout:
//clock = 0
//latch = 1
//data = 2

unsigned short player_input() {
	unsigned short SNES_input = 0;
	PORTB = SetBit(PORTB, 0, 0); //Start clock and latch low
	PORTB = SetBit(PORTB, 1, 0);
	
	PORTB = SetBit(PORTB, 1, 1); //Pulse latch to store input in storage
	PORTB = SetBit(PORTB, 1 ,0);
	
	for(unsigned char i = 0; i < 16; i++) {
		PORTB = SetBit(PORTB, 0, 1);			//set clock high
		SNES_input |= GetBit(~PINB, 2);
		PORTB = SetBit(PORTB, 0, 0);			//set clock low to prepare for next input
	}
	return SNES_input;
}
#endif