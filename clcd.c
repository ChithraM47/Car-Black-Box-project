#include <xc.h>
#include "clcd1.h"

// Function to write a byte to CLCD with control bit specifying data/instruction
void clcd_write(unsigned char byte, unsigned char control_bit)
{
    CLCD_RS = control_bit;    // Set RS: 0 = instruction, 1 = data
    CLCD_PORT = byte;         // Send byte to CLCD data lines

    // Pulse the Enable pin to latch the data/command
    CLCD_EN = HI;
    CLCD_EN = LO;

    PORT_DIR = INPUT;         // Set port as input to read busy flag
    CLCD_RW = HI;             // Read mode
    CLCD_RS = INSTRUCTION_COMMAND; // Ensure command register is selected

    // Wait until CLCD is not busy
    do
    {
        CLCD_EN = HI;
        CLCD_EN = LO;
    } while (CLCD_BUSY);

    CLCD_RW = LO;             // Set back to write mode
    PORT_DIR = OUTPUT;        // Set port back to output
}

// Function to initialize the CLCD
void init_clcd(void)
{
    TRISD = 0x00;             // PortD as output (CLCD data lines)
    TRISC = TRISC & 0xF8;     // RC0, RC1, RC2 as output (control lines)

    CLCD_RW = LO;             // Set RW to write mode

    __delay_ms(30);           // Wait for CLCD to power up

    // Send initialization commands
    clcd_write(EIGHT_BIT_MODE, INSTRUCTION_COMMAND);
    __delay_us(4100);
    clcd_write(EIGHT_BIT_MODE, INSTRUCTION_COMMAND);
    __delay_us(100);
    clcd_write(EIGHT_BIT_MODE, INSTRUCTION_COMMAND);
    __delay_us(1);

    CURSOR_HOME;              // Move cursor to home position
    __delay_us(100);
    TWO_LINE_5x8_MATRIX_8_BIT; // Function set: 2-line, 5x8 font
    __delay_us(100);
    CLEAR_DISP_SCREEN;        // Clear display
    __delay_us(500);
    DISP_ON_AND_CURSOR_OFF;   // Display ON, cursor OFF
    __delay_us(100);
}

// Function to print a string starting from a specific CLCD address
void clcd_print(const unsigned char *data, unsigned char addr)
{
    clcd_write(addr, INSTRUCTION_COMMAND); // Set cursor to address
    while (*data != '\0')                  // Loop until null terminator
    {
        clcd_write(*data++, DATA_COMMAND); // Write each character
    }
}

// Function to write a single character to a specific CLCD address
void clcd_putch(const unsigned char data, unsigned char addr)
{
    clcd_write(addr, INSTRUCTION_COMMAND); // Set cursor to address
    clcd_write(data, DATA_COMMAND);        // Write character to CLCD
}
