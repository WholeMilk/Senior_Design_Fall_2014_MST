/*
 * patient.c
 *
 *  Created on: Nov 19, 2014
 *      Author: Xiao Deng
 */

#include "patient.h"
#include "trf7970.h"
#include <stdio.h>

///> patient database for storing patient tag ID as a C-string array
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
 * @return patient_pos Index of patient in the database.
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
 * @return IS_PATIENT When patient is found in database.
 * @return NO_PATIENT When paritne is not found in database.
 * @return UNKNOWN_PATIENT When there is no patient in database.
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
 * @return ADDED_PATIENT When patient is successfully added.
 * @return FULL_PATEINT When patient database if full.
 * @return YOLO When something weird is going on.
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
 * @return REMOVED_PATIENT When patient is successfully removed from database.
 * @return NOT_PATIENT When paritne is not found in database.
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

/**
 * Authenticate patient besed on its ID, ID size and tag size.
 * This function checks patient ID against database and output response according
 * to if the patient is in the database or not, also acknowledge empty database.
 * 
 * @param mode_sel Mode select flag, 1 = Edit mode, 0 = Authentication mode.
 * @param person String of patient to look for.
 * @param patient_size Size of the actual input patient ID as int.
 * @param tag_size Size of the type of patient tag in int, to identify tyoe of tags.
 * 
 * @return IS_PATEINT If patient is found in Authentication mode.
 * @return NOT_PATEINT If patient is not found in Authentication mode.
 * @return ADDED_PATIENT If patient is added in Edit mode.
 * @return REMOVED_PATIENT If patient is removed in Edit mode.
 * @return FULL_PATIENT If trying to add a patient when patient database is full in Edit mode.
 * 
 * @return NO_PATEINT If patient database is empty in either mode.
 * @return ERROR If an error has occured.
 * 
 */ 
