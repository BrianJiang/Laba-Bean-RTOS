
#ifndef __OS_H__
#define __OS_H__

/* LabaBeanOS V2.0 2016/1/16 */
/* Mainly add preempt portion */
/* update to V2.1, added mutex portion */
#ifdef LabaBeanOS
#define EXTERN
#else
#define EXTERN extern
#endif

/*define total task no. and preempt task no. */
#define MAXTASKS 4
#define MAXPREEMPTS 2
        
EXTERN volatile unsigned short Wtimers[MAXTASKS];
EXTERN volatile unsigned char Event[MAXTASKS];
 /* WaitTime and WaitUntilTimeOut Timers */
// EXTERN unsigned int (*fp[MAXTASKS])(void); /* Task pointers*/
EXTERN volatile unsigned char TaskPrio; /* change secuence of round robin task execution */


/* hardware related, correspond int request set */


/* Below parameters are to decide if priprity Inversion is allowed or not */
#define PrioInversion 1
/* Below array is preempt no to interrupt vector no. if it is highest task priority level, it is
Priority Inheritance; if it is higher than task highest priority level, it is Priority Ceilings. */
#define HighestPrio 1

#ifdef LabaBeanOS
const unsigned char TaskVectorNo[MAXPREEMPTS]={30,31};     // task0 is int30, task1 is int31
const unsigned char TaskPrioAssigned[MAXPREEMPTS] = {2,1}; // task0 int priority is 1, task1 is 2
#else
extern const unsigned char TaskVectorNo[], TaskPrioAssigned[];
#endif

/* set highest priority */
#define SetHighestPrio(task) NVIC_SetPriority((IRQn_Type) TaskVectorNo[task], HighestPrio)
#define BackPrio(task) NVIC_SetPriority((IRQn_Type) TaskVectorNo[task], TaskPrioAssigned[task])


#define Task0IntSet NVIC_SetPendingIRQ((IRQn_Type)TaskVectorNo[0] )
#define Task1IntSet NVIC_SetPendingIRQ((IRQn_Type)TaskVectorNo[1] )
#define PreemptTaskInit do{ NVIC_SetPriority((IRQn_Type) TaskVectorNo[0], TaskPrioAssigned[0]);\
	                          NVIC_SetPriority((IRQn_Type) TaskVectorNo[1], TaskPrioAssigned[1]);\
	                          NVIC_EnableIRQ((IRQn_Type)TaskVectorNo[0] );                       \
	                          NVIC_EnableIRQ((IRQn_Type)TaskVectorNo[1] );                       \
                        }while(0)

#define PreemptStart NVIC_SetPendingIRQ((IRQn_Type)TaskVectorNo[0] ); \
		     NVIC_SetPendingIRQ((IRQn_Type)TaskVectorNo[1] )				

/* need add task name in curespond vector table */
#define Preempt(i) do{                             \
	             switch(i){                    \
		       case 0: Task0IntSet; break; \
	               case 1: Task1IntSet; break; \
	               default: break;             \
                     }                             \
                   }while(0)
/* hardware related portion ends */
#define _TaskStart      do{static unsigned char _cpn=0,CurID,Trigged; \
			   switch(_cpn){                              \
			        case 0:  
#define _TaskEnd            break;                          \
                          default: if(Trigged); break;      \
                        }                                   \
                        _cpn=0;                             \
                        return;}while(0);                             
                     
#define WaitTime(ticks,caseno)   _cpn=caseno;               \
                                 Wtimers[CurID] = ticks;    \
			         case caseno:               \
                                   if(Wtimers[CurID])return
																		 
#define WaitEventAnd(c, timeout, caseno) _cpn=caseno;                                   \
			                 Wtimers[CurID]=timeout;                        \
			                 case caseno:                                   \
					   if((Event[CurID]&c^c)&&Wtimers[CurID])return;\
                                           Trigged = !(Event[CurID]&c^c)																						 
																				 
#define WaitEventOr(c, timeout, caseno) _cpn=caseno;                                    \
			                Wtimers[CurID] = timeout;                       \
				        case caseno:                                    \
			                  if(!(c&Event[CurID])&&Wtimers[CurID]) return; \
					  Trigged = c&Event[CurID]
                                          						

																			
