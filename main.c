/*
 * File:   main.c
 * NAME :CHITHRA M
 *DATE:25/01/25
 * 
 * DESCRIPTION:To design and develop an embedded system that functions as a Car Black Box, 
 * capable of logging critical events, maintaining real-time timestamps, 
 * and allowing the user to interact with the system using a matrix keypad and 
 * a character LCD (CLCD). The system enhances vehicle monitoring and 
 * data recording for post-incident analysis.
Overview:
The Car Black Box is inspired by aviation black boxes and 
 * aims to record vital data in a vehicle, such as accident events, 
 * sharp turns, or sudden braking. This data can be used for accident investigation, 
 * driving behavior analysis, or fleet management.

Key Features:
Event Logging: Automatically logs events such as sudden impacts or abnormal decelerations.
Time-stamping: Uses a DS1307 Real-Time Clock (RTC) to timestamp each event.
User Interface: Controlled through a 4x3 matrix keypad and a 16x2 CLCD display.
Log Management:
View Logs: Scroll through stored event data.
Download Logs: Logs can be transferred to a PC via UART or USB.
Clear Logs: Allows the user to erase the memory.
Set Time: Enables time configuration using the keypad.
Non-volatile Storage: Events are stored in EEPROM or external memory to retain logs after power loss.
Hardware Components:
Microcontroller: PIC16F877A (or any 8-bit MCU)
RTC Module: DS1307 with battery backup
Display: 16x2 Character LCD
Keypad: 4x3 Matrix Keypad
Memory: Internal EEPROM or external I2C EEPROM
Communication: UART interface for PC connectivity
Power Supply: 5V regulated supply with vehicle interface
Software Components:
I2C Communication routines (for RTC and EEPROM)
UART routines for data transfer
LCD and keypad drivers
Event detection algorithm
Menu-driven interface logic
Data storage and retrieval logic
Applications:
Accident analysis and post-crash investigation
Monitoring driver behavior
Fleet vehicle tracking and management
Insurance claims and evidence support
 */

// Include necessary header files
#include "black_box.h"      // Main black box header
#include "adc.h"           // ADC functions
#include"clcd1.h"          // Character LCD functions
#include"matrix1.h"        // Matrix keypad functions
#include"uart.h"           // UART communication
#include"e_EEPROM.h"       // External EEPROM
#include "i2c.h"           // I2C communication
#include "ds1037.h"        // DS1307 RTC functions

// Define state variable
State_t state; // Current system state

// Menu items array
char menu[4][17] = {"e_view_log", "e_download_log", "e_clear_log", "e_set_time"};
char star = 0; // Menu selection indicator
int index = 0; // Menu index
char k = 0; // Temporary variable
int key; // Key press value
char add = 0; // EEPROM write address
int r_add = 0; // EEPROM read address
char v_flag = 1; // View log flag
unsigned char clock_reg[3]; // RTC time registers
unsigned char time[9]; // Time string buffer
int d_flag = 1; // Download flag
int k_flag = 1; // Key flag

/*this for settime*/
int s_hr; // Set hour value
int s_min; // Set minute value
int s_sec; // Set second value

// Initialize system configuration

void init_config() {
    init_clcd(); // Initialize LCD
    init_matrix_keypad(); // Initialize keypad
    init_adc(); // Initialize ADC
    state = e_dashboard; // Set initial state to dashboard
    init_i2c(); // Initialize I2C
    init_ds1307(); // Initialize RTC
    init_uart(); // Initialize UART
}

// Read time from RTC and format it

static void get_time(void) {
    // Read hours, minutes, seconds from RTC
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    // Format hours (handle 12/24 hour mode)
    if (clock_reg[0] & 0x40) // If 12-hour mode
    {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    } else // 24-hour mode
    {
        time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
        time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    // Format time string
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F); // Minutes tens
    time[4] = '0' + (clock_reg[1] & 0x0F); // Minutes units
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F); // Seconds tens
    time[7] = '0' + (clock_reg[2] & 0x0F); // Seconds units
    time[8] = '\0'; // Null terminator
}

// Main program loop

void main() {
    init_config(); // Initialize system


    while (1) { // Infinite loop
        // Detect key press
        get_time(); // Update current time

        key = read_switches(STATE_CHANGE); // Read keypad
        if (key == 11) { // If Enter key pressed
            CLEAR_DISP_SCREEN;
            if (state == e_set_time) // If in set time mode
            {
                // Save new time to RTC
                write_ds1307(HOUR_ADDR, (((s_hr / 10) << 4) | (s_hr % 10)));
                write_ds1307(MIN_ADDR, (((s_min / 10) << 4) | (s_min % 10)));
                write_ds1307(SEC_ADDR, (((s_sec / 10) << 4) | (s_sec % 10)));
                state = e_dashboard;
            } else if (k_flag) // First Enter press
            {
                k_flag = 0;
                state = e_main_menu; // Go to main menu
            } else // Subsequent Enter presses
                if (star == 0 && index == 0) {
                state = e_view_log; // View log
            } else if ((star == 1 && index == 0) || (star == 0 && index == 1)) {
                state = e_download_log; // Download log
            } else if ((star == 1 && index == 1) || (star == 0 && index == 2)) {
                state = e_clear_log; // Clear log
            } else if (star == 1 && index == 2) {
                state = e_set_time; // Set time
            }
        } else if (key == MK_SW12) { // If Cancel key pressed
            CLEAR_DISP_SCREEN;
            if (state == e_main_menu) {
                state = e_dashboard;
            } else if (state == e_set_time) // If in set time mode
            {
                state = e_dashboard; // Return to dashboard
            } else
                state = e_main_menu; // Return to main menu
            k_flag = 0; // Reset flags
            add = 0;
            r_add = 0;
            v_flag = 1;
            d_flag = 1;
        }
        // State machine
        switch (state) {
            case e_dashboard:
                view_dashboard(); // Show dashboard
                k_flag = 1;
                break;

            case e_main_menu:
                display_main_menu(); // Show main menu
                break;

            case e_view_log:
                view_log(); // Show log
                break;

            case e_download_log:
                if (d_flag) // Only download once
                {
                    d_flag = 0;
                    download_log(); // Download log
                    for (int wait = 1000; wait--;)
                        CLEAR_DISP_SCREEN;
                    state = e_dashboard;
                }
                break;

            case e_clear_log:
                clear_log(); // Clear log
                 for (int wait = 1000; wait--;)
                    CLEAR_DISP_SCREEN;
                    state = e_dashboard;
                break;

            case e_set_time:
                set_time(); // Set time
                break;
        }
    }
}

// Display main menu function

void display_main_menu() {
    // Handle star position
    if (star == 0) {
        clcd_putch('*', LINE1(0)); // Star on first line
    }

    // Handle key presses
    if (key == MK_SW2) { // Down key
        CLEAR_DISP_SCREEN;
        if (star == 0) {
            star = 1; // Move star down
        } else if (star == 1) {
            if (index != 2)
                index++; // Scroll menu down
        }
    } else if (key == MK_SW1) { // Up key
        CLEAR_DISP_SCREEN;
        if (star == 1) {
            star = 0; // Move star up
        } else if (star == 0) {
            if (index != 0)
                index--; // Scroll menu up
        }
    }

    // Display star at current position
    if (star == 0) {
        clcd_putch('*', LINE1(0));
    } else if (star == 1) {
        clcd_putch('*', LINE2(0));
    }

    // Display menu items
    clcd_print(menu[index], LINE1(2)); // Current item
    clcd_print(menu[index + 1], LINE2(2)); // Next item
}