int authenticate_patient( int mode_sel, unsigned char* patient, int patient_size,  int tag_size )
{
	///> check if patient is in database
	int check_patient_rtn = check_patient( patient, tag_size );
	int i = 0;
	int LED_rtn;

	///> print out tag size message in debug mode
	if( DEBUG_MODE == 1 )
	{
		char buf[50];
		sprintf( buf, "[DEBUG] actual_tag_size = %d, tag_size = %d\n", patient_size, tag_size );
		UartSendCString( buf );
	}
	///> if tag size conflict occures, return TAG_SIZE_ERROR
	if( patient_size < tag_size )
	{
		return TAG_SIZE_ERROR;
	}

	///> In Authentication mode,
	if( mode_sel == 0 )
	{
		///> if patient is found to be an existing patient in database,
		if( check_patient_rtn == IS_PATIENT )
		{
			///> print out patient ID
			UartSendCString("[INFO] Patient acknowledged, ID = ");
			for(i=0; i<tag_size; i++)
			{
				UartPutByte(patient[i]);
			}
			///> blink green LED indicating patient is acknowledged
			LED_rtn = IS_PATIENT;
		}
		///> else if patient is not a patient in database,
		else if( check_patient_rtn == NOT_PATIENT )
		{
			///> print out patient ID
			UartSendCString("[INFO] Unauthorized patient, ID = ");
			for(i=0; i<tag_size; i++)
			{
				UartPutByte(patient[i]);
			}
			///> blink red LED indicating patient is not acknowledged
			LED_rtn = NOT_PATIENT;
		}
		///> else if databse is empty,
		else if( check_patient_rtn == NO_PATIENT )
		{
			///> print out empty database message
			UartSendCString("[INFO] No patient in database");
			LED_rtn = NO_PATIENT;
		}
		///> if an error has occured
		else
		{
			///> print out error message
			UartSendCString("[ERROR] Patient checking error");
			LED_rtn = ERROR;
		}
		///> printout return char for formating
		UartPutCrlf();
	}
	///> In Edit mode,
	else if( mode_sel == 1 )
	{
		///> if databse is empty or patient is not found,
		if( ( check_patient_rtn == NOT_PATIENT ) || ( check_patient_rtn == NO_PATIENT ) )
		{
			///> attempt to add the patient
			int add_patient_rtn = add_patient( patient, tag_size );
			///> if adding patient success,
			if( add_patient_rtn == ADDED_PATIENT )
			{
				///> print out patient added message
				UartSendCString("[INFO] Patient Added, ID = ");
				for(i=0; i<tag_size; i++)
				{
					UartPutByte(patient[i]);
				}
				///> blink gree LED indicating patient added
				LED_rtn = ADDED_PATIENT;
			}
			///> if databse is full,
			else if ( add_patient_rtn == FULL_PATIENT )
			{
				///> blink both green and red LED indicating patient database is full
				LED_rtn = FULL_PATIENT;
				UartSendCString("[INFO] Patient Databse FULL");
			}
			///> if error,
			else
			{
				///> blink LEDs in error pattern indicating error occured
				LED_rtn = ERROR;
				char buf[20];
				sprintf( buf, "[DEBUG] YOLO, add_patient returned: %d", add_patient_rtn );
				UartSendCString(buf);
			}
			///> printout return char for formating
			UartPutCrlf();
		}
		///> else if patient is found in the database,
		else if( check_patient_rtn == IS_PATIENT )
		{
			///> attempt to remove patient,
			if( remove_patient( patient, tag_size ) == REMOVED_PATIENT )
			{
				///> if patient is successfully removed, print out its ID
				UartSendCString("[INFO] Patient Removed, ID = ");
				for(i=0; i<tag_size; i++)
				{
					UartPutByte( patient[i] );
				}
				///> blink green LED indicating patient is removed
				LED_rtn = REMOVED_PATIENT;
			}
			///> if remove patient failed, print out database empty message
			else
			{
				UartSendCString( "[INFO] No patient in database" );
				///> blink both green and red LED indicating empty database
				LED_rtn = NO_PATIENT;
			}
			///> printout return char for formating
			UartPutCrlf();
		}
		///> if check patient returned error,
		else
		{
			///> print out error message
			UartSendCString( "[ERROR] Patient checking error" );
			UartPutCrlf();
			///> blink LEDs in error pattern indicating error occured
			LED_rtn = ERROR;

		}
	}
	///> if mode select in invalid,
	else
	{
		///> print out invalid mode message
		UartSendCString( "[ERROR] Invalid mode" );
		UartPutCrlf();
		///> blink LEDs in error pattern indicating error occured
		LED_rtn = ERROR;
	}
	///> return LED value to LED_disco()
	return LED_rtn;
}


/**
 * Blink the red LED.
 * Blink the red LED in 20Hz frequency for a predefined times.
 * 
 */ 
void LED_red_blink()
{
	int i=0;
	///> blink LED number of times defined in LED_BLINK_RATE
	for( i=0; i<LED_BLINK_RATE; i++ )
	{
		LED_14443A_ON;
		McuDelayMillisecond(50);
		LED_14443A_OFF;
		McuDelayMillisecond(50);
	}
}


/**
 * Blink the green LED.
 * Blink the green LED in 20Hz frequency for a predefined times.
 * 
 */ 
void LED_green_blink()
{
	int i=0;
	///> blink LED number of times defined in LED_BLINK_RATE
	for( i=0; i<LED_BLINK_RATE; i++ )
	{
		LED_15693_ON;
		McuDelayMillisecond(50);
		LED_15693_OFF;
		McuDelayMillisecond(50);
	}
}


/**
 * Blink both LEDs.
 * Blink both LEDs in 20Hz frequency for a predefined times.
 * 
 */ 
void LED_yolo_blink()
{
	int i=0;
	///> blink LED number of times defined in LED_BLINK_RATE
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

/**
 * Blink LEDs according to return values from authenticate_patient().
 * 
 * Case:	IS_PATIENT, blink green LED.
 * 		NOT_PATIENT, blink red LED.
 * 		ADDED_PATIENT, blink green LED.
 * 		REMOVED_PATIENT, blink red LED.
 * 		FULL_PATIENT, blink both LED.
 * 		NO_PATIENT, blink both LED.
 * 		TAG_SIZE_ERROR, blink both LED then red LED.
 * 		other cases, blink green LED then red LED then both LED.
 * 
 * @param LED_val Return value from authenticate_patient().
 * 
 */ 
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
