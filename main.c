
#include "stm32f0xx.h"
#include "core_cm0.h"

#define LabaBeanOS

#define task1 1
#define task3 3
#define tasksync 1
#define clearall 0xff

#include "os.h"
#define LED_ON() (GPIOA->ODR |= GPIO_ODR_4)
#define LED_OFF() (GPIOA->ODR &= ~(GPIO_ODR_4))
#define PA3High GPIOA->ODR |= GPIO_ODR_3
#define PA3Low GPIOA->ODR &= ~(GPIO_ODR_3)

/* This program is based on STM32F030F4, and it has 2 preemtable threads, which are Ptask0 and Ptask1  
   it has two round-robin threads which are Rtask2 and Rtask3.
	 Please pay attention on these 3 files: os.h, main.c and startup_stm32f030.s
	 Ptask0 is int30 inetrrupt handler and priority level is 2(totally 4 levles, level3 is minim priority level)
	 Ptask1 is int31 interrupt handler and priority level is 1 (higher priority)
	 GPIOA.4 drive a led
	 GPIOA.3 is task switching signal to measure switching time 
*/

void ALL_Config(void);


unsigned char LED1= 0;


void Ptask0(void){ /* task example 1: led1 on */ 
	/* add variable defination here */

_TaskStart
	CurID = 0;
		/* add task initialilization here */ 
  while(1){
		LED_ON();
		
		LED1 = 0;
		PA3High;
                PA3Low;
                PA3High;		
		SetEvent(task1,tasksync);
		WaitTime(1000,1);	
			  
	}
_TaskEnd
}
void Ptask1(void){ /* task eaxmple 2:led1 off, driven bytask 1 */
/* add variable defination here */

_TaskStart
	CurID = 1;
	/* add task initialilization here */ 
  while(1){
		WaitEventOr(tasksync, 0xffff, 1);
		PA3Low;
		ClearEvent(clearall);
		  WaitTime(500,2);
		  LED_ON();

		} 
_TaskEnd
}

void Rtask2(void){ /* task example 1: led1 on */ 
	/* add variable defination here */

_TaskStart
	CurID = 2;
		/* add task initialilization here */ 
  while(1){
		WaitTime(250,1);
		LED_OFF();
		SetEvent(task3,tasksync);
                WaitTime(750,2);	
		
	}
_TaskEnd
}

void Rtask3(void){ /* task example 1: led1 on */ 
	/* add variable defination here */

_TaskStart
	CurID = 3;
		/* add task initialilization here */ 
  while(1){
		WaitEventAnd(tasksync,0xffff,1);
		ClearEvent(tasksync);
		WaitTime(500,2);
		LED_OFF();

			  
	}
_TaskEnd
}


int main(void)
{	
	ALL_Config();
	
	InitTasks();
	PreemptTaskInit;
	SysTick_Config(48000); // 1ms system tick
	PreemptStart;
	TaskPrio = MAXPREEMPTS;
	while (1){
		
		switch(TaskPrio++){ //change TaskPrio value can change sequence of task execution.
			case MAXPREEMPTS: 
                           Rtask2();  
		           break;
			case (MAXPREEMPTS+1): 
                           Rtask3();
        	           break;
                        default: TaskPrio=MAXPREEMPTS; // add sleep command here
                           break;
		}
		
		;
  }		

}

/************************
Function: Initialize
************************/
void ALL_Config(void)
{
/* Set GPIO A CLOCK */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
/* Set PA4 as output   */
	GPIOA->MODER &= ~((GPIO_MODER_MODER3)|(GPIO_MODER_MODER4));
	GPIOA->MODER |= GPIO_MODER_MODER3_0|GPIO_MODER_MODER4_0; // as output
	GPIOA->OTYPER &= ~((GPIO_OTYPER_OT_3)|(GPIO_OTYPER_OT_4)); // push pull
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4|GPIO_OSPEEDER_OSPEEDR4; // 50MHz speed
}

/* This is RTOS ticker interrupt handler */
void SysTick_Handler(void){
  UpdateTimers();
}


