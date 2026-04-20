#include "e_EEPROM.h"    // Header file for EEPROM-related functions
#include "i2c.h"         // Header file for I2C communication functions

// Function to write a byte of data to an external EEPROM at a given address
void write_external_eeprom(unsigned char address, unsigned char data)
{
    i2c_start();                  // Initiate I2C start condition
    i2c_write(SLAVE_WRITE_E);    // Send slave address with write command
    i2c_write(address);          // Send memory address to write to
    i2c_write(data);             // Send the data byte to be written
    i2c_stop();                  // Send I2C stop condition
    for(unsigned int i=3000; i--;); // Delay to allow EEPROM write cycle to complete
}

// Function to read a byte of data from an external EEPROM at a given address
unsigned char read_external_eeprom(unsigned char address)
{
    unsigned char data;

    i2c_start();                 // Initiate I2C start condition
    i2c_write(SLAVE_WRITE_E);   // Send slave address with write command (to set address)
    i2c_write(address);         // Send memory address to read from
    i2c_rep_start();            // Send I2C repeated start condition
    i2c_write(SLAVE_READ_E);    // Send slave address with read command
    data = i2c_read();          // Read the data byte from EEPROM
    i2c_stop();                 // Send I2C stop condition

    return data;                // Return the read data
}
