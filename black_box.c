#include "black_box.h"          // Includes black_box header file
#include "clcd1.h"              // Includes clcd1 header file for LCD functions
#include "matrix1.h"            // Includes matrix1 header file
#include "adc.h"                // Includes ADC header file
#include"uart.h"               // Includes UART header file
#include"e_EEPROM.h"           // Includes external EEPROM header file

/* this variables for view_dashboard*/
extern int key; // External variable for key input
unsigned char time[9]; // Time array initialized to 00:00:00
unsigned char ev[8][3] = {"ON", "GR", "GN", "G1", "G2", "G3", "G4", "c "}; // Event array
int i = 0; // Index variable for events
int sp; // Speed variable
unsigned int flag = 0; // Flag variable

/* this variables for event_store*/
extern char add = 0; // External address variable for EEPROM
unsigned int count1 = 0; // Counter for events
char data = 0; // Temporary data variable

/* this variables for view_log*/
extern int r_add; // External read address variable
unsigned char r_event[10][15]; // Array to store read events
int i1 = 0; // Index variable for view log
char j1 = 0; // Secondary index variable
extern char v_flag = 1; // External view flag

/* THIS FOR SET TIME*/
char s_flag = 1; // Set time flag
extern int s_hr; // External hour variable
extern int s_min; // External minute variable
extern int s_sec; // External second variable

int count = 0; // Counter for blinking effect
char h_flag = 0; // Hour flag
char m_flag = 0; // Minute flag
char sec_flag = 0; // Second flag
int s_f = 0; // State flag for set time

extern int d_flag; // Download flag
extern int k_flag; // Key flag

// Function to display dashboard

void view_dashboard() {
    clcd_print("TIME", LINE1(0)); // Print "TIME" on LCD line 1
    clcd_print("EVENT", LINE1(7)); // Print "EVENT" on LCD line 1
    clcd_print("SPd", LINE1(13)); // Print "SPd" on LCD line 1

    clcd_print(time, LINE2(0)); // Print time on LCD line 2

    // Handle key presses for event selection
    if (key == MK_SW3 && (!flag)) { // If SW3 pressed and flag not set
        clcd_print("   ", LINE2(9)); // Clear event display
        flag = 1; // Set flag
        i = 7; // Set event index
        count1++; // Increment event count
        event_store(); // Store event
    } else if (key == MK_SW1) { // If SW1 pressed
        if (i < 6 && (!flag)) { // If index < 6 and flag not set
            ++i; // Increment index
            event_store(); // Store event
            count1++; // Increment event count
        } else if (flag) { // If flag set
            i = 2; // Reset index
            event_store(); // Store event
            count1++; // Increment event count
        }
    } else if (key == MK_SW2) { // If SW2 pressed
        if (i > 1 && (!flag) ) { // If index > 1 and flag not set
            i--; // Decrement index
             event_store(); // Store event
             count1++; // Increment event count
        } else if (flag) { // If flag set
            i = 2; // Reset index
            event_store(); // Store event
            count1++; // Increment event count
        }
    }
    clcd_print(ev[i], LINE2(9)); // Print current event

    // Read and display speed from ADC
    unsigned short adc_reg_val;
    adc_reg_val = read_adc(CHANNEL4); // Read ADC channel 4
    sp = (adc_reg_val * 99) / 1023; // Convert to speed (0-99)
    clcd_putch((sp / 10 + '0'), LINE2(14)); // Display tens digit
    clcd_putch((sp % 10 + '0'), LINE2(15)); // Display units digit
}

// Function to store events in EEPROM

void event_store() {
    if (count1 > 10) { // If more than 10 events
        for (int j = 0; j < 108; j++) { // Shift old events
            data = read_external_eeprom(j + 12); // Read from external EEPROM
            write_external_eeprom(j, data); // Write to external EEPROM
        }
        count1--; // Decrement count
        add = 108; // Set new address
    }
    // Store time in EEPROM
    for (int j = 0; j < 8; j++) {
        write_external_eeprom(add++, time[j]); // Store each time character
    }
    // Store event in EEPROM
    write_external_eeprom(add++, ev[i][0]); // Store first event character
    write_external_eeprom(add++, ev[i][1]); // Store second event character

    // Store speed in EEPROM
    write_external_eeprom(add++, (sp / 10) + '0'); // Store tens digit
    write_external_eeprom(add++, (sp / 10) + '0'); // Store tens digit (duplicate?)
}

