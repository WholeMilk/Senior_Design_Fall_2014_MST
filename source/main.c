
/**
 * Senior Design Project II - EE/CPE 4097
 * 
 * Project:	Smart Medical Identification Reader
 * 
 * Author: 	Xiao Deng 	- xddkf@mst.edu
 * 		Neshat Osmani 	- nox93@mst.edu
 * 		Justin Vance	- jpvdmb@mst.edu
 * 		Al Dohmen	- ardqm9@mst.edu
 * 
 * Advisor:	Dr. Kosbar
 * Instructor:	Dr Rosa Zheng
 * Institution:	Missouri University of Science & Technology
 * 
 */

/*
 * {main.c}
 *
 * {main application}
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

/****************************************************************
* Firmware for the TRF7970A Booster Pack on the MSP430G2553 Launch Pad.
* Rev. # 1.1 -- 27.Sep.2013
*
* DESCRIPTION: This Example detects 15693, Type A, and Type B NFC tags.
* It then indicates the Tag type through LED's on the TRF7970A Booster pack.
* The UID and the RSSI (field strength) are sent out via a UART at 9600 Baud
* and can be read on a Computer.  This Firmware assumes the IRQ SEL jumper on
* the Booster pack to be in position #2.
*
* The Trf7970 is an integrated analog front end and
* data framing system for a 13.56 MHz RFID reader system.
* Built-in programming options make it suitable for a wide range
* of applications both in proximity and vicinity RFID systems.
* The reader is configured by selecting the desired protocol in
* the control registers. Direct access to all control registers
* allows fine tuning of various reader parameters as needed.
*
* The Trf7970A is interfaced to a MSP430F2553 through a SPI (serial)
* interface using a hardware USCI. The MCU is the master device and
* initiates all communication with the reader.
*
* The anti-collision procedures (as described in the ISO
* standards 14443A/B and 15693) are implemented in the MCU
* firmware to help the reader detect and communicate with one
* PICC/VICC among several PICCs/VICCs.
*
*
* AUTHOR:   Mai Long    		DATE: 06 MAY 2013
* 			J.D. Crutchfield	DATE: 27 SEP 2013
*
* CHANGES:
* REV.    DATE        WHO    DETAIL
* 1.1	  Sep.2013    J.D.	 Fixed delay between EN high until TRF is ready
*
* BUILT WITH:
* Code Composer Studio Core Edition Version: 5.4.0.00091
* (C) Copyright Texas Instruments, 2009. All rights reserved.
*****************************************************************/

//===============================================================
// Program with hardware USART and SPI communication	        ;
// interface with TRF7970A reader chip.                         ;
//                                                              ;
// PORT1.0 - HEART BEAT LED                                     ;
// PORT1.1 - UART RX                                            ;
// PORT1.2 - UART TX                                            ;
// PORT1.5 - SPI DATACLK                                        ;
// PORT1.6 - SPI MISO (REMOVE LED2 JUMPER)                      ;
// PORT1.7 - SPI MOSI                                           ;
//                                                              ;
// PORT2.0 - IRQ (INTERUPT REQUEST from TRF7970A)               ;
// PORT2.1 - SLAVE SELECT                                       ;
// PORT2.2 - TRF7970A ENABLE                                    ;
// PORT2.3 - ISO14443B LED                                      ;
// PORT2.4 - ISO14443A LED                                      ;
// PORT2.5 - ISO15693  LED                                      ;
//===============================================================

/********** HEADER FILES **********/
//===============================================================
#include <msp430.h>                         // processor specific header
#include <stdlib.h>                         // general purpose standard library
#include <stdio.h>                          // standard input/output header
#include "iso14443a.h"
#include "iso14443b.h"
#include "iso15693.h"
#include "mcu.h"
#include "Trf7970.h"
#include "types.h"
#include "uart.h"
#include "patient.h"

/************	Smart Medical NFC Scanner Project	************/
#define SCAN_DELAY_INIT_MS 500
#define MODE_SWITCH_DELAY_MS 250
/************	Smart Medical NFC Scanner Project	************/

//===============================================================

