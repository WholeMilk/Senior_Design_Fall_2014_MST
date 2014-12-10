/*
 * patient.c
 *
 *  Created on: Nov 19, 2014
 *      Author: Xiao Deng
 */

#include "patient.h"
#include "trf7970.h"
#include <stdio.h>

///> patient database for storing patient tag ID as C-string
unsigned char patient[MAX_NUM_PATIENTS][TAG_ID_SIZE];

/**
 * Initialize patient database
 * This function initializes all patient to 00.
 */ 
void init_patient()
{
	int i, j;
	for( i=0; i<MAX_NUM_PATIENTS; i++ )
	{
		for( j=0; j<TAG_ID_SIZE; j++ )
		{
			patient[i][j] = 0x00;
		}
	}
}

/**
 * Print out patient data base
 * This function prints out all patient strings 
 * in the database for debug purpose.
 * 
 */ 
void print_patients()
{
	UartSendCString( "[DEBUG] All patient_id in database: -----\n");
	int i=0;
	int j=0;
	///> loop through all patient in database
	for( i=0; i<MAX_NUM_PATIENTS; i++ )
	{
		for( j=0; j<TAG_ID_SIZE; j++ )
		{
			///> print out patient string char by char
			UartPutByte( patient[i][j] );
		}
		UartPutCrlf();
	}
	UartSendCString( "[DEBUG] End of patient_id list      -----\n");
}

/**
 * Get number of patient stored in database
 * This function returns number of patients in the database currently.
 * 
 */ 
int get_patient_num()
{
	int i=0;
	int j=0;
	int counter=0;
	int empty_patient_cnt = 0;
	///> loop through all patients in database
	while( i < MAX_NUM_PATIENTS )
	{
		for( j=1; j<TAG_ID_SIZE; j++ )
		{
			if( patient[i][j] == 0x00 )
			{
				///> counter to keep tracker of consecutive 0 in patient string
				counter++;
			}
		}
		///> if tag string char are all 0, in increment empty patient counter
		if( counter >= ( TAG_ID_SIZE - 1 ) )
		{
			empty_patient_cnt++;
		}
		counter = 0;		///> reset 0 char counter
		i++;			///> increment patient index
	}
	///> return non-empty patient number
	return ( MAX_NUM_PATIENTS - empty_patient_cnt );
}

/**
 * Find patient in database
 * This function finds a given patient in the database,
 * if found, it returns its position in the database.
 * 
 * @param person String of patient to look for.
 * @param tag_size Size of the tag ID in int, to identify tyoe of tags.
 * 
 * @return patient_pos, index of patient in the database.
 * @return UNKNOWN_PATIENT
 */ 
int find_patient( unsigned char* person, int tag_size )
{
	int found = 0;			///> patient found flag
	int counter=0;			///> counter for matching char
	int patient_pos=0;		///> patient position index
	int j=0;			///> loop index

	///> loop through all patients in database
	while( patient_pos < MAX_NUM_PATIENTS )
	{
		for( j=0; j<tag_size; j++ )
		{
			if( person[j] == patient[patient_pos][j] )
			{
				///> increment counter for matching char
				counter++;
			}
		}
		///> if number of matching char equal to tag ID size, set found flag and end loop
		if( counter == tag_size )
		{
			found = 1;
			break;
		}
		///> if not, reset counter and scan next patient
		else
		{
			counter = 0;
			patient_pos++;
		}
	}
	///> if a matching patient is found
	if( found == 1 )
	{
		///> reset found flag and reutn found patient position index
		found = 0;
		return patient_pos;
	}
	///> if no patient is found, return UNKNON_PATIENT
	else
	{
		return UNKNOWN_PATIENT;
	}
}

/**
 * Check patient existance in database.
 * 
 * @param person String of patient to look for.
 * @param tag_size Size of the tag ID in int, to identify tyoe of tags.
 * 
 * @return IS_PATIENT, when patient is found in database.
 * @return NO_PATIENT, when paritne is not found in database.
 * @return UNKNOWN_PATIENT, when there is no patient in database.
 */ 
