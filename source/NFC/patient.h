/*
 * patient.h
 *
 *  Created on: Nov 19, 2014
 *      Author: Xiao Deng
 */

///> Header define protection
#ifndef _PATIENT_H_
#define _PATIENT_H_

#define LED_BLINK_RATE 10		///> Number of times LED blinks
#define MAX_NUM_PATIENTS 6		///> Maximum number of patients support in the database
#define TAG_ID_SIZE 8			///> Maximum supported tag ID size, bytes
#define DEBUG_MODE 1			///> Debug mode flag, 1 = Enabled, 0 = Disabled

/**
 * Patient return status enumerable
 * Standard return values for functions in this program,
 * valid returns are positive,
 * error returns are negative.
 */
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
	YOLO = -420
};

///> patient database as array of C-strings
extern unsigned char patient[MAX_NUM_PATIENTS][TAG_ID_SIZE];

///> Function prototypes, detail infromation for each fucntion please see patient.c
void init_patient();
void print_patients();
int check_patient( unsigned char* person, int tag_size );
int add_patient( unsigned char* person, int tag_size );
int remove_patient( unsigned char* person, int tag_size );
int authenticate_patient( int mode_sel, unsigned char* patient, int patient_size,  int tag_size );
void LED_disco( int LED_val );

#endif /* _PATIENT_H_ */
