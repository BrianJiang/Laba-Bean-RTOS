# Laba-Bean-RTOS
World's fastest task switch RTOS, use interrupt handler as task program, and use interupt priority as preemptable mechanism
This RTOS is base on ProtoThreads created by Adam Dunkles. He is really a genius, create a simple but elegant way to simulate
re-entry of a thread by using switch-case-return, just like Wiki says: “Protothreads function as stackless, lightweight threads
providing a blocking context cheaply using minimal memory per protothread (on the order of single bytes).”. But ProtoThreads
is not  preempt able which is not convenience in some cases.
Laba Bean RTOS uses similar way of re-entry, but use hardware interrupt mechanism to save context and use interrupt priority to
preempt. 
So there comes a stackless, light weight  and preemptable RTOS with very little resource spending (5 bytes for each threads!) and 
fastest task switching time!( 78 cycles in STM32F030F4 )!  