int check_patient( unsigned char* person, int tag_size )
{
	if( get_patient_num() > 0 )
	{
		if( find_patient( person, tag_size ) >= 0 )
		{
			return IS_PATIENT;
		}
		else if( find_patient( person, tag_size ) == UNKNOWN_PATIENT )
		{
			return NOT_PATIENT;
		}
	}
	return NO_PATIENT;
}

/**
 * Add patient to database.
 * Add a patient ID into database at the first empty spot found.
 * 
 * @param person String of patient to look for.
 * @param tag_size Size of the tag ID in int, to identify tyoe of tags.
 * 
 * @return ADDED_PATIENT, when patient is successfully added.
 * @return FULL_PATEINT, when patient database if full.
 * @return YOLO, when something weird is going on.
 */ 
int add_patient( unsigned char* person, int tag_size )
{
	///> get number of patients in database
	int patient_num = get_patient_num();
	///> new patient index
	int new_patient = 0;
	///> some loop index counter
	int m=0;
	int n=0;

	///> if database is not full
	if( patient_num < MAX_NUM_PATIENTS )
	{
		///> if databse is not empty
		if( patient_num != 0 )
		{
			///> loop through all patients, find the first empty patient spot
			while( m<MAX_NUM_PATIENTS )
			{
				int counter=0;
				int p=0;
				for( p=0; p<tag_size; p++ )
				{
					if( patient[m][p] == 0x00 )
					{
						counter++;
					}
				}
				if( counter == tag_size )
				{
					///> if all char in pateint are 0, save this position to new patient
					new_patient = m;
					break;
				}
				counter = 0;
				m++;
			}
		}
		///> copy patient ID char to new patient position char by char
		for( n=0; n<tag_size; n++ )
		{
			patient[ new_patient ][n] = person[n];
		}
		///> return ADDED_PATIENT
		return ADDED_PATIENT;
	}
	///> return FULL_PATIENT if database if full
	else if( patient_num == MAX_NUM_PATIENTS )
	{
		return FULL_PATIENT;
	}
	///> retun YOLO when error
	else
	{
		return YOLO;
	}
}

/**
 * Remove a patient from database.
 * Remove a patient by ID from database if it exist.
 * 
 * @param person String of patient to look for.
 * @param tag_size Size of the tag ID in int, to identify tyoe of tags.
 * 
 * @return REMOVED_PATIENT, when patient is successfully removed from database.
 * @return NOT_PATIENT, when paritne is not found in database.
 */ 
int remove_patient( unsigned char* person, int tag_size )
{
	int i;
	///> get patient index in the databse
	i = find_patient( person, tag_size );

	///> if patient is not found, return NOT_PATIENT
	if( i == UNKNOWN_PATIENT )
	{
		return NOT_PATIENT;
	}
	///> else set all char for that patient to 0 and return REMOVED_PATEINT
	else
	{
		int j=0;
		for(j=0; j<TAG_ID_SIZE; j++ )
		{
			patient[i][j] = 0x00;
		}
		return REMOVED_PATIENT;
	}
}

