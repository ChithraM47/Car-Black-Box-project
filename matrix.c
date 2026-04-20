#include <xc.h>
#include "matrix1.h"

void init_matrix_keypad(void)
{
	/* Config PORTB as digital */
	ADCON1 = 0x0F;                        // Set all PORTB pins as digital I/O

	/* Set Rows (RB7 - RB5) as Outputs and Columns (RB4 - RB1) as Inputs */
	TRISB = 0x1E;                         // 00011110b -> RB4-RB1 input (columns), RB7-RB5 output (rows)

	/* Set PORTB input as pull up for columns */
	RBPU = 0;                             // Enable PORTB internal pull-ups

	MATRIX_KEYPAD_PORT = MATRIX_KEYPAD_PORT | 0xE0; // Set rows high (RB7-RB5)
}

unsigned char scan_key(void)
{
	ROW1 = LO;                            // Activate row 1
	ROW2 = HI;
	ROW3 = HI;

	if (COL1 == LO)                       // Check column 1
	{
		return 1;
	}
	else if (COL2 == LO)                 // Check column 2
	{
		return 4;
	}
	else if (COL3 == LO)                 // Check column 3
	{
		return 7;
	}
	else if (COL4 == LO)                 // Check column 4
	{
		return 10;
	}

	ROW1 = HI;
	ROW2 = LO;                            // Activate row 2
	ROW3 = HI;

	if (COL1 == LO)
	{
		return 2;
	}
	else if (COL2 == LO)
	{
		return 5;
	}
	else if (COL3 == LO)
	{
		return 8;
	}
	else if (COL4 == LO)
	{
		return 11;
	}

	ROW1 = HI;
	ROW2 = HI;
	ROW3 = LO;                            // Activate row 3
	/* TODO: Why more than 2 times? */
	ROW3 = LO;                            // Redundant line ? could be a mistake or for emphasis

	if (COL1 == LO)
	{
		return 3;
	}
	else if (COL2 == LO)
	{
		return 6;
	}
	else if (COL3 == LO)
	{
		return 9;
	}
	else if (COL4 == LO)
	{
		return 12;
	}

	return 0xFF;                           // No key pressed
}

unsigned char read_switches(unsigned char detection_type)
{
	static unsigned char once = 1, key;

	if (detection_type == STATE_CHANGE)
	{
		key = scan_key();                // Get current key
		if(key != 0xFF && once)
		{
			once = 0;                   // Set flag to prevent multiple detections
			return key;
		}
		else if(key == 0xFF)
		{
			once = 1;                   // Reset flag when no key is pressed
		}
	}
	else if (detection_type == LEVEL_CHANGE)
	{
		return scan_key();              // Directly return current key
	}

	return 0xFF;                         // Default return if no key is pressed
}