// Function to view logged events

void view_log() {
    if (count1 == 0) // If no events logged
    {
        clcd_print("LIST EMPTY", LINE1(0)); // Display empty message
    } else {
        if (v_flag == 1) { // If view flag set
            v_flag = 0; // Clear flag
            r_add = 0; // Reset read address
            // Read all events from EEPROM
            for (int k = 0; k < count1; k++) {
                for (int j = 0; j < 15; j++) {
                    if (j == 8 || j == 11) { // Add spaces at specific positions
                        r_event[k][j] = ' ';
                    } else if (j == 14) // Null terminate
                        r_event[k][j] = '\0';
                    else {
                        r_event[k][j] = read_external_eeprom(r_add++); // Read from EEPROM
                    }
                }
            }
        }
        // Display log header
        clcd_print("I", LINE1(0)); // Print index header
        clcd_print("TIME", LINE1(2)); // Print time header
        clcd_print("EVENT", LINE1(7)); // Print event header
        clcd_print("SPd", LINE1(13)); // Print speed header

        // Handle navigation keys
        if (key == MK_SW2) { // If SW2 pressed
            if (i1 < count1 - 1) // If not at last event
                i1++; // Increment index
        }
        if (key == MK_SW1) { // If SW1 pressed
            if (i1 > 0) // If not at first event
                i1--; // Decrement index
        }
        // Display current event
        clcd_putch((i1 + 48), LINE2(0)); // Display event number
        clcd_print(r_event[i1], LINE2(2)); // Display event details
    }
}

// Function to download log via UART

void download_log(void) {
    
    // Read all events from EEPROM
    clcd_print("e_download_log", LINE1(4)); // Display download message
    for(unsigned long int i=1000000;i--;);
    if(count1 == 0)
    {
         uart_puts("LIST EMPTY\n\r");
    }
    else
    {
    for (int k = 0; k < count1; k++) {
        for (int j = 0; j < 15; j++) {
            if (j == 8 || j == 11) { // Add spaces
                r_event[k][j] = ' ';
            } else if (j == 14) // Null terminate
                r_event[k][j] = '\0';
            else {
                r_event[k][j] = read_external_eeprom(r_add++); // Read from EEPROM
            }
        }
    }
    uart_puts("download\n\r"); // Send download header
    // Send all events via UART
    
    for (int j = 0; j < count1; j++) {
        uart_puts(r_event[j]); // Send event
        uart_putch('\n'); // New line
        uart_putch('\r'); // Carriage return
    }
}
}

// Function to clear the log

void clear_log(void) {
    clcd_print("e_clear_log", LINE1(4)); // Display clear message
    for(unsigned long int i=1000000;i--;);
    count1 = 0; // Reset event count
    add = 0; // Reset write address
    r_add = 0; // Reset read address
    d_flag = 1; // Download flag
    k_flag = 1; // Key flag
}

// Function to set time