int authenticate_patient( int mode_sel, unsigned char* patient, int patient_size,  int tag_size )
{
	int check_patient_rtn = check_patient( patient, tag_size );
	int i = 0;
	int LED_rtn;

	if( DEBUG_MODE == 1 )
	{
		char buf[50];
		sprintf( buf, "[DEBUG] actual_tag_size = %d, tag_size = %d\n", patient_size, tag_size );
		UartSendCString( buf );
	}

	if( patient_size < tag_size )
	{
		return TAG_SIZE_ERROR;
	}

	if( mode_sel == 0 )
	{
		if( check_patient_rtn == IS_PATIENT )
		{
			UartSendCString("[INFO] Patient acknowledged, ID = ");
			for(i=0; i<tag_size; i++)
			{
				UartPutByte(patient[i]);
			}
			LED_rtn = IS_PATIENT;
		}
		else if( check_patient_rtn == NOT_PATIENT )
		{
			UartSendCString("[INFO] Unauthorized patient, ID = ");
			for(i=0; i<tag_size; i++)
			{
				UartPutByte(patient[i]);
			}
			LED_rtn = NOT_PATIENT;
		}
		else if( check_patient_rtn == NO_PATIENT )
		{
			UartSendCString("[INFO] No patient in database");
			LED_rtn = NO_PATIENT;
		}
		else
		{
			UartSendCString("[ERROR] Patient checking error");
			LED_rtn = ERROR;
		}
		UartPutCrlf();
	}
	else if( mode_sel == 1 )
	{
		if( ( check_patient_rtn == NOT_PATIENT ) || ( check_patient_rtn == NO_PATIENT ) )
		{
			int add_patient_rtn = add_patient( patient, tag_size );
			if( add_patient_rtn == ADDED_PATIENT )
			{
				UartSendCString("[INFO] Patient Added, ID = ");
				for(i=0; i<tag_size; i++)
				{
					UartPutByte(patient[i]);
				}
				LED_rtn = ADDED_PATIENT;
			}
			else if ( add_patient_rtn == FULL_PATIENT )
			{
				LED_rtn = FULL_PATIENT;
				UartSendCString("[INFO] FULL you fool");
			}
			else
			{
				LED_rtn = ERROR;
				char buf[20];
				sprintf( buf, "[DEBUG] YOLO, add_patient returned: %d", add_patient_rtn );
				UartSendCString(buf);
			}
			UartPutCrlf();
		}
		else if( check_patient_rtn == IS_PATIENT )
		{
			if( remove_patient( patient, tag_size ) == REMOVED_PATIENT )
			{
				UartSendCString("[INFO] Patient Removed, ID = ");
				for(i=0; i<tag_size; i++)
				{
					UartPutByte( patient[i] );
				}
				LED_rtn = REMOVED_PATIENT;
			}
			else
			{
				UartSendCString( "[INFO] No patient in database" );
				LED_rtn = NO_PATIENT;
			}
			UartPutCrlf();
		}
		else
		{
			UartSendCString( "[ERROR] Patient checking error" );
			UartPutCrlf();
			LED_rtn = ERROR;

		}
	}
	else
	{
		UartSendCString( "[ERROR] Invalid mode" );
		UartPutCrlf();
		LED_rtn = ERROR;
	}
	return LED_rtn;
}

void LED_red_blink()
{
	int i=0;
	for( i=0; i<LED_BLINK_RATE; i++ )
	{
		LED_14443A_ON;
		McuDelayMillisecond(50);
		LED_14443A_OFF;
		McuDelayMillisecond(50);
	}
}


void LED_green_blink()
{
	int i=0;
	for( i=0; i<LED_BLINK_RATE; i++ )
	{
		LED_15693_ON;
		McuDelayMillisecond(50);
		LED_15693_OFF;
		McuDelayMillisecond(50);
	}
}

void LED_yolo_blink()
{
	int i=0;
	for( i=0; i<LED_BLINK_RATE; i++ )
	{
		LED_15693_ON;
		LED_14443A_ON;
		McuDelayMillisecond(50);
		LED_15693_OFF;
		LED_14443A_OFF;
		McuDelayMillisecond(50);
	}
}

void LED_disco( int LED_val )
{
	switch ( LED_val )
	{
		case IS_PATIENT:
			LED_green_blink();
			break;
		case NOT_PATIENT:
			LED_red_blink();
			break;
		case ADDED_PATIENT:
			LED_green_blink();
			break;
		case REMOVED_PATIENT:
			LED_red_blink();
			break;
		case FULL_PATIENT:
			LED_yolo_blink();
			break;
		case NO_PATIENT:
			LED_yolo_blink();
			break;
		case TAG_SIZE_ERROR:
			LED_yolo_blink();
			LED_red_blink();
			break;
		default:
			LED_green_blink();
			LED_red_blink();
			LED_yolo_blink();
			break;
	}
}