/********** GLOBAL VARIABLES **********/
//===============================================================
u08_t buf[100];					// TX/RX BUFFER FOR TRF7970A
u08_t enable = 0;
u08_t Tag_Count;
u08_t i_reg = 0x01;             // INTERRUPT REGISTER
u08_t irq_flag = 0x00;
u08_t rx_error_flag = 0x00;
s08_t rxtx_state = 1;           // USED FOR TRANSMIT RECEIVE BYTE COUNT
u08_t host_control_flag = 0;
u08_t stand_alone_flag = 1;

/************	Smart Medical NFC Scanner Project	************/
int edit_mode = 0;			///< Edit mode switch, 1 = ON, 0 = OFF
int found_tag_ISO14443a = 0;		///< Flag for ISO14443a tag found
int found_tag_ISO15693 = 0;		///< Flag for ISO15693 tag found
int empty_scan_cnt = 0;			///< Empty scan counter, scan frequency modification
int delay_factor = 0;			///< Delay factor, scan delay modification
/************	Smart Medical NFC Scanner Project	************/

//===============================================================

void main(void)
{

	// WDT ~350ms, ACLK=1.5kHz, interval timer
	WDTCTL = WDT_ADLY_16;

	// Enable WDT interrupt
	IE1 |= WDTIE;

	SLAVE_SELECT_PORT_SET;
	SLAVE_SELECT_HIGH;

	ENABLE_PORT_SET;
	ENABLE_TRF;

	// wait until TRF7970A system clock started
	McuDelayMillisecond(2);

	// settings for communication with TRF7970A
	Trf7970CommunicationSetup();

	// Set Clock Frequency and Modulation
	Trf7970InitialSettings();

	// set the DCO to 8 MHz
	McuOscSel(1);

	// Re-configure the USART with this external clock
	Trf7970ReConfig();

	// Configure UART
	UartSetup();

	/************	Smart Medical NFC Scanner Project	************/
	McuDelayMillisecond(5);
	UartSendCString("[INFO] NFC Reader ENABLED.");
	UartPutCrlf();
	McuDelayMillisecond(2);

	P1SEL &= ~0x08;					// Select Port 1 P1.3 (push button)
	P1DIR &= ~0x08;					// Port 1 P1.3 (push button) as input, 0 is input
	P1REN |= 0x08;					// Enable Port P1.3 (push button) pull-up resistor
	P1IE |= 0x08;					// Port 1 Interrupt Enable P1.3 (push button)
	P1IFG &= ~0x08;					// Clear interrupt flag
	/************	Smart Medical NFC Scanner Project	************/
	
	// General enable interrupts
	__bis_SR_register(GIE);

	// indicates that setting are done
	enable = 1;

	// stand alone mode
	stand_alone_flag = 1;

	// launchpad LED1
	P1DIR |= BIT0;

	/************	Smart Medical NFC Scanner Project	************/
	//init function for the patient array
	init_patient();

	P1IN&=BIT3;		///< Port 1.3 (left button) as input as mode switch
	/************	Smart Medical NFC Scanner Project	************/
	
	while(1)
	{
		Tag_Count = 0;
		IRQ_OFF;
		DISABLE_TRF;

		// Enter LPM3
		__bis_SR_register(LPM3_bits);

		// launchpad LED1 - Toggle (heartbeat)
		P1OUT ^= BIT0;

		// Clear IRQ Flags before enabling TRF7970A
		IRQ_CLR;
		IRQ_ON;

		ENABLE_TRF;

		/************	Smart Medical NFC Scanner Project	************/
		// Must wait at least 4.8 ms to allow TRF7970A to initialize.
		__delay_cycles(40000);
		#ifdef ENABLE15693
				found_tag_ISO15693 = Iso15693FindTag( edit_mode );	///< Scan for 15693 tags
		#endif

		#ifdef ENABLE14443A
				found_tag_ISO14443a = Iso14443aFindTag( edit_mode );	///< Scan for 14443A tags
		#endif
		/*	We are not using 14443B type tag
		#ifdef ENABLE14443B
			  //Iso14443bFindTag();	// Scan for 14443B tags
		#endif
		*/
		
		/**
		 * Write total number of tags read to UART
		 */
		if(Tag_Count > 0){
			Tag_Count = UartNibble2Ascii(Tag_Count & 0x0F);		///< convert to ASCII
			UartSendCString("[INFO] Tags Found: ");
			UartPutChar(Tag_Count);
			UartPutCrlf();
			UartPutCrlf();
		}
		/**
		 * If either type of tag is found:
		 * reset empty scan counter,
		 * reset delay factor,
		 * delay MCU to prevent duplicate scan,
		 * reset tag found counter for both types of tag.
		 * 
		 */
		if( ( found_tag_ISO15693 == 1 ) || ( found_tag_ISO14443a == 1 ) )
		{
			empty_scan_cnt = 0;
			delay_factor = 0;
			McuDelayMillisecond( SCAN_DELAY_INIT_MS );
			found_tag_ISO15693 = 0;
			found_tag_ISO14443a = 0;
		}
		
		/**
		 * If edit mode is disabled:
		 * delay MCU by an increasing amount of time based on 
		 * delay_factor and initial delay,
		 * increase empty scan counter.
		 * 
		 */
		if( edit_mode == 0 )
		{
			//Dynamic delay for power saving
			McuDelayMillisecond( delay_factor*SCAN_DELAY_INIT_MS );

			//increment empty scan counter
			empty_scan_cnt++;
		}

		/**
		 * If debug mode is enabled:
		 * print out empty scan count.
		 * 
		 */
		if( DEBUG_MODE == 1 )
		{
			char buf[20];
			sprintf( buf, "[DEBUG] Scan#%d\r", empty_scan_cnt );
			UartSendCString( buf );
		}


		/**
		 * After 20 consecutive empty scan:
		 * reset empty scan counter,
		 * increment of delay counter if it's under threshold,
		 * print out message for additional delay occurrence.
		 * 
		 */
		if( empty_scan_cnt >= 20 )
		{
			empty_scan_cnt = 0;
			if( delay_factor < 4 )
			{
				delay_factor++;
				UartSendCString( "[DEBUG] No TAG in range, additional 500ms delay added\n" );
			}
		}
		/************	Smart Medical NFC Scanner Project	************/
	}
}

