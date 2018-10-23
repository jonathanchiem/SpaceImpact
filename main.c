/*
 * jchie008_custom_lab_project.c
 *
 * Created: 3/1/2018 2:28:49 PM
 * Author : Jonathan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "io.c"
#include "bit.h"
#include "timer.h"
#include "scheduler.h"
#include "shiftreg.h"
#include <stdio.h>



//buttons
unsigned char up, down, left, right, shoot, st_button;

//Global Variables
unsigned char start, restart, lose;
unsigned char sPosition, e1Position, e2Position, bPosition;
unsigned char e1_hit, e2_hit;
unsigned char reloadTime = 0;

uint8_t EEMEM storedHS = 0x00;
uint8_t highscore = 0;
uint8_t ammo = 0;	//used temporarily instead of score to demonstrate EEPROM
uint8_t score = 0;

//SM for Start Menu
enum SM1_States { SM1_init, SM1_wait, SM1_menu, SM1_playing, SM1_gameover };
int SMTick1(int state) {
	
	st_button = ~PINB & 0x20;
			
	//State machine transitions
	switch(state) {
		case SM1_init:
		state = SM1_menu;
		break;
		
		case SM1_wait: {
			if(!st_button) {
				state = SM1_init;
			}
			else {
				state = SM1_wait;
			}
		}
		break;
		
		case SM1_menu:
		if(st_button) {
			LCD_ClearScreen();
			start = 1;
			state = SM1_playing;
		}
		else {
			state = SM1_menu;
		}
		break;
		
		case SM1_playing:
		if(lose) {
			state = SM1_gameover;
		}
		else if(st_button) {
			state = SM1_wait;
		}
		else {
			state = SM1_playing;
		}
		break;
		
		case SM1_gameover:
		start = 0;
		if(st_button) {
			state = SM1_init;
		}
		break;
		
		default:
		state = SM1_init;
		break;
	}
	
	//State machine actions
	switch(state) {
		case SM1_init:
		score = 0;
		start = 0;
		restart = 0;
		lose = 0;
		LCD_ClearScreen();
		break;
		
		case SM1_wait:
		break;
		
		case SM1_menu:
		LCD_DisplayString(3, "Space Impact");
		LCD_DisplayString(17, "High score: ");
		highscore = eeprom_read_byte((uint8_t*)0x00);
		LCD_WriteData(highscore+'0');
		LCD_Cursor(0);
		break;
		
		case SM1_playing:
		if(st_button) {
			restart = 1;
		}
		break;
		
		case SM1_gameover:
		LCD_ClearScreen();
		LCD_DisplayString(3, " GAME OVER");
		LCD_DisplayString(17, "Score: ");
		printScore(ammo);
		LCD_Cursor(0);
		highscore = eeprom_read_byte((uint8_t*)&storedHS);
//		if(score > highscore) {
			eeprom_update_byte((uint8_t*)&storedHS, ammo);
//		}
		highscore = eeprom_read_byte(&storedHS);
		break;
		
		default:
		break;
	}
	
	return state;
}

//SM for ship spawn
enum SM2_states { SM2_init, SM2_wait, SM2_spawn, SM2_playing };
int SMTick2(int state) {

	//State machine transitions
	switch(state) {	
		case SM2_init:
		state = SM2_wait;
		break;
		
		case SM2_wait:
		if(start) {
			state = SM2_spawn;
		}
		else {
			state = SM2_wait;
		}
		break;
		
		case SM2_spawn:
		state = SM2_playing;
		break;
		
		case SM2_playing:
		if(lose || restart) {
			state = SM2_init;
		}
		else {
			state = SM2_playing;
		}
		break;
		
		default:
		state = SM2_init;
		break;
	}
	
	//State machine actions
	switch(state) {	
		case SM2_init:
		restart = 0;
		sPosition = 1;
		break;
		
		case SM2_wait:
		break;
		
		case SM2_spawn:
		LCD_Cursor(sPosition);
		LCD_WriteData(0);
		LCD_Cursor(0);
		break;
		
		case SM2_playing:
		break;
		
		default:
		break;
		
	}
	return state;
}



//SM for ship movement; ship starts on first position in row 2 of LCD
//updates ship's current position
enum SM3_States { SM3_init, SM3_wait, SM3_up, SM3_down, SM3_left, SM3_right };
int SMTick3(int state) {
	
	up = ~PINB & 0x01;
	down = ~PINB & 0x02;
	left = ~PINB & 0x04;
	right = ~PINB & 0x08;

	//State machine transitions
	switch(state) {
		case SM3_init:
		if(start) {
			if(up) {
				state = SM3_up;
			}
			else if(down) {
				state = SM3_down;
			}
			else if(left) {
				state = SM3_left;
			}
			else if(right) {
				state = SM3_right;
			}
			else {
				state = SM3_init;
			}
		}
		break;
		
		case SM3_wait:
		if(up||down||left||right) {
			state = SM3_wait;
		}
		else {
			state = SM3_init;
		}
		break;
		
		case SM3_up:
		state = SM3_wait;
		break;
		
		case SM3_down:
		state = SM3_wait;
		break;
		
		case SM3_left:
		state = SM3_wait;
		break;
		
		case SM3_right:
		state = SM3_wait;
		break;
		
		default:
		state = SM3_init;
		break;
	}

	//State machine actions
	switch(state) {
		case SM3_init:
		break;
		
		case SM3_wait:
		break;

		case SM3_up:	//move up
		if((sPosition-16) > 0) {
			LCD_Cursor(sPosition);
			LCD_WriteData(0x20);
			LCD_Cursor(sPosition-16);
			LCD_WriteData(0);
			LCD_Cursor(0);
			sPosition -= 16;
		}
		break;
		
		case SM3_down:	//move down
		if((sPosition+16) < 33) {
			LCD_Cursor(sPosition);
			LCD_WriteData(0x20);
			LCD_Cursor(sPosition+16);
			LCD_WriteData(0);
			LCD_Cursor(0);
			sPosition += 16;
		}
		break;
		
		case SM3_left:	//move left
		if((sPosition-1) > 0 && (sPosition-1) < 16) {
			LCD_Cursor(sPosition-1);
			LCD_WriteData(0);
			LCD_WriteData(0x20);
			LCD_Cursor(0);
			sPosition--;
		}
		else if((sPosition-1) > 16) {
			LCD_Cursor(sPosition-1);
			LCD_WriteData(0);
			LCD_WriteData(0x20);
			LCD_Cursor(0);
			sPosition--;
		}
		break;
		
		case SM3_right:	//move right
		if((sPosition+1) < 17) {
			LCD_Cursor(sPosition);
			LCD_WriteData(0x20);
			LCD_Cursor(sPosition++);
			LCD_WriteData(0x20);
			LCD_WriteData(0);
			LCD_Cursor(0);
		}
		else if((sPosition+1) > 17 && (sPosition+1) < 33) {
			LCD_Cursor(sPosition);
			LCD_WriteData(0x20);
			LCD_Cursor(sPosition++);
			LCD_WriteData(0x20);
			LCD_WriteData(0);
			LCD_Cursor(0);
		}
		break;

		default:
		break;
	}
	return state;
}




//SM for ship's firing
enum SM4_States{ SM4_init, SM4_wait, SM4_shoot };
 int SMTick4(int state) {

	 shoot = ~PINB & 0x10;
	 
	 switch(state) {
		 case SM4_init:
		 if(start) {
			 state = SM4_wait;
		 }
		 else {
			 state = SM4_init;
		 }
		 break;
		 
		 case SM4_wait:
		 if(lose || restart) {
			 state = SM4_init;
		 }
		 else if(shoot) {
			 state = SM4_shoot;
		 }
		 else {
			 state = SM4_wait;
		 }
		 break;
		 
		 case SM4_shoot:
		 state = SM4_wait;
		 break;
		 
		 default:
		 state = SM4_init;
		 break;
	 }
	 
	switch(state) {
		case SM4_init:
		if(start && !lose) {
		ammo = 10;
		updateAmmo();	
		}
		break;
		 
		case SM4_wait:
		if(ammo < 10 && reloadTime == 100) {
		ammo++;
		updateAmmo();
		eeprom_update_byte((uint8_t*)&storedHS, ammo);
		reloadTime = 0;
		}
		else {
		reloadTime++;
		}
		break;
		 
		case SM4_shoot:
		if(sPosition <= 16 && ammo != 0) {	//ship in top row
			ammo--;
			updateAmmo();
			eeprom_update_byte((uint8_t*)&storedHS, ammo);
			for(bPosition = sPosition+1; bPosition < 16 ; bPosition++) {
				if(bPosition == e1Position) {
					score++;
					e1_hit = 1;
					LCD_Cursor(e1Position);
					LCD_WriteData(0x20);
				}
				else if(bPosition == e2Position) {
					score++;
					e2_hit = 1;
					LCD_Cursor(e2Position);
					LCD_WriteData(0x20);
				}
				if(bPosition > sPosition+1) {
				LCD_Cursor(bPosition-1);
				LCD_WriteData(0x20);
				}
			LCD_Cursor(bPosition);
			LCD_WriteData(2);
			delay_ms(100);
			}
			LCD_Cursor(bPosition);
			LCD_WriteData(0x20);
			LCD_Cursor(15);
			LCD_WriteData(0x20);
			LCD_Cursor(0);
		}
		else if(sPosition >= 17 && ammo != 0) {	//ship in bottom row
			ammo--;
			updateAmmo();
			eeprom_update_byte((uint8_t*)&storedHS, ammo);
			for(unsigned char bPosition = sPosition+1; bPosition < 32 ; bPosition++) {
				if(bPosition == e1Position) {
					score++;
					e1_hit = 1;
					LCD_Cursor(e1Position);
					LCD_WriteData(0x20);
				}
				else if(bPosition == e2Position) {
					score++;
					e2_hit = 1;
					LCD_Cursor(e2Position);
					LCD_WriteData(0x20);
				}
				if(bPosition > sPosition+1) {
					LCD_Cursor(bPosition-1);
					LCD_WriteData(0x20);
				}

				LCD_Cursor(bPosition);
				LCD_WriteData(2);
				delay_ms(100);
			}
			LCD_Cursor(bPosition);
			LCD_WriteData(0x20);
			LCD_Cursor(31);
			LCD_WriteData(0x20);
			LCD_Cursor(0);
		}
		break;

		default:
		break;
	}
	 return state;
 }

//SM for first enemy spawn
enum SM5_States { SM5_init, SM5_wait, SM5_spawn, SM5_move };
unsigned char st_button = 0x20;
int SMTick5(int state) {
	
	//State machine transitions
	switch(state) {
		case SM5_init:
		state = SM5_wait;
		break;
		
		case SM5_wait:
		if(start) {
			state = SM5_spawn;
		}
		else if(restart) {
			state = SM5_init;
		}
		else {
			state = SM5_wait;
		}
		break;
		
		case SM5_spawn:
		state = SM5_move;
		break;
		
		case SM5_move:
		if(sPosition == e1Position) {
			lose = 1;
		}
		if(e1_hit ) {
			state = SM5_init;
		}
		else{
			state = SM5_move;
		}
		break;
		
		default:
		state = SM5_init;
		break;
	}
	
	//State machine actions
	switch(state) {
		case SM5_init:
		e1Position = 16;
		break;
		
		case SM5_wait:
		break;
		
		case SM5_spawn:
		e1Position = 16;
		LCD_Cursor(e1Position);
		LCD_WriteData(0x20);	//clears space
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;
		
		case SM5_move:
		if(sPosition == e1Position) {
			lose = 1;
		}
		if(e1Position > 0 && e1Position <= 16 && !e1_hit) {
			LCD_Cursor(e1Position);
			LCD_WriteData(0x20);
			e1Position += 15;
			LCD_Cursor(e1Position);
			LCD_WriteData(1);
			LCD_Cursor(0);
		}
		else if(e1Position > 17 && e1Position <= 32 && !e1_hit) {
			LCD_Cursor(e1Position);
			LCD_WriteData(0x20);
			e1Position -= 17;
			LCD_Cursor(e1Position);
			LCD_WriteData(1);
			LCD_Cursor(0);
		}
		else if(e1Position == 0 || e1_hit) {
			LCD_Cursor(e1Position);
			LCD_WriteData(0x20);
		}
		break;
		
		default:
		break;
	}
	
	return state;
}

//SM for second enemy spawn
enum SM6_States { SM6_init, SM6_wait, SM6_spawn, SM6_move };
int SMTick6(int state) {
	//State machine transitions
	switch(state) {
		case SM6_init:
		state = SM6_wait;
		break;
		
		case SM6_wait:
		if(start) {
			state = SM6_spawn;
		}
		else if(restart) {
			state = SM6_init;
		}
		else {
			state = SM6_wait;
		}
		break;
		
		case SM6_spawn:
		state = SM6_move;
		break;
		
		case SM6_move:
		if(e2_hit) {
			state = SM6_init;
		}
		else{
			state = SM6_move;
		}
		break;
		
		default:
		state = SM6_init;
		break;
	}
	
	//State machine actions
	switch(state) {
		case SM6_init:
		e2Position = 32;
		break;
		
		case SM6_wait:
		break;
		
		case SM6_spawn:
		e2Position = 32;
		LCD_Cursor(e2Position);
		LCD_WriteData(0x20);	//clears space
		LCD_WriteData(1);
		LCD_Cursor(0);
		break;

		
		case SM6_move:
		if(sPosition == e2Position) {
			lose = 1;
		}
		if(e2Position > 0 && e2Position <= 16 && !e2_hit) {
			LCD_Cursor(e2Position);
			LCD_WriteData(0x20);
			e2Position += 14;
			LCD_Cursor(e2Position);
			LCD_WriteData(1);
			LCD_Cursor(0);
		}
		else if(e2Position > 17 && e2Position <= 32 && !e2_hit) {
			LCD_Cursor(e2Position);
			LCD_WriteData(0x20);
			e2Position -= 18;
			LCD_Cursor(e2Position);
			LCD_WriteData(1);
			LCD_Cursor(0);
		}
		else if(e2Position == 16 || e2_hit) {
			LCD_Cursor(e2Position);
			LCD_WriteData(0x20);
		}
		break;
		
		default:
		break;
	}
}

void updateAmmo() {
	if(ammo == 10) {
		transmit_data(0xFF);
		PORTA |= 0x30;
	}
	else if(ammo == 9) {
		transmit_data(0x7F);
		PORTA |= 0x30;
	}
	else if(ammo == 8) {
		transmit_data(0x3F);
		PORTA |= 0x30;
	}
	else if(ammo == 7) {
		transmit_data(0x1F);
		PORTA |= 0x30;
	}
	else if(ammo == 6) {
		transmit_data(0x0F);
		PORTA |= 0x30;
	}
	else if(ammo == 5) {
		transmit_data(0x07);
		PORTA |= 0x30;
	}
	else if(ammo == 4) {
		transmit_data(0x03);
		PORTA |= 0x30;
	}
	else if(ammo == 3) {
		transmit_data(0x01);
		PORTA |= 0x30;
	}
	else if(ammo == 2) {
		transmit_data(0x00);
		PORTA |= 0x30;
	}
	else if(ammo == 1) {
		transmit_data(0x00);
		PORTA |= 0x10;
	}
	else {
		PORTA = 0x00;
	}
}

void printScore(int num) {
	int tens = 0;
	int ones = 0;
	if(num > 9) {
		tens = num/10;
		ones = num%10;
	}
	else {
		ones = num;
	}
	if(tens != 0) {
		LCD_WriteData(tens+'0');
	}
	LCD_WriteData(ones+'0');
}


void drawChar(unsigned char location, unsigned char *ptr){
	LCD_WriteCommand(location);
	for(unsigned char i = 0; i < 8; ++i) {
		LCD_WriteData(ptr[ i ]);
	}
	LCD_WriteCommand(0x80);
}


int main()
{

DDRA = 0xFF; PORTA = 0x00; //LCD data lines
DDRC = 0xFF; PORTC = 0x00; //LCD output
DDRB = 0x00; PORTB = 0xFF; //button input
DDRD = 0xFF; PORTD = 0x00; //Shift register output

LCD_init(); //initialize LCD

//Custom Characters
unsigned char spaceShip[] = {0x08, 0x04, 0x0E, 0x07, 0x07, 0x0E, 0x04, 0x08};
unsigned char enemy[] = {0x00, 0x11, 0x0E, 0x17, 0x17, 0x0E, 0x11, 0x00};
unsigned char bullet[] = {0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00};

//Initialize custom characters into memory
drawChar(0x40, spaceShip);
drawChar(0x48, enemy);
drawChar(0x50, bullet);

// Period for the tasks
unsigned long int SMTick1_calc = 500;
unsigned long int SMTick2_calc = 100;
unsigned long int SMTick3_calc = 10;
unsigned long int SMTick4_calc = 10;
unsigned long int SMTick5_calc = 500;
unsigned long int SMTick6_calc = 500;


//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
tmpGCD = findGCD(tmpGCD, SMTick3_calc);
tmpGCD = findGCD(tmpGCD, SMTick4_calc);
tmpGCD = findGCD(tmpGCD, SMTick5_calc);
tmpGCD = findGCD(tmpGCD, SMTick6_calc);

//Greatest common divisor for all tasks or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;

//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;
unsigned long int SMTick2_period = SMTick2_calc/GCD;
unsigned long int SMTick3_period = SMTick3_calc/GCD;
unsigned long int SMTick4_period = SMTick4_calc/GCD;
unsigned long int SMTick5_period = SMTick5_calc/GCD;
unsigned long int SMTick6_period = SMTick6_calc/GCD;

//Declare an array of tasks 
static task task1, task2, task3, task4, task5, task6;
task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6 };
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

// Task 1
task1.state = -1;//Task initial state.
task1.period = SMTick1_period;//Task Period.
task1.elapsedTime = SMTick1_period;//Task current elapsed time.
task1.TickFct = &SMTick1;//Function pointer for the tick.

// Task 2
task2.state = -1;//Task initial state.
task2.period = SMTick2_period;//Task Period.
task2.elapsedTime = SMTick2_period;//Task current elapsed time.
task2.TickFct = &SMTick2;//Function pointer for the tick.

// Task 3
task3.state = -1;//Task initial state.
task3.period = SMTick3_period;//Task Period.
task3.elapsedTime = SMTick3_period; // Task current elasped time.
task3.TickFct = &SMTick3; // Function pointer for the tick.

// Task 4
task4.state = -1;//Task initial state.
task4.period = SMTick4_period;//Task Period.
task4.elapsedTime = SMTick4_period; // Task current elasped time.
task4.TickFct = &SMTick4; // Function pointer for the tick.

// Task 5
task5.state = -1;//Task initial state.
task5.period = SMTick5_period;//Task Period.
task5.elapsedTime = SMTick5_period; // Task current elasped time.
task5.TickFct = &SMTick5; // Function pointer for the tick.

// Task 6
task6.state = -1;//Task initial state.
task6.period = SMTick6_period;//Task Period.
task6.elapsedTime = SMTick6_period; // Task current elasped time.
task6.TickFct = &SMTick6; // Function pointer for the tick.

// Set the timer and turn it on
TimerSet(GCD);
TimerOn();

unsigned short i; // Scheduler for-loop iterator

while(1) {
	// Scheduler code
	for ( i = 0; i < numTasks; i++ ) {
		// Task is ready to tick
		if ( tasks[i]->elapsedTime == tasks[i]->period ) {
			// Setting next state for task
			tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
			// Reset the elapsed time for next tick.
			tasks[i]->elapsedTime = 0;
		}
		tasks[i]->elapsedTime += 1;
	}
	while(!TimerFlag);
	TimerFlag = 0;
}

// Error: Program should not exit!
return 0;
}