#define SetEvent(taskid,event) do{                      \
	                         __disable_irq();       \
	                         Event[taskid] |= event;\
	                          __enable_irq();       \
                                 Preempt(taskid);       \
                               }while(0)

#define ClearEvent(c) do{ __disable_irq();                   \
                          Event[CurID] &= ~(unsigned char)c; \
                           __enable_irq();}while(0)																		

#define InitTasks() do{                                    \
	              unsigned char i;                     \
	              for(i=MAXTASKS;i>0 ;i--){            \
                        Wtimers[i-1]=0;                    \
		      }                                    \
                    }while(0)
#define UpdateTimers() do{                                                    \
	                 unsigned char i;                                     \
	                 for(i=0;i<MAXTASKS;i++){                             \
		           if(Wtimers[i]&&Wtimers[i]!=(unsigned short)(-1)) { \
		             if(!--Wtimers[i]) Preempt(i);                    \
			   }                                                  \
                         }                                                    \
                       }while(0)

typedef unsigned int Mutex_F;

#define InitMutex(mut) mut=0
#define WaitMutex(mut,timeout,caseno) _cpn=caseno;                          \
			      if(CurID<MAXPREEMPTS){                        \
				__disable_irq();                            \
	                        mut |= (unsigned int)0x01<<(CurID+1);       \
				__enable_irq();                             \
			      }                                             \
			      Wtimers[CurID] = timeout;                     \
			      case caseno:                                  \
			        if(mut&0x01&&Wtimers[CurID]) return;        \
  				__disable_irq();                            \
			        Trigged= !(mut&0x01);                       \
			        mut |= 0x01;                                \
			        __enable_irq();                             \
			        if(!PrioInversion){                         \
				  __disable_irq();                          \
			          if(CurID<MAXPREEMPTS){                    \
				    SetHighestPrio(CurID);                  \
			            __enable_irq();                         \
			          }                                         \
				 }
#define ReleaseMutex(mut) do{                                               \
	                          unsigned char i;                          \
	                          __disable_irq();                          \
	                          mut &= ~(unsigned int)0x01<<(CurID+1);    \
	                          mut &= ~(unsigned int)0x01;               \
	                          for(i=1; i<(sizeof(mut)*8);i++){          \
			            if(!(mut>>i)) break;                    \
				    if(mut>>i&0x01) Preempt(i-1);           \
				    if(!PrioInversion){                     \
				      if(CurID<MAXPREEMPTS)                 \
				         BackPrio(CurID);                   \
				    }                                       \
				  }                                         \
				  __enable_irq();                           \
                          }while(0)																

		
	
	
	

 /*****Laba Bean OS**************************************/
 /*********************************************************
 1. switch case return can't be used in threads, but can be used in sub-function.
 2. In threads, some variables are need to set as "static" to save context, if are not sure of one, just use "static" variable. 
 3. A system ticker is needed, when porting different MCU, just use one timer as system ticker and add "UpdateTimers()" in its
		interrupt handler.
 4. WaitTime£¨ticks,caseno) system function, ticks value is in 1¡«0xFFFF.
 5. WaitEventAnd/Or(c,timeout,caseno)£¬c is event flag, timeout is like ticks above.
 6. WaitMutex(mut,timeout,caseno), mut is mutex, an exclusive resource, timeout is same as above
 7. when ticks or timeout is 0xffff, it means to wait forever, no time expire happened.
 8. WaitEventAnd/Or and ClearEvent are used together to clear event used.
 9. Trigged is a flag to show a WaitEvent or WaitMutex trigged or not. if Trigged is true, it means it is not a time expire case.
 10. In same thread, caseno can't be same, increasing number is good for compliler to optimize.
 11. Maxim preemptable threads number depends on MCU 'int' length, the number is sizeof(int)*8-1.
 12. Round-robin threads number is no limit. Preemptable threads number is depending on MCU. a CM0 and STM8 have 3 pr3eemptable level
     and CM3 has more than 15 preemptable level.
 
 *********************************************************/ 


#endif








