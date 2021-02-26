/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
 /*
* Author: Amanda La  
* Date: December 9th 2020
* Course: CSE321
* Purpose: Social Distance System - Scaled
* Extra modules/functions in file: turnOnBuzzer, objectVisible, noObjectVisible
*                                   
* Assignment: Project 3
*********************************
* Inputs: PC_9, PC_13
* Outputs: PB_7, PE_9
* Constraints:   Watchdog must be enabled and included into system, synchronization
*                must be integrated. Must have one preivous output perihperal 
*                a new output peripheral (buzzer), and a new input peripheral (IR sensor).
*                
*                
*********************************
* References: Nov 23rd lecture video, Oct 5th lecture video, Nov 13th lecture video
* Summary of File:
*    This file establishes the IR sensor as an interrupt input and triggers the buzzer on/off 
*    based on whether an object is detected by the IR sensor. If an object is detected, the buzzer
*    will turn on and continue to buzz for as long as the object is visible to the IR sensor.
*    If there is no object in front of the sensor, the buzzer will turn off. The system will return 
*    to a safe state through the use of watchdog. Watchdog will reset the system every 25000 microseconds. 
*    If the IR sensor detects an object, the watchdog timer will reset back to 0. 
*/


// link to headers and other files 
#include "InterruptIn.h"
#include "PinNamesTypes.h"
#include "mbed.h"
#include "mbed_thread.h"
#include "mbed_wait_api.h"

//  Additional function declarations
void turnOnBuzzer(void); 
void objectVisible(void);
void noObjectVisible(void);

Watchdog &watchMe = Watchdog::get_instance();
#define wdTimeout 25000

InterruptIn inter(PC_13); 
PwmOut buzzer(PE_9);
InterruptIn ir(PC_9, PullDown);
EventQueue e(32 * EVENTS_EVENT_SIZE);
Thread t;

int val = 0;
int boom = 0;
int main(){
    watchMe.start(wdTimeout);               // start the WD
    printf("----------Start-----------\n"); // habit to make things easier in the
    t.start(callback(&e, &EventQueue::dispatch_forever));

    //enable clock for ports C and B
    RCC->AHB2ENR |= 0x6; 

    // set up port B an output
    GPIOB->MODER &= ~(0x8000);
    GPIOB->MODER |= 0x4000;

    //set up port C an input
    GPIOC->MODER &= ~(0xC0C0000); //set pc_9 and pc_13 as input

    // trigger and destination config
    inter.rise(e.event(turnOnBuzzer)); 
    ir.rise(e.event(noObjectVisible));
    ir.fall(e.event(objectVisible));
    inter.enable_irq(); 
    ir.enable_irq();

    // turn off buzzer upon system start
    buzzer.suspend();

    while (true) {

        thread_sleep_for(20);
    }//end while
}//end main

//Additional functions

    /***************************************************************
    function trigger for IR input
    When an object is detected by the IR device, this function
    prints to system console that an object is detected by the IR device and
    buzzes to alert users of object within range of IR sensor.
    ***************************************************************/

void objectVisible(void){
    buzzer.resume();
    printf("object detected!\n");
    thread_sleep_for(200);
    wait_us(5000);
}

    /*************************************************************
    function to trigger for no object detected based on
    ISR fall, (when the input value changes to a low value).
    When an object is no longer visible, this function turns
    off the buzzer and prints to the monitor that the object is
    no longer visible.
    *******************************************************/
void noObjectVisible(void){

    printf("no object visible\n");
    buzzer.suspend();
    watchMe.kick(); // reset the watchdog
    wait_us(5000);
}

    /***************************************************************
    Used to test that buzzer is working. Press bottom left user onboard button
    to turn buzzer on and press again to turn off. Also turns on 
    an onboard LED to confirm button press was received
    ***************************************************************/
void turnOnBuzzer(void) { 
  // toggle when triggered
  val = !val;
  if (val == 1) {
    // turn on
    GPIOB->ODR |= 0x80;
    buzzer.resume();
  } else {
    // turn off
    GPIOB->ODR &= ~(0x80);
    buzzer.suspend();
  }
}

