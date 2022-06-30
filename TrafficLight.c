// Lab5.c starter program EE319K Lab 5, Spring 2022
// Runs on TM4C123
// Desheng Liu, Andrew Kim
// Last Modified: 1/11/2021

/* Option A1, connect LEDs to PB5-PB0, switches to PA5-3, walk LED PF321
   Option A2, connect LEDs to PB5-PB0, switches to PA4-2, walk LED PF321
   Option A6, connect LEDs to PB5-PB0, switches to PE3-1, walk LED PF321
   Option A5, connect LEDs to PB5-PB0, switches to PE2-0, walk LED PF321
   Option B4, connect LEDs to PE5-PE0, switches to PC7-5, walk LED PF321
   Option B3, connect LEDs to PE5-PE0, switches to PC6-4, walk LED PF321
   Option B1, connect LEDs to PE5-PE0, switches to PA5-3, walk LED PF321
   Option B2, connect LEDs to PE5-PE0, switches to PA4-2, walk LED PF321
  */
// east/west red light connected to bit 5
// east/west yellow light connected to bit 4
// east/west green light connected to bit 3
// north/south red light connected to bit 2
// north/south yellow light connected to bit 1
// north/south green light connected to bit 0
// pedestrian detector connected to most significant bit (1=pedestrian present)
// north/south car detector connected to middle bit (1=car present)
// east/west car detector connected to least significant bit (1=car present)
// "walk" light connected to PF3-1 (built-in white LED)
// "don't walk" light connected to PF3-1 (built-in red LED)
#include <stdint.h>
#include "SysTick.h"
#include "Lab5grader.h"
#include "../inc/tm4c123gh6pm.h"
// put both EIDs in the next two lines
char EID1[] = "dl36526"; //  ;replace abc123 with your EID
char EID2[] = "ak46428"; //  ;replace abc123 with your EID

void DisableInterrupts(void);
void EnableInterrupts(void);

#define SENSOR (*((volatile uint32_t *) 0x4002400C))
#define LIGHT  (*((volatile uint32_t *) 0x400050FC))

struct State {
	uint32_t OutE;
	uint32_t OutF;
	uint32_t WaitTime;
	uint32_t Next[8];
};
typedef struct State State_t;
#define Safe 0
#define goSouth 1
#define waitSouth 2
#define stopSouth 3
#define goWest 4
#define waitWest 5
#define stopWest 6
#define goWalk 7
#define warnZero 8
#define warnOne 9
#define warnTwo 10
#define warnThree 11
#define warnFour 12
#define warnFive 13
#define stopWalk 14

State_t FSM[15]={
{0x24, 0x02, 50, {Safe, goSouth, goWest, goSouth, goWalk, goWalk, goWalk, goSouth}}, //Safe
{0x21, 0x02, 70, {waitSouth, goSouth, waitSouth, waitSouth, waitSouth, waitSouth, waitSouth, waitSouth}}, // Go South
{0x22, 0x02, 30, {stopSouth, stopSouth, stopSouth, stopSouth, stopSouth, stopSouth, stopSouth, stopSouth}}, //Wait South
{0x24, 0x02, 50, {Safe, goSouth, goWest, goWest, goWalk, goWalk, goWalk, goWalk}}, //Stop South
{0x0C, 0x02, 70, {waitWest, waitWest, goWest, waitWest, waitWest, waitWest, waitWest, waitWest}}, //Go West
{0x14, 0x02, 30, {stopWest, stopWest, stopWest, stopWest, stopWest, stopWest, stopWest, stopWest}}, //Wait West
{0x24, 0x02, 50, {Safe, goSouth, goWest, goSouth, goWalk, goWalk, goWalk, goSouth}}, // Stop West
{0x24, 0x0E, 70, {warnZero, warnZero, warnZero, warnZero, warnZero, warnZero, warnZero, warnZero}}, // Go Walk
{0x24, 0x02, 20, {warnOne, warnOne, warnOne, warnOne, warnOne, warnOne, warnOne, warnOne}}, // Warn0
{0x24, 0x00, 20, {warnTwo, warnTwo, warnTwo, warnTwo, warnTwo, warnTwo, warnTwo, warnTwo}}, //Warn1
{0x24, 0x02, 20, {warnThree, warnThree, warnThree, warnThree, warnThree, warnThree, warnThree, warnThree}},//Warn2
{0x24, 0x00, 20, {warnFour, warnFour, warnFour, warnFour, warnFour, warnFour, warnFour, warnFour}}, //Warn3
{0x24, 0x02, 20, {warnFive, warnFive, warnFive, warnFive, warnFive, warnFive, warnFive, warnFive}}, //Warn4
{0x24, 0x00, 20, {stopWalk, stopWalk, stopWalk, stopWalk, stopWalk, stopWalk, stopWalk, stopWalk}}, //Warn5
{0x24, 0x02, 50, {Safe, goSouth, goWest, goSouth, goWalk, goSouth, goWest, goWest}}, //Stop Walk
};
uint32_t S; //index to the current state
uint32_t Input; //just the input

int main(void){ 
	volatile uint32_t delay;
  DisableInterrupts();
  TExaS_Init(GRADER);
  SysTick_Init();   // Initialize SysTick for software waits
  // initialize system
	SYSCTL_RCGCGPIO_R |= 0x34; //turns on clock for port E and F
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_DIR_R |= 0x3F;     //all outputs
  GPIO_PORTE_DEN_R |= 0x3F; 
	
  GPIO_PORTF_DIR_R |= 0x0E;   // all outputs  
  GPIO_PORTF_DEN_R |= 0x0E; 
  
	GPIO_PORTC_DIR_R &= ~0x70;	  //all inputs
	GPIO_PORTC_DEN_R |= 0x70;
	S = Safe;
	
  EnableInterrupts(); 
  while(1){
		// 1) output
		GPIO_PORTE_DATA_R = FSM[S].OutE;
		GPIO_PORTF_DATA_R = FSM[S].OutF;
    // 2) wait
		SysTick_Wait10ms(FSM[S].WaitTime);
    // 3) input
		Input = (GPIO_PORTC_DATA_R&0x70)>>4;
    // 4) next
		S = FSM[S].Next[Input];

  }
}