// Watchdog Timer interrupt service routine
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
   //exit LPM3
   __bic_SR_register_on_exit(LPM3_bits);
}

/********** IT'S A TRAP!!!! (ISR'S) **********/
//===============================================================

/************	Smart Medical NFC Scanner Project	************/
///> The following codes are for P1 ISR
#pragma vector= PORT1_VECTOR
__interrupt void PORT1_ISR (void)
{
	P1IFG &= ~0x08;	 					///> P1.3 Interrupt Flag clear

	while( P1IN == 0x08 );					///> debounce on P1.3
	McuDelayMillisecond( MODE_SWITCH_DELAY_MS );		///> 250ms delay to avoid accidental mdoe switch

	P1IFG &= ~0x08;	 					///> P1.3 Interrupt Flag clear

	/**
	 * If edit mode is enabled:
	 * reset edit mode flag,
	 * reset empty scan counter,
	 * reset delay counter,
	 * print out mode message,
	 * turn off edit mode LED.
	 * 
	 */
	if( edit_mode == 1 )
	{
		edit_mode=0;
		empty_scan_cnt = 0;
		delay_factor = 0;
		UartSendCString("[MODE] Verification Mode");
		UartPutCrlf();
		LED_14443B_OFF;
	}
	/**
	 * If edit mode is disbaled:
	 * set edit mode flag,
	 * reset empty scan counter,
	 * reset delay counter,
	 * print out mode message.
	 * 
	 */
	else if( edit_mode == 0 )
	{
		edit_mode=1;
		empty_scan_cnt = 0;
		delay_factor = 0;
		UartSendCString("[MODE] Edit Mode");
		UartPutCrlf();

		///> In debug mode, print out all patients currently stored in database
		if( DEBUG_MODE == 1 )
		{
			print_patients();
		}
		///> turn on edit mode LED
		LED_14443B_ON;
	}

}

#pragma vector= ADC10_VECTOR
__interrupt void ADC12_ISR (void)
{
	while(1);
}

#pragma vector= USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR (void)
{
	while(1);
}

#pragma vector= USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR (void)
{
	while(1);
}

#pragma vector= TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR (void)
{
	while(1);
}

#pragma vector= COMPARATORA_VECTOR
__interrupt void COMPARATORA_ISR (void)
{
	while(1);
}

#pragma vector= TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR (void)
{
	while(1);
}

#pragma vector= TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR (void)
{
	while(1);
}

#pragma vector= NMI_VECTOR
__interrupt void NMI_ISR (void)
{
	while(1);
}
//===============================================================


