

#include"i2c.h"
#include"ds1037.h"

/* 
 * DS1307 Slave address
 * D0  -  Write Mode
 * D1  -  Read Mode
 */

void init_ds1307(void)
{
	unsigned char dummy;

	/* Setting the CH bit of the RTC to Stop the Clock */
	dummy = read_ds1307(SEC_ADDR);
	write_ds1307(SEC_ADDR, dummy | 0x80); 

	/* Seting 12 Hr Format */
	dummy = read_ds1307(HOUR_ADDR);
	write_ds1307(HOUR_ADDR, dummy | 0x40); 

	/* 
	 * Control Register of DS1307
	 * Bit 7 - OUT
	 * Bit 6 - 0
	 * Bit 5 - OSF
	 * Bit 4 - SQWE
	 * Bit 3 - 0
	 * Bit 2 - 0
	 * Bit 1 - RS1
	 * Bit 0 - RS0
	 * 
	 * Seting RS0 and RS1 as 11 to achive SQW out at 32.768 KHz
	 */ 
	write_ds1307(CNTL_ADDR, 0x93); 

	/* Clearing the CH bit of the RTC to Start the Clock */
	dummy = read_ds1307(SEC_ADDR);
	write_ds1307(SEC_ADDR, dummy & 0x7F); 

}

void write_ds1307(unsigned char address, unsigned char data)
{
	i2c_start();                 // Initiate I2C communication
	i2c_write(SLAVE_WRITE);      // Send DS1307 write address
	i2c_write(address);          // Send the register address to write to
	i2c_write(data);             // Send the data to be written
	i2c_stop();                  // Stop I2C communication
}

unsigned char read_ds1307(unsigned char address)
{
	unsigned char data;

	i2c_start();                 // Initiate I2C communication
	i2c_write(SLAVE_WRITE);      // Send DS1307 write address
	i2c_write(address);          // Send the register address to read from
	i2c_rep_start();             // Send a repeated start condition
	i2c_write(SLAVE_READ);       // Send DS1307 read address
	data = i2c_read();           // Read the data from DS1307
	i2c_stop();                  // Stop I2C communication

	return data;                 // Return the read data
}