void set_time(void) {
    clcd_print("HH:MM:SS", LINE1(4)); // Display time format

    if (s_flag) { // If set flag active
        s_flag = 0; // Clear flag
        // Convert time string to numbers
        s_hr = ((time[0] - '0')*10)+(time[1] - '0');
        s_min = ((time[3] - '0')*10)+(time[4] - '0');
        s_sec = ((time[6] - '0')*10)+(time[7] - '0');
        h_flag = 1; // Set hour flag
    }

    // Handle key presses for time setting
    if (key == MK_SW2) { // If SW2 pressed (mode change)
        if (s_f == 0) { // If in hour mode
            count = 0; // Reset counter
            s_f = 1; // Change to minute mode
            m_flag = 1; // Set minute flag
            sec_flag = 0; // Clear second flag
            h_flag = 0; // Clear hour flag
        } else if (s_f == 1) { // If in minute mode
            count = 0; // Reset counter
            s_f = 2; // Change to second mode
            sec_flag = 1; // Set second flag
            m_flag = 0; // Clear minute flag
            h_flag = 0; // Clear hour flag
        } else if (s_f == 2) { // If in second mode
            count = 0; // Reset counter
            s_f = 0; // Change to hour mode
            h_flag = 1; // Set hour flag
            m_flag = 0; // Clear minute flag
            sec_flag = 0; // Clear second flag
        }
    } else if (key == MK_SW1) { // If SW1 pressed (increment)
        if (h_flag) { // If in hour mode
            if (s_hr < 24) { // If hour < 24
                s_hr++; // Increment hour
            }
        } else if (m_flag) { // If in minute mode
            if (s_min < 60) { // If minute < 60
                s_min++; // Increment minute
            } else if (s_min == 60) { // If minute == 60
                s_hr++; // Increment hour
            }
        } else if (sec_flag) { // If in second mode
            if (s_sec < 60) { // If second < 60
                s_sec++; // Increment second
            } else if (s_sec == 60) { // If second == 60
                s_min++; // Increment minute
            }
        }
    }

    // Display time with blinking effect for current field
    if (h_flag) { // If in hour mode
        if (count < 100) { // Display hours normally
            clcd_putch(((s_hr / 10) + '0'), LINE2(4));
            clcd_putch(((s_hr % 10) + '0'), LINE2(5));
        } else if (count < 200) { // Hide hours (blink effect)
            clcd_write(0xff, 0);
            clcd_putch(0xff, LINE2(4));
            clcd_putch(0xff, LINE2(5));
        } else {
            count = 0; // Reset counter
        }
        // Display other fields normally
        clcd_putch(':', LINE2(6));
        clcd_putch((s_min / 10 + '0'), LINE2(7));
        clcd_putch((s_min % 10 + '0'), LINE2(8));
        clcd_putch(':', LINE2(9));
        clcd_putch((s_sec / 10 + '0'), LINE2(10));
        clcd_putch((s_sec % 10 + '0'), LINE2(11));
    } else if (m_flag) { // If in minute mode
        // Display hours normally
        clcd_putch(((s_hr / 10) + '0'), LINE2(4));
        clcd_putch(((s_hr % 10) + '0'), LINE2(5));
        clcd_putch(':', LINE2(6));
        // Handle minute display with blink
        if (count < 100) { // Display minutes normally
            clcd_putch(((s_min / 10) + '0'), LINE2(7));
            clcd_putch((s_min % 10) + '0', LINE2(8));
        } else if (count < 200) { // Hide minutes (blink)
            clcd_putch(0xff, LINE2(7));
            clcd_putch(0xff, LINE2(8));
        } else
            count = 0; // Reset counter
        // Display other fields normally
        clcd_putch(':', LINE2(9));
        clcd_putch((s_sec / 10 + '0'), LINE2(10));
        clcd_putch((s_sec % 10 + '0'), LINE2(11));
    } else if (sec_flag) { // If in second mode
        // Display hours and minutes normally
        clcd_putch((s_hr / 10 + '0'), LINE2(4));
        clcd_putch((s_hr % 10 + '0'), LINE2(5));
        clcd_putch(':', LINE2(6));
        clcd_putch((s_min / 10 + '0'), LINE2(7));
        clcd_putch((s_min % 10 + '0'), LINE2(8));
        clcd_putch(':', LINE2(9));
        // Handle second display with blink
        if (count < 100) { // Display seconds normally
            clcd_putch((s_sec / 10 + '0'), LINE2(10));
            clcd_putch((s_sec % 10 + '0'), LINE2(11));
        } else if (count < 200) { // Hide seconds (blink)
            clcd_putch(0xff, LINE2(10));
            clcd_putch(0xff, LINE2(11));
        } else
            count = 0; // Reset counter
    }
    count++; // Increment counter
}