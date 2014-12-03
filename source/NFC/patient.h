/*
 * patient.h
 *
 *  Created on: Nov 19, 2014
 *      Author: Xiao
 */

#ifndef PATIENT_H_
#define PATIENT_H_

#define LED_BLINK_RATE 10
#define MAX_NUM_PATIENTS 6
#define TAG_ID_SIZE 8
#define DEBUG_MODE 1

enum patient_rtn_status
{
	NOT_PATIENT = 0,
	IS_PATIENT = 1,
	ADDED_PATIENT = 2,
	REMOVED_PATIENT = 3,
	NO_PATIENT = -1,
	FULL_PATIENT = -2,
	UNKNOWN_PATIENT = -3,
	TAG_SIZE_ERROR = -4,
	ERROR = -10,
	YOLO = 420
};

extern unsigned char patient[MAX_NUM_PATIENTS][TAG_ID_SIZE];

void init_patient();
void print_patients();
int check_patient( unsigned char* person, int tag_size );
int add_patient( unsigned char* person, int tag_size );
int remove_patient( unsigned char* person, int tag_size );
int authenticate_patient( int mode_sel, unsigned char* patient, int patient_size,  int tag_size );
void LED_disco( int LED_val );

#endif /* PATIENT_H_ */